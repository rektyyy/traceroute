#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <assert.h>
#include <stdlib.h>

u_int16_t compute_icmp_checksum(const void *buff, int length)
{
    const u_int16_t *ptr = buff;
    u_int32_t sum = 0;
    assert(length % 2 == 0);
    for (; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16U) + (sum & 0xffffU);
    return (u_int16_t)(~(sum + (sum >> 16U)));
}

int send_packets(int sock_fd, struct sockaddr_in recipient, int pid, int ttl, int howmany)
{
    if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) != 0)
    {
        printf("Could not set TTL to the socket!");
        return EXIT_FAILURE;
    }

    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = pid;
    header.icmp_hun.ih_idseq.icd_seq = ttl;
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum((u_int16_t *)&header, sizeof(header));
    for (int i = 0; i < howmany; i++)
    {
        if (sendto(sock_fd, &header, sizeof(header), 0, (struct sockaddr *)&recipient, sizeof(recipient)) == -1)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}