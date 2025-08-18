#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/ip.h"
#define PORT 80



int main(char argc,char** argv){

    struct sockaddr_in server_addr;
    int socketfd;




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



    char request[256];

    int request_len = snprintf(request,sizeof(request),
    "GET / HTTP/1.1\r\n"
    "Host: %s\r\n\r\n"
    "Connection:close\r\n"
    "\r\n",
    argv[1]);

    if(send(socketfd,request,request_len,0)<0){
        printf("send failed\n");
    }


    char res[2048];
    while(recv(socketfd,res,2048,0)>0){
    printf("%s\n",res);
    }

    return 0;
}