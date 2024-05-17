#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "send.h"
#include "recieve.h"

#define MAX_TTL 30
#define PACKET_COUNT 3

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Only one argument is needed(IP address)\n");
        return EXIT_FAILURE;
    }

    int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock_fd < 0)
    {
        printf("Socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;

    // Converting ip
    int errcheck = inet_pton(AF_INET, argv[1], &recipient.sin_addr);
    if (errcheck != 1)
    {
        if (errcheck == -1)
        {
            printf("Given IP address is invalid!\n");
        }
        else
        {
            printf("Not in presentation format!\n");
        }
        return EXIT_FAILURE;
    }

    int pid = getpid() & 0xFFFF;
    for (int i = 1; i < MAX_TTL; i++)
    {
        // Measure time before sending and after recieving
        struct timeval start;
        gettimeofday(&start, NULL);
        if (send_packets(sock_fd, recipient, pid, i, PACKET_COUNT) == EXIT_FAILURE)
        {
            printf("Could not send packet!\n");
            return EXIT_FAILURE;
        }
        int response = recieve_packets(sock_fd, recipient, pid, i, argv[1], PACKET_COUNT, start);
        if (response == EXIT_FAILURE){
            printf("Error in recieving packages!\n");
            return EXIT_FAILURE;
        }
        if (response == 0)
            break;
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}