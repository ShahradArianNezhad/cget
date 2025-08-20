char* getIp(char* address);

typedef struct{
    int http_status;
    int content_len;
    char* content_type;
}http_res;

SSL_CTX* create_ssl_ctx();

int handle_headers(char* buff,http_res* res);


