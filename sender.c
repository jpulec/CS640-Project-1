#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include "syscalls.h"

#define min(a,b) (((a) < (b)) ? (a): (b))

int main(int argc, char **argv){
	if (argc != 11) {
		printf("Usage: sender -p <port> -g <requester port> -r <rate> -q <seq_no> -l <length>\n");
		return 1;
	}
	int sock = 0, connfd = 0;
        struct sockaddr_in serv_addr;
        struct sockaddr_in cli_addr;
        unsigned int cli_addr_len;
        int opt, sender_port, requester_port, rate, seqNo, length, bytes_read;
        char *endptr;

	while( (opt = getopt(argc, argv, "p:g:r:q:l:")) != -1) {
		switch(opt) {
                  case 'p':
			sender_port = strtol(optarg, &endptr, 10);
			if (*endptr != 0 || sender_port < 1024 || sender_port > 65536) {
                            printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
				return 1;
			}
			break;
		case 'g': 
			requester_port = strtol(optarg, &endptr, 10);
			if (*endptr != 0 || requester_port < 1024 || requester_port > 65536) {
                            printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
				return 1;
			}
			break;
		case 'r': 
			rate = strtol(optarg, &endptr, 10);
			if (*endptr != 0) {
                            printf("Rate number %s is invalid.\n", optarg);
				return 1;
			}
			break;
		case 'q': 
			seqNo = strtol(optarg, &endptr, 10);
			if (*endptr != 0) {
                            printf("Rate number %s is invalid.\n", optarg);
				return 1;
			}
			break;
		case 'l': 
			length = strtol(optarg, &endptr, 10);
			if (*endptr != 0) {
                            printf("Length %s is invalid.\n", optarg);
				return 1;
			}
			break;
		default:
			printf("Usage: sender -p <port> -g <requester port> -r <rate> -q <seq_no> -l <length>\n");
			return 1;
		}

	}

        if (( sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            printf("Socket error\n");
            return 1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(sender_port);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(serv_addr.sin_zero), 8);

	if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
            printf("Bind error\n");
            return 1;
            }
        int pkt_len = length + sizeof(unsigned int)*2 + sizeof(char);
        char buf[pkt_len];
        char *pkt;
        struct timeval *tp;

        FILE *fd = NULL;


        if ( (bytes_read = recvfrom(sock, buf, pkt_len, 0, 
             (struct sockaddr *) &cli_addr, &cli_addr_len)) == -1){
            printf("Error receiving data from client\n");
            return 1;
        }

        if(buf[0] = 'R') {

            if ((fd = fopen(buf + 9, "r")) == NULL){
                printf("Error opening file %s\n", buf + 9);
                return 1;
            }
            int offset = 0;
            char data[length];
            while (read(fd, data, length) != -1){
                printf("%s",strerror(errno));
                pkt_len = min(strlen(data), length) + sizeof(unsigned int)*2 + sizeof(char);

                pkt = (char *) malloc(pkt_len);
                seqNo = htonl(seqNo);
                length = htonl(length);
                memset(pkt, 'D', 1);
                memcpy(pkt + 1, &seqNo, 4);
                memcpy(pkt + 5, &length, 4);
                strcpy(pkt + 9, data);

                if (sendto(sock, pkt, pkt_len, 0,
                          (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1){
                    printf("Error sending data to client\n");
                    return 1;
                }
                else{
                    gettimeofday(tp, NULL);
                    printf("Time:%d %d\n", tp->tv_sec, tp->tv_usec);
                    printf("IP:%s\n", cli_addr.sin_addr.s_addr);
                    printf("SeqNo:%d\n", seqNo);
                    printf("First 4 bytes:%s\n", data);
                }
            
            }
        }
}

