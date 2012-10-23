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
#include <sys/time.h>
#include "utility.h"

#define min(a,b) (((a) < (b)) ? (a): (b))

int main(int argc, char **argv){
	if (argc != 5) {
		printf("Usage: requester -p <port> -o <file option>\n");
		return 1;
	}
	int sock = 0, connfd = 0;
        struct sockaddr_in serv_addr;
        struct sockaddr_in cli_addr;
        socklen_t serv_addr_len = sizeof(serv_addr);
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


        cli_addr.sin_family = AF_INET;
        cli_addr.sin_port = htons(port);
        cli_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(cli_addr.sin_zero), 8);
	
	if (bind(sock, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) == -1){
            printf("Bind error\n");
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
        struct timeval *time_start;
        struct timeval *time_end;
        struct timeval *time_result;

        memset(&hints, 0, sizeof(hints));

        while(getline(&buf, &n, fd) != -1){
            
            int total_pkts = 0;
            int total_data_bytes = 0;
            float avg_pkts_sec = 0;
            float time_start = 0;
            float time_end = 0;
            
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
                
                //HACK: This prevents from receiving pkts larger
                //than 1500
                pkt_len = 1500;

                
                int returned = recvfrom(sock, buf, pkt_len, MSG_PEEK, (struct sockaddr *)
                             &serv_addr, &serv_addr_len); 
                pkt_len = min(returned, pkt_len);
                buf = (char *) malloc(pkt_len);
                if(( reply = recvfrom(sock, buf, pkt_len, 0, (struct sockaddr *)
                                      &serv_addr, &serv_addr_len)) != pkt_len) {
                    printf("Error receiving data\n");
                    return 1;
                }
                gettimeofday(&time_start, NULL);
                while(buf[0] != 'E'){
                    total_pkts += 1;

                    int pay_len = strlen(buf + 9);
                    total_data_bytes += pay_len;
                    FILE *fp = fopen(option, "a");
                    fwrite(buf + 9, pay_len, 1, fp);

                    gettimeofday(&time_result, NULL);
                    //printf("Time:%fms\n", (((long)tp.tv_sec) * 1000) + (((long)tp.tv_usec) / 1000));
                    printf("IP:%s\n", inet_ntoa(serv_addr.sin_addr));
                    printf("SeqNo:%d\n", (buf + 1));
                    printf("Length:%d\n",(buf + 5));
                    char *data = (char *) malloc(5*sizeof(char));
                    memcpy(data, buf + 9, 4);
                    data[4] = '\0';
                    printf("First 4 bytes:%s\n\n", data);
                    free(data);
                
                    int returned = recvfrom(sock, buf, pkt_len, MSG_PEEK, (struct sockaddr *)
                                 &serv_addr, &serv_addr_len); 
                    pkt_len = min(returned, pkt_len);
                    buf = (char *) malloc(pkt_len);
                    if(( reply = recvfrom(sock, buf, pkt_len, 0, (struct sockaddr *)
                                          &serv_addr, &serv_addr_len)) != pkt_len) {
                        printf("Error receiving data\n");
                        return 1;
                    }
                }
                printf("Total packets:%d\n", total_pkts);
                printf("Total data bytes:%d\n", total_data_bytes);
                printf("Average pkts/sec:%f\n", avg_pkts_sec);
                gettimeofday(&time_end, NULL);
                timeval_subtract(&time_result, &time_start, &time_end);
                printf("Total time:%ld.%06ld\n", time_result->tv_sec, time_result->tv_usec);

            }

        }
        fclose(fd);
        close(sock);

}
