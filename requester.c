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
#include <sys/queue.h>
#include <sys/time.h>
#include "utility.h"

#define min(a,b) (((a) < (b)) ? (a): (b))

struct host{
    int piece;
    char *name;
    int port;
    LIST_ENTRY(host) pointers;
};

LIST_HEAD(list, host);

int main(int argc, char **argv){
    if (argc != 5) {
            printf("Usage: requester -p <port> -o <file option>\n");
            return 1;
    }
    int sock = 0;
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
    int pkt_len;

    //File tracking business
    FILE *fd = NULL;
    if ((fd = fopen("tracker.txt", "r")) == NULL){
        printf("Error opening file tracker.txt\n");
        return 1;
    }

    char *file;
    char *serv;
    int serv_port = 0;
    int sent = 0;
    unsigned int piece;
    unsigned int zero = 0;
    struct addrinfo hints;
    struct timeval time_start;
    struct timeval time_end;
    struct timeval time_result;
    FILE *fp = fopen(option, "w");
    int max_id = 1;
    int cur = 1;
    struct host *cpy;
    LIST_HEAD(list, host) head;
    LIST_INIT(&head);
    memset(&hints, 0, sizeof(hints));

    while(getline(&buf, &n, fd) != -1){
        file = strtok(buf," ");           
        piece = strtoul(strtok(NULL," "), NULL, 10);           
        serv = strtok(NULL," ");           
        serv_port = strtoul(strtok(NULL," "), NULL, 10);           
        if(strcmp(file, option) == 0){
            if ( piece > max_id) { max_id = piece;} 
            struct host *item = malloc(sizeof(struct host));
            item->piece = piece;
            item->name = serv;
            item->port = serv_port;
            LIST_INSERT_HEAD(&head, item, pointers);
            cpy = item;
        }
        free(buf);
        buf = NULL;
    }
    free(file);
    file = NULL;
    while(cur <= max_id){
        LIST_FOREACH(cpy, &head, pointers){
            if(cpy->piece == cur){
                int total_pkts = 0;
                int total_data_bytes = 0;
                float avg_pkts_sec = 0;
                //getaddrinfo(serv, NULL, &hints, &res);
                struct hostent *host = (struct hostent *) gethostbyname((char *) cpy->name);       


                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(cpy->port);
                serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
                bzero(&(serv_addr.sin_zero), 8);
                pkt_len = 1 + strlen(option) + sizeof(unsigned int)*2 + sizeof(char);

                pkt = (char *) malloc(pkt_len);
                piece = htonl(cpy->piece);
                zero = htonl(zero);
                memset(pkt, 'R', 1);
                memcpy(pkt + 1, &piece, 4);
                memcpy(pkt + 5, &zero, 4);
                strcpy(pkt + 9, option);

                if((sent = sendto(sock, pkt, pkt_len, 0, (struct sockaddr *)
                          &serv_addr, sizeof(serv_addr))) != pkt_len){
                    printf("Error sending to server:%s\n", strerror(errno));
                    return 1;
                }
                free(pkt);     
                //HACK: This prevents from receiving pkts larger
                //than 1500
                pkt_len = 1500;

                
                int returned = recvfrom(sock, buf, pkt_len, MSG_PEEK, (struct sockaddr *)
                             &serv_addr, &serv_addr_len); 
                pkt_len = min(returned, pkt_len);
                buf = (char *)realloc(buf, pkt_len);
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

                    fwrite(buf + 9, pay_len, 1, fp);

                    gettimeofday(&time_result, NULL);
                    printf("Time:");
                    timeval_print_s(&time_result);
                    printf("IP:%s\n", inet_ntoa(serv_addr.sin_addr));
                    int seq = 0;
                    int len = 0;
                    memcpy(&seq, buf + 1, 4);
                    memcpy(&len, buf + 5, 4);
                    printf("SeqNo:%d\n", ntohl(seq));
                    printf("Length:%d\n", ntohl(len));
                    char *data = (char *) malloc(5*sizeof(char));
                    memcpy(data, buf + 9, 4);
                    data[4] = '\0';
                    printf("First 4 bytes:%s\n\n", data);
                    free(data);
                
                    int returned = recvfrom(sock, buf, pkt_len, MSG_PEEK, (struct sockaddr *)
                                 &serv_addr, &serv_addr_len); 
                    pkt_len = min(returned, pkt_len);
                    buf = (char *)realloc(buf, pkt_len);
                    if(( reply = recvfrom(sock, buf, pkt_len, 0, (struct sockaddr *)
                                          &serv_addr, &serv_addr_len)) != pkt_len) {
                        printf("Error receiving data\n");
                        return 1;
                    }
                }
                free(buf);
                buf = NULL;
                printf("Total packets:%d\n", total_pkts);
                printf("Total data bytes:%d\n", total_data_bytes);
                gettimeofday(&time_end, NULL);
                timeval_subtract(&time_result, &time_end, &time_start);
                avg_pkts_sec = (float)total_pkts / (float)(time_result.tv_sec + (float)time_result.tv_usec / 1000000.f); 
                printf("Average pkts/sec:%f\n", avg_pkts_sec);
                printf("Total time:");
                struct timeval *ptr = &time_result;
                timeval_print(ptr);
                printf("sec\n");



                cur++;
                break;
            }

        }

    }
    fclose(fd);
    close(sock);
    return 0;
}
