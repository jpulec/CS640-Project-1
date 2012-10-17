#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

int main(int argc, char **argv){
	if (argc != 11) {
		printf("Usage: sender -p <port> -g <requester port> -r <rate> -q <seq_no> -l <length>\n");
		return 1;
	}
	int listenfd = 0, connfd = 0;
        struct sockaddr_in serv_addr;
        int opt, senderPort, requesterPort, rate, seqNo, length;
        char *endptr;

	while( (opt = getopt(argc, argv, "p:g:r:q:l:")) != -1) {
		switch(opt) {
                  case 'p':
			senderPort = strtol(optarg, &endptr, 10);
			if (*endptr != 0 || senderPort < 1024 || senderPort > 65536) {
                            printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
				return 1;
			}
			break;
		case 'g': 
			requesterPort = strtol(optarg, &endptr, 10);
			if (*endptr != 0 || requesterPort < 1024 || requesterPort > 65536) {
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

        if (( listenfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            perror("Socket error");
            return 1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(senderPort);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(serv_addr.sin_zero), 8);

	if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){
            perror("Bind error");
            return 1;
            }

        listen(listenfd, 10);
        
        while (1){
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        
        }
}

