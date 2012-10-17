#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

int main(int argc, char **argv){
	if (argc != 5) {
		printf("Usage: requester -p <port> -o <file option>\n");
		return 1;
	}
	int sock = 0, connfd = 0;
        struct sockaddr_in serv_addr;
        struct sockaddr_in ret_addr;
        int opt, port, reply;
        char *endptr, *option;

	while( (opt = getopt(argc, argv, "p:o:")) != -1) {
		switch(opt) {
                case 'p':
                    port = strtol(optarg, &endptr, 10);
                    if (*endptr != 0 || port < 1024 || port > 65536) {
                        printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
                                return 1;
                        }
                    break;
		case 'o':
		    option = optarg;
		    break;
		default:
		    printf("Usage: requester -p <port> -o <file option>\n");
		    return 1;
		}

	}

        if (( sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            printf("Socket error\n");
            return 1;
        }

        char *pkt, *buf;
        int pkt_len, buf_len, ret_len;

        //File tracking business
        FILE *fd = NULL;
        if ((fd = fopen("tracker.txt", "r")) == NULL){
            printf("Error opening file tracker.txt\n");
            return 1;
        }

        char *file, *serv;
        int piece, serv_port;

        while(fscanf(fd,"%s %d %s %d", file, &piece, serv, &serv_port) == 4){
           
            //TODO:
            getaddrinfo(serv, );

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(serv_port);
            serv_addr.sin_addr.s_addr = sock.gethostbyname(serv);
            bzero(&(serv_addr.sin_zero), 8);
            
            if(sendto(sock, pkt, pkt_len, 0, (struct sockaddr *)
                      &serv_addr, sizeof(serv_addr)) != pkt_len){
                printf("Error sending to server\n");
                return 1;
            }

            if(( reply = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *)
                                  &ret_addr, &ret_len)) != pkt_len) {
                printf("Error receiving data\n");
                return 1;
            }

            if(serv_addr.sin_addr.s_addr != ret_addr.sin_addr.s_addr) {
                printf("Error - Packet received from unknown source\n");
                return 1;
            }

        }
        fclose(fd);
        fclose(sock);


        

}
