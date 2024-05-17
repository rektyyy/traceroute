#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

#define TIME_TO_WAIT 1000

void prettyprint(int responses, int expected_responses, char ip_addr[][20], int time, int ttl)
{
    if (responses == 0)
    {
        printf("%d. *\n", ttl);
    }
    else
    {
        printf("%d. ", ttl);
        // Check for responses from multiple ip addresses
        for (int i = 0; i < responses; i++)
        {
            int check = 0;
            for (int j = 0; j < i; j++)
                if (strcmp(ip_addr[i], ip_addr[j]) == 0)
                {
                    check = 1;
                    break;
                }
            if (check == 0)
            {
                printf("%s ", ip_addr[i]);
            }
        }

        if (responses == expected_responses)
        {
            printf(" %dms\n", time);
        }
        else
        {
            printf(" ???\n");
        }
    }
}

int recieve_packets(int sock_fd, struct sockaddr_in recipient, int expected_pid, int expected_ttl, char *expected_ip, int packet_count, struct timeval start)
{

    u_int8_t buffer[IP_MAXPACKET];
    char ip_str[packet_count][20];

    struct timeval end;
    struct pollfd ps;
    ps.fd = sock_fd;
    ps.events = POLLIN;
    ps.revents = 0;

    uint16_t pid;
    uint16_t ttl;

    int responses = 0;
    while (responses < packet_count)
    {
        int ready = poll(&ps, 1, TIME_TO_WAIT);
        if (ready == 0)
        {
            break;
        }
        else if (ready < 0)
        {
            printf("Polling error!\n");
            return EXIT_FAILURE;
        }
        else
        {

            if (ps.revents & POLLIN)
            {
                socklen_t recipient_len = sizeof(recipient);

                ssize_t packet_len = recvfrom(sock_fd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&recipient, &recipient_len);
                if (packet_len < 0)
                {
                    printf("Recvfrom error: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
                if (inet_ntop(AF_INET, &(recipient.sin_addr), ip_str[responses], sizeof(ip_str[responses])) == NULL)
                {
                    printf("Inet_ntop error!\n");
                    return EXIT_FAILURE;
                }

                struct ip *ip_header = (struct ip *)buffer;
                struct icmp *icmp_header = (struct icmp *)(buffer + 4 * (ssize_t)(ip_header->ip_hl));

                pid = icmp_header->icmp_hun.ih_idseq.icd_id;
                ttl = icmp_header->icmp_hun.ih_idseq.icd_seq;

                if (icmp_header->icmp_type & ICMP_TIME_EXCEEDED)
                {
                    ip_header = (struct ip *)((u_int8_t *)icmp_header + 8);
                    icmp_header = (struct icmp *)((u_int8_t *)ip_header + ip_header->ip_hl * 4);

                    pid = icmp_header->icmp_hun.ih_idseq.icd_id;
                    ttl = icmp_header->icmp_hun.ih_idseq.icd_seq;
                }

                // More instances of program can work at the same time
                if (pid != expected_pid || ttl != expected_ttl)
                    continue;

                responses++;
            }
        }
    }
    gettimeofday(&end, NULL);
    int time = (double)(end.tv_usec - start.tv_usec) / 1000 + (double)(end.tv_sec - start.tv_sec) * 1000;
    prettyprint(responses, packet_count, ip_str, time, expected_ttl);

    int reached_dest = 0;
    for (int i = 0; i < packet_count; i++)
    {
        reached_dest += strcmp(ip_str[i], expected_ip);
    }
    if (reached_dest == 0) return EXIT_SUCCESS;
    else return -1;
}