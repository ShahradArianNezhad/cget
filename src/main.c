#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include "../include/ip.h"
#define PORT 80



int main(char argc,char** argv){

    char res[2048];
    char request[256];
    struct sockaddr_in server_addr;
    int socketfd;

    int request_len = snprintf(request,sizeof(request),
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection:close\r\n"
    "\r\n",
    argv[2],argv[1]);



    if((socketfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("SOCKET CREATION FAILED\n");
        return -1;
    }
    
    inet_pton(AF_INET, getIp(argv[1]), &server_addr.sin_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);


    if(connect(socketfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        printf("CONNECTION FAILED\n");
        return -2;
    }

    int fd = open("meeeeez", O_WRONLY | O_CREAT | O_TRUNC, 0644);





    // printf("%s",request);
    if(send(socketfd,request,request_len,0)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    int hit_header=0;
    char* temp;
    while((bytes_recv=recv(socketfd,res,2048,0))>0){
        if(hit_header==1){
            write(fd,res,bytes_recv);
            continue;
        }
        if((temp = strstr(res,"\r\n\r\n"))!=NULL){
            hit_header=1;
            int header_size = temp-res+4;
            write(fd,temp,bytes_recv-header_size);
            // printf("%s\n",temp);
            continue;
        }

    }

    return 0;
}