#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>


char* getIp(char* address){


    struct hostent* he;
    he = gethostbyname(address);

    return inet_ntoa(*(struct in_addr*)he->h_addr_list[0]);
}



