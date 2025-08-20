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
#define PORT 443



int main(int argc,char** argv){


    if(argc<4){
        printf("Incorrect usage:%s <hostname(WITHOUT www. OR HTTPS://)> <path> <outputfilename>\n",argv[0]);
        return 0;
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
    argv[2],argv[1]);




    if((socketfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        printf("SOCKET CREATION FAILED\n");
        SSL_CTX_free(ctx);
        return -1;
    }
    char* server_ip=getIp(argv[1]);
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
    SSL_set_tlsext_host_name(ssl, argv[1]);
    int status =SSL_connect(ssl);
    if(status<0){
        ERR_print_errors_fp(stderr);
    }

    printf("Connected to %s with %s\n", argv[1], SSL_get_cipher(ssl));


    // int fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);


    if(SSL_write(ssl,request,request_len)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    bytes_recv=SSL_read(ssl,HEADER_BUFFER,HEADER_BUFFER_SIZE-1);
    HEADER_BUFFER[bytes_recv]='\0';

    http_res response;
    handle_headers(HEADER_BUFFER,&response);
    printf("%d\n",response.content_len);
    printf("%d\n",response.http_status);
    printf("%s\n",response.content_type);


    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socketfd);
    SSL_CTX_free(ctx);
    // close(fd);
    return 0;
}