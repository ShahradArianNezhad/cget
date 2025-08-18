#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "../include/ip.h"
#define PORT 80



int main(int argc,char** argv){


    if(argc<4){
        printf("Incorrect usage:%s <hostname> <path> <outputfilename>\n",argv[0]);
        return 0;
    }


    char res[2048];
    char request[256];
    struct sockaddr_in server_addr;
    int socketfd;

    int request_len = snprintf(request,sizeof(request),
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection:close\r\n"
    // "Range: bytes=0-60\r\n"
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


    int fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);


    if(send(socketfd,request,request_len,0)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    char* temp;
    bytes_recv=recv(socketfd,res,2048,0);
    if((temp = strstr(res,"\r\n\r\n"))!=NULL){
        int header_size = temp-res+4;
        write(fd,temp,bytes_recv-header_size);
        char* cont_len_ptr = strstr(res,"content-length:");

        // skip the 'content-lenght:'
        cont_len_ptr+=16;

        // content lenght in bytes
        char cont_len[15];
        int int_cont_len;

        for(int i=0;i<50;i++){
            if((*(cont_len_ptr+i))=='\r'&&(*(cont_len_ptr+i+1))=='\n'){
                break;
            }
            cont_len[i]=*(cont_len_ptr+i);
        }
        sscanf(cont_len,"%d",&int_cont_len);
        printf("%d\n",int_cont_len);
        
    }
    return 0;
}