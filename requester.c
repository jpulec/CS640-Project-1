#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>

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

        char *pkt;
        char *buf = NULL;
        size_t n = 0;
        int pkt_len, buf_len, ret_len;

        //File tracking business
        FILE *fd = NULL;
        if ((fd = fopen("tracker.txt", "r")) == NULL){
            printf("Error opening file tracker.txt\n");
            return 1;
        }

        char *file;
        char *serv;
        int serv_port, sent;
        unsigned int piece;
        unsigned int zero = 0;
        struct addrinfo hints;
        struct addrinfo *res;

        memset(&hints, 0, sizeof(hints));

        while(getline(&buf, &n, fd) != -1){
             // fscanf(fd,"%s %d %s %d", file, &piece, serv, &serv_port) == 4){
            file = strtok(buf," ");           
            piece = strtoul(strtok(NULL," "), NULL, 10);           
            serv = strtok(NULL," ");           
            serv_port = strtoul(strtok(NULL," "), NULL, 10);           
            
            if(strcmp(file, option) == 0){
            

                //getaddrinfo(serv, NULL, &hints, &res);
                 struct hostent *host = (struct hostent *) gethostbyname((char *) serv);       

                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(serv_port);
                serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
                bzero(&(serv_addr.sin_zero), 8);
                pkt_len = 1 + strlen(file) + sizeof(unsigned int)*2 + sizeof(char);

                pkt = (char *) malloc(pkt_len);
                piece = htonl(piece);
                zero = htonl(zero);
                memset(pkt, 'R', 1);
                memcpy(pkt + 1, &piece, 4);
                memcpy(pkt + 5, &zero, 4);
                strcpy(pkt + 9, file);

                if((sent = sendto(sock, pkt, pkt_len, 0, (struct sockaddr *)
                          &serv_addr, sizeof(serv_addr))) != pkt_len){
                    printf("Error sending to server:%s\n", strerror(errno));
                    return 1;
                }

                if(( reply = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *)
                                      &ret_addr, &ret_len)) != pkt_len) {
                    printf("Error receiving data\n");
                    return 1;
                }else{

                    struct timeval *tp;
                    gettimeofday(tp, NULL);
                    printf("Time:%d %d\n", tp->tv_sec, tp->tv_usec);
                    printf("IP:%s\n", serv_addr.sin_addr.s_addr);
                    printf("SeqNo:%d\n", buf[8]);
                    printf("Length:%d\n", buf[4]);
                    printf("First 4 bytes:%s\n", buf[14]);
                }

                if(serv_addr.sin_addr.s_addr != ret_addr.sin_addr.s_addr) {
                    printf("Error - Packet received from unknown source\n");
                    return 1;
                }
            }

        }
        fclose(fd);
        fclose(sock);


        

}
