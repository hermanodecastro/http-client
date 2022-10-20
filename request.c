#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

const int DEFAULT_HTTP_PORT = 80;

int beginsWithW3 (char *string);
int sizeOfString (char *string);
char *concatenate (char *origin, char *source);
char *createHttpHeader (char *address);
char *getDomain (char *address);

int main (int argc, char **argv) {
    if (argc < 2) {
        printf ("Missing address!\n");
        return 1;
    }

    char *address = argv[1];

    if (beginsWithW3 (address)) {
        printf ("Please, insert the url without \"www\"\nInstead \"www.example.com\" use \"example.com\"\n");
        return 0;
    }

    struct hostent *addressInfo = gethostbyname (getDomain (address));
    
    if (addressInfo == NULL) {
        perror("addressInfo ");
        fprintf(stderr, "ERROR Could not resolve address: %s\n", getDomain (address));
        return 1;
    }

    char *ip = inet_ntoa (* (struct in_addr*) addressInfo->h_addr_list[0]);

    int socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd < 0) {
        perror("bind ");
        fprintf(stderr, "ERROR couldn't create socket\n");
        return 1;
    }

    struct sockaddr_in serverAddress;

    memset (&serverAddress, 0, sizeof (serverAddress));

    serverAddress.sin_addr.s_addr = inet_addr (ip);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons (DEFAULT_HTTP_PORT);

    if (connect (socketFd, (struct sockaddr*) &serverAddress, sizeof (serverAddress)) < 0) {
        perror("connect ");
        fprintf(stderr, "ERROR couldn't connect\n");
        return 1;
    }

    char *header = createHttpHeader (address);

    send (socketFd, header, strlen (header), 0);

    char buffer[1024 * 10];

    recv (socketFd, buffer, sizeof (buffer), 0);
    printf("Response:\n\n%s", buffer);

    return 0;
}

int beginsWithW3 (char *address) {
   return address[0] == 'w' &&
    address[1] == 'w' && 
    address[2] == 'w' &&
    address[3] == '.';
}

int sizeOfString (char *string) {
    int size = 0;

    for (int i = 0; string[i] != '\0'; i++) {
        size++;
    }

    return size;
}

char *concatenate (char *origin, char *source) {
    int concatenatedStringSize = sizeOfString (origin) + sizeOfString (source);
    char *concatenatedString = (char *) malloc (concatenatedStringSize * sizeof (char));
    
    int endString = 0;

    for (int i = 0; origin[i] != '\0'; i++) {
        concatenatedString[i] = origin[i];
        endString++;
    }

    for (int i = 0, j = endString; source[i] != '\0'; i++, j++) {
        concatenatedString[j] = source[i];
        endString++;
    }

    concatenatedString[endString] = '\0';

    return concatenatedString;
}

char *createHttpHeader (char *address) {
    return concatenate ("GET /", 
            concatenate (address, 
                concatenate (" HTTP/1.1\r\nHost: ",
                    concatenate (getDomain(address), "\r\n\r\n"))));
}

char *getDomain (char *address) {
    int domainSize = 0;
    
    for (int i = 0; address[i] != '/' && address[i] != '?' && address[i] != '\0'; i++) {
        domainSize++;
    }

    char *domain = (char *) malloc (domainSize * sizeof (char));

    for (int i = 0; i < domainSize; i++) {
        domain[i] = address[i];
    }

    return domain;
}

