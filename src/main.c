#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../include/network.h"
#define HEADER_BUFFER_SIZE 8192
#define BUFFER_SIZE 1048576
#define PORT 443



int main(int argc,char** argv){

    char* file_name;
    char* host;
    char* path;
    if(argc==2){
        char *temp = argv[1];
        while(*temp!='/'){
            temp++;
        }
        host = malloc(temp-argv[1]+1);
        strncpy(host,argv[1],temp-argv[1]);
        host[temp-argv[1]]='\0';
        printf("%s",host);

        char* ptr = temp;

        while(*ptr!='\0'){
            ptr++;
        }
        path = malloc(ptr-temp+1);
        strncpy(path,temp,ptr-temp);
        path[ptr-temp]='\0';



        file_name = argv[1];
        while(*file_name!='\0'){
            file_name++;
        }
        while(*(file_name-1)!='/'){
            file_name--;
        }
    }
    else if(argc==3){
        file_name = argv[2];
        path= argv[2];
        host = argv[1];
        while(*file_name!='\0'){
            file_name++;
        }
        while(*(file_name-1)!='/'){
            file_name--;
        }
    }
    else if(argc==4){
        file_name =argv[3];
        path= argv[2];
        host = argv[1];
    }else{
        printf("Incorrect usage:%s <hostname(WITHOUT www. OR HTTPS://)> <path> <outputfilename>\n",argv[0]);
        return 1;
    }


    char *HEADER_BUFFER = malloc(HEADER_BUFFER_SIZE);
    HEADER_BUFFER[HEADER_BUFFER_SIZE-1]='\0';
    if(!HEADER_BUFFER){
        printf("MEMORY ALLOCATION FAILED");
        return -1;
    }


    struct sockaddr_in server_addr;
    int socketfd;
    SSL *ssl;
    SSL_CTX *ctx;

    ctx = create_ssl_ctx();
    if(!ctx){
        printf("SSL CONTEXT CREATION FAILED");
        return -5;
    }
    char request[256];
    int request_len = snprintf(request,sizeof(request),
    "GET %s HTTP/1.1\r\n"
    "User-Agent: cdow/1.0\r\n"
    "Accept: */*\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    // "Range: bytes=0-60\r\n"
    "\r\n",
    path,host);




    if((socketfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("SOCKET CREATION FAILED\n");
        SSL_CTX_free(ctx);
        return -1;
    }
    char* server_ip=getIp(host);
    if(!server_ip){
        printf("DNS RESOLOUTION FAILED: make sure you dont include http:// or www. in your hostname\n");
        return -2;
    }
    
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);


    if(connect(socketfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
        printf("CONNECTION FAILED\n");
        SSL_CTX_free(ctx);
        return -2;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl,socketfd);
    SSL_set_tlsext_host_name(ssl, host);
    int status =SSL_connect(ssl);
    if(status<0){
        ERR_print_errors_fp(stderr);
    }

    printf("Connected to %s with %s\n", host, SSL_get_cipher(ssl));


    int filefd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);


    if(SSL_write(ssl,request,request_len)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    bytes_recv=SSL_read(ssl,HEADER_BUFFER,HEADER_BUFFER_SIZE);

    http_res response;
    
    if(handle_headers(HEADER_BUFFER,&response)==0){
        return 1;
    }
    if(response.http_status!=200){
        printf("ERROR: Server responded with non 200 response code : %d",response.http_status);
        return 1;
    }

    // char* content = malloc(response.content_len*sizeof(char));


    // SSL_write(ssl,request,request_len);
    char* BUFFER = malloc(BUFFER_SIZE);

    while((bytes_recv=SSL_read(ssl,BUFFER,BUFFER_SIZE))>0){
        write(filefd,BUFFER,bytes_recv);
    }
    




    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socketfd);
    SSL_CTX_free(ctx);
    close(filefd);
    return 0;
}