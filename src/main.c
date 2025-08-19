#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../include/ip.h"
#define HEADER_BUFFER_SIZE 8192  
#define PORT 443


SSL_CTX* create_ssl_ctx(){
    SSL_CTX *ctx;

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(TLS_client_method());
    if(!ctx){
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);


    SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER,NULL);
    SSL_CTX_load_verify_locations(ctx,NULL,"/etc/ssl/certs");

    return ctx;
}





int main(int argc,char** argv){


    if(argc<4){
        printf("Incorrect usage:%s <hostname(WITHOUT www. OR HTTPS://)> <path> <outputfilename>\n",argv[0]);
        return 0;
    }


    char *HEADER_BUFFER = malloc(HEADER_BUFFER_SIZE);
    if(!HEADER_BUFFER){
        printf("MEMORY ALLOCATION FAILED");
        return -1;
    }

    char request[256];
    struct sockaddr_in server_addr;
    int socketfd;
    SSL *ssl;
    SSL_CTX *ctx;

    ctx = create_ssl_ctx();
    if(!ctx){
        printf("SSL CONTEXT CREATION FAILED");
        return -5;
    }



    int request_len = snprintf(request,sizeof(request),
    "GET %s HTTP/1.1\r\n"
    "User-Agent: cdow/1.0\r\n"
    "Accept: */*\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
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
        printf("DNS RESOLOUTION FAILED: make sure you dont include http:// or www. in your hostname");
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


    int fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);


    if(SSL_write(ssl,request,request_len)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    char* temp;
    bytes_recv=SSL_read(ssl,HEADER_BUFFER,HEADER_BUFFER_SIZE-1);
    HEADER_BUFFER[HEADER_BUFFER_SIZE]='\0';

    // printf("%s",HEADER_BUFFER);
    // if((temp = strstr(HEADER_BUFFER,"\r\n\r\n"))!=NULL){
    //     int header_size = temp-HEADER_BUFFER+4;
    //     write(fd,temp,bytes_recv-header_size);
    //     char* cont_len_ptr = strstr(HEADER_BUFFER,"content-length:");
    //     if(cont_len_ptr!=NULL){
    //         // skip the 'content-lenght:'
    //         cont_len_ptr+=16;

    //         // content lenght in bytes
    //         char cont_len[15];
    //         int int_cont_len;

    //         for(int i=0;i<50;i++){
    //             if((*(cont_len_ptr+i))=='\r'&&(*(cont_len_ptr+i+1))=='\n'){
    //                 break;
    //             }
    //             cont_len[i]=*(cont_len_ptr+i);
    //         }
    //     }
    // }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socketfd);
    SSL_CTX_free(ctx);
    close(fd);
    return 0;
}