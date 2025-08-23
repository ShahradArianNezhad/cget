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
        char* link = argv[1];
        if(strstr(argv[1],"https://")!=NULL){
            link+=8;
        }

        char *temp = link;
        while(*temp!='/'){
            temp++;
        }
        host = malloc(temp-link+1);
        strncpy(host,link,temp-link);
        host[temp-link]='\0';

        char* ptr = temp;

        while(*ptr!='\0'){
            ptr++;
        }
        path = malloc(ptr-temp+1);
        strncpy(path,temp,ptr-temp);
        path[ptr-temp]='\0';



        file_name = link;
        while(*file_name!='\0'){
            file_name++;
        }
        while(*(file_name-1)!='/'){
            file_name--;
        }
    }else if(argc==4){
        if(*argv[2]=='-'&&*(argv[2]+1)=='o'){
            char *temp = argv[1];
            while(*temp!='/'){
                temp++;
            }
            host = malloc(temp-argv[1]+1);
            strncpy(host,argv[1],temp-argv[1]);
            host[temp-argv[1]]='\0';

            char* ptr = temp;

            while(*ptr!='\0'){
                ptr++;
            }
            path = malloc(ptr-temp+1);
            strncpy(path,temp,ptr-temp);
            path[ptr-temp]='\0';
            file_name=argv[3];
        }else{
            printf("Incorrect usage:%s <hostname(WITHOUT www.)> [-o OUTPUT] \n",argv[0]);
            return 1;
        }

    }else{
        printf("Incorrect usage:%s <hostname(WITHOUT www. OR HTTPS://)> [-o OUTPUT] \n",argv[0]);
        return 1;
    }


    char *HEADER_BUFFER = malloc(HEADER_BUFFER_SIZE);
    HEADER_BUFFER[HEADER_BUFFER_SIZE-1]='\0';
    if(!HEADER_BUFFER){
        printf("MEMORY ALLOCATION FAILED\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    int socketfd;
    SSL *ssl;
    SSL_CTX *ctx;

    ctx = create_ssl_ctx();
    if(!ctx){
        printf("SSL CONTEXT CREATION FAILED\n");
        return -5;
    }
    char request[512];
    int request_len = snprintf(request,sizeof(request),
    "GET %s HTTP/1.1\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36\r\n"
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




    if(SSL_write(ssl,request,request_len)<0){
        printf("send failed\n");
        return -3;
    }

    int bytes_recv;
    bytes_recv=SSL_read(ssl,HEADER_BUFFER,HEADER_BUFFER_SIZE);

    http_res response;
    
    char* ptr;
    float total_bytes_recv=0.0;
    FILE* filefd;

    if((ptr=handle_headers(HEADER_BUFFER,&response))==0){
        return 1;
    }else if(response.http_status==200){
        FILE* checker = fopen(file_name,"r");
        if(checker!=NULL){
            printf("a file with the name %s already exists do you want to overwrite it? (y/n)\n",file_name);
            char* usrinp= malloc(50*sizeof(char));
            scanf("%s",usrinp);
            if(usrinp[0]=='y'){
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
                remove(file_name);
            }else if(usrinp[0]=='n'){
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
                printf("enter a new filename to use\n");
                scanf("%s",usrinp);
                file_name=usrinp;
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
                printf("\033[1A");
                printf("\033[2K");
            }else{
                printf("input unrecognized quitting...\n");
                return 1;
            }
        }
        fclose(checker);
        filefd = fopen(file_name,"ab+");
        total_bytes_recv+=bytes_recv-(ptr-HEADER_BUFFER);
        fwrite(ptr,sizeof(char),bytes_recv-(ptr-HEADER_BUFFER),filefd);
    }
    else{
        printf("ERROR: Server responded with non 200 response code : %d\n",response.http_status);
        return 1;
    }
    free(HEADER_BUFFER);

    // char* content = malloc(response.content_len*sizeof(char));


    // SSL_write(ssl,request,request_len);
    char* BUFFER = malloc(BUFFER_SIZE);
    uint8_t counter=0;
    while((bytes_recv=SSL_read(ssl,BUFFER,BUFFER_SIZE))>0){
        fwrite(BUFFER,sizeof(char),bytes_recv,filefd);
        total_bytes_recv+=bytes_recv;
        if(total_bytes_recv/response.content_len*100>=counter*10){
            counter++;
            printf("[");
            for(int i=0;i<counter-1;i++){
                printf("**");
            }
            for(int i=0;i<=10-counter;i++){
                printf("--");
            }
            printf("] ");
            printf("%.0f%% downloaded\n",total_bytes_recv/response.content_len*100);
            printf("\033[1A");
            printf("\033[2K");
        }
    }
    printf("Downloaded %s\n",file_name);

    free(response.content_type);
    free(BUFFER);
    free(host);
    free(path);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(socketfd);
    SSL_CTX_free(ctx);
    fclose(filefd);
    return 0;
}