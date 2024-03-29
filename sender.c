#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include "utility.h"

#define min(a,b) (((a) < (b)) ? (a): (b))

int main(int argc, char **argv){
    if (argc != 11) {
            printf("Usage: sender -p <port> -g <requester port> -r <rate> -q <seq_no> -l <length>\n");
            return 1;
    }
    int sock = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    int opt, sender_port, requester_port, rate, length, bytes_read;
    long seqNo;
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
                    seqNo = strtoul(optarg, &endptr, 10);
                    if (*endptr != 0) {
                        printf("Rate number %s is invalid.\n", optarg);
                            return 1;
                    }
                    break;
            case 'l': 
                    length = strtoul(optarg, &endptr, 10);
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
        printf("Bind error:\n");
        return 1;
        }
    int pkt_len = length + sizeof(unsigned int)*2 + sizeof(char);
    char buf[pkt_len];
    char *pkt = malloc(0);
    struct timeval tp;

    FILE *fd = NULL;

    if ( (bytes_read = recvfrom(sock, buf, pkt_len, 0, 
         (struct sockaddr *) &cli_addr, &cli_addr_len)) == -1){
        printf("Error receiving data from client\n");
        return 1;
    }

    if(buf[0] == 'R') {

        if ((fd = fopen(buf + 9, "r")) == NULL){
            printf("Error opening file %s\n", buf + 9);
            return 1;
        }
        //int offset = 0;
        char data[length + 1];
        while (fread(&data, 1, length, fd) > 0){

            //add NULL char
            data[length] = '\0';
            pkt_len = 1 + (strlen(data) > length ? strlen(data) : length ) + sizeof(unsigned int)*2 + sizeof(char);

            pkt = (char *) realloc(pkt, pkt_len);
            int nseqNo = htonl(seqNo);
            int nlength = htonl( (strlen(data) > length ? strlen(data) : length ));
            memset(pkt, 'D', 1);
            memcpy(pkt + 1, &nseqNo, 4);
            memcpy(pkt + 5, &nlength, 4);
            strcpy(pkt + 9, data);


            cli_addr.sin_family = AF_INET;
            cli_addr.sin_port = htons(requester_port);
            bzero(&(cli_addr.sin_zero), 8);

            if (sendto(sock, pkt, pkt_len, 0,
                      (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1){
                printf("Error sending data to client\n");
                return 1;
            }else{
                gettimeofday(&tp, NULL);
                printf("Time:");
                timeval_print_s(&tp);
                printf("IP:%s\n", inet_ntoa(cli_addr.sin_addr));
                printf("SeqNo:%ld\n", seqNo);
                char *pay4 = (char *) malloc(5*sizeof(char));
                memcpy(pay4, pkt + 9, 4);
                pay4[4] = '\0';
                printf("First 4 bytes:%s\n\n", pay4);
                seqNo += length;
                free(pay4);
            }
            sleep(rate);
        }

        memset(pkt, 'E', 1);
        if (sendto(sock, pkt, pkt_len, 0,
                  (struct sockaddr *) &cli_addr, sizeof(cli_addr)) == -1){
            printf("Error sending end pkt to client\n");
            return 1;
        }
        free(pkt);
    }
    fclose(fd);
    close(sock);
    return 0;
}

