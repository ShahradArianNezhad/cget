#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct{
    int http_status;
    int content_len;
    char* content_type;
}http_res;

char* getIp(char* address) {
    struct hostent* he;
    static char ip[INET_ADDRSTRLEN];
    
    he = gethostbyname(address);
    if (he == NULL) {
        herror("gethostbyname");
        return NULL;
    }
    
    if (he->h_addr_list[0] == NULL) {
        fprintf(stderr, "No address found for %s\n", address);
        return NULL;
    }
    
    // Convert binary IP to string
    inet_ntop(AF_INET, he->h_addr_list[0], ip, INET_ADDRSTRLEN);
    return ip;
}

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



int handle_headers(char* buff,http_res* res){



    char* window_start;
    char* window_end;

    window_start = strstr(buff,"HTTP");
    while(*window_start!=' '){
        window_start++;
    }
    window_start++;
    window_end = window_start;
    while(*window_end!=' '){
        window_end++;
    }
    window_end--;

    int http_status = strtol(window_start,&window_end,10);
    res->http_status = http_status;


    window_start = strstr(buff,"Content-Length");
    while(*window_start!=' '){
        window_start++;
    }
    window_start++;
    window_end = window_start;
    while(*window_end!='\r'){
        window_end++;
    }
    window_end--;

    int content_len = strtol(window_start,&window_end,10);
    res->content_len=content_len;


    window_start = strstr(buff,"Content-Type");
    while(*window_start!=' '){
        window_start++;
    }
    window_start++;
    window_end = window_start;
    while(*window_end!=';'){
        window_end++;
    }

    res->content_type = malloc(20*sizeof(char));

    strncpy(res->content_type,window_start,window_end-window_start);
    res->content_type[window_end-window_start+1]='\0';


    return http_status;

}