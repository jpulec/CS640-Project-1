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
	int listenfd = 0, connfd = 0;
        struct sockaddr_in serv_addr;
        int opt, port;
        char *endptr, *option;

	while( (opt = getopt(argc, argv, "p:o:")) != -1) {
		switch(opt) {
                case 'p':
                    senderPort = strtol(optarg, &endptr, 10);
                    if (*endptr != 0 || senderPort < 1024 || senderPort > 65536) {
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

