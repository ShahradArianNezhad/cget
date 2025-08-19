#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

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