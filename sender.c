#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>



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
			senderPort = strtol(optarg, &endptr);
			if (endptr != NULL || senderPort < 1024 || senderPort > 65536) {
                            printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
				return 1;
			}
			break;
		case 'g': 
			requesterPort = strtol(optarg, &endptr);
			if (endptr != NULL || requesterPort < 1024 || requesterPort > 65536) {
                            printf("Port number %s is invalid. Port must be between 1024 and 65536.\n", optarg);
				return 1;
			}
			break;
		case 'r': 
			rate = strtol(optarg, &endptr);
			if (endptr != NULL) {
                            printf("Rate number %s is invalid.\n", optarg);
				return 1;
			}
			break;
		case 'q': 
			seqNo = strtol(optarg, &endptr);
			if (endptr != NULL) {
                            printf("Rate number %s is invalid.\n", optarg);
				return 1;
			}
			break;
		case 'l': 
			break;
		default:
			printf("Usage: sender -p <port> -g <requester port> -r <rate> -q <seq_no> -l <length>\n");
			return 1;
		}

	}

        listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        listen(listenfd, 10);
        
        while (1){
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        
        }
}

