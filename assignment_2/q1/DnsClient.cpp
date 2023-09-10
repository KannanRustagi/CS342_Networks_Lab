#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>

// Ip address of the DNS Server
// Google DNS = 8.8.8.8
// IITG DNS = 172.17.1.1
char dnsServer[100] = "172.17.1.1";

// DNS header structure
struct dns_header
{
    unsigned short id;        // identification number
    unsigned char rd : 1;     // recursion desired
    unsigned char tc : 1;     // truncated message
    unsigned char aa : 1;     // authoritive answer
    unsigned char opcode : 4; // purpose of message
    unsigned char qr : 1;     // query/response flag
    unsigned char rcode : 4;  // response code
    unsigned char cd : 1;     // checking disabled
    unsigned char ad : 1;     // authenticated data
    unsigned char z : 1;      // its z! reserved
    unsigned char ra : 1;     // recursion available
    unsigned short qCount;    // number of question entries
    unsigned short ansCount;  // number of answer entries
    unsigned short authCount; // number of authority entries
    unsigned short addCount;  // number of resource entries
};

// Constant sized fields of query structure
struct question
{
    unsigned short qtype;
    unsigned short qclass;
};

// Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct r_data
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
#pragma pack(pop)

// Pointers to resource record contents
struct res_record
{
    unsigned char *name;
    struct r_data *resource;
    unsigned char *rdata;
};

// Structure of a Query
struct query
{
    unsigned char *name;
    struct question *ques;
};

// Cache for DNS Queries
std::map<std::string, std::vector<std::string>> dnsCache;

// This will convert the domain name into DNS Format
// eg. www.x.com to 3www1x3com
void DnsFormat(unsigned char *dns, unsigned char *host)
{
    int prev = 0, i;
    strcat((char *)host, ".");

    for (i = 0; i < strlen((char *)host); i++)
    {
        if (host[i] == '.')
        {
            *dns++ = i - prev;
            for (; prev < i; prev++)
            {
                *dns++ = host[prev];
            }
            prev++;
        }
    }
    *dns++ = '\0';
}

// This reads the received answers to user readable format taking DNS compression in account and decoding from DNS Format
u_char *ReadName(unsigned char *reader, unsigned char *buffer, int *count)
{
    unsigned char *name;
    unsigned int p = 0, jumped = 0, offset;
    int i, j;

    *count = 1;
    name = (unsigned char *)malloc(256);

    name[0] = '\0';

    // read the names in 3www1x3com format
    while (*reader != 0)
    {
        if (*reader >= 192)
        {
            offset = (*reader) * 256 + *(reader + 1) - 49152; // 49152 = 11000000 00000000
            reader = buffer + offset - 1;
            jumped = 1; // we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++] = *reader;
        }

        reader = reader + 1;

        if (jumped == 0)
        {
            *count = *count + 1; // if we havent jumped to another location then we can count up
        }
    }

    name[p] = '\0'; // string complete
    if (jumped == 1)
    {
        *count = *count + 1; // number of steps we actually moved forward in the packet
    }

    // now convert 3www1x3com0 to www.x.com
    for (i = 0; i < (int)strlen((const char *)name); i++)
    {
        p = name[i];
        for (j = 0; j < (int)p; j++)
        {
            name[i] = name[i + 1];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[i - 1] = '\0'; // remove the last dot
    return name;
}

// Perform a DNS query by sending a packet
void FetchIpFromServer(unsigned char *host)
{
    unsigned char buf[65536], *qname, *reader;
    int i, j, stop, s;

    struct sockaddr_in a;

    struct res_record answers[20]; // struct to store answers received back in the query response
    struct sockaddr_in dest;

    struct dns_header *dns = NULL;
    struct question *qinfo = NULL;

    std::cout << "Resolving " << host;

    // Initialising UDP socket for DNS queries
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(dnsServer); // dns server

    // Set the DNS structure to standard queries
    dns = (struct dns_header *)&buf;

    dns->id = (unsigned short)htons(getpid());
    dns->qr = 0;     // This is a query
    dns->opcode = 0; // This is a standard query
    dns->aa = 0;     // Not Authoritative
    dns->tc = 0;     // This message is not truncated
    dns->rd = 1;     // Recursion Desired
    dns->ra = 0;     // Recursion not available
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->qCount = htons(1);
    dns->ansCount = 0;
    dns->authCount = 0;
    dns->addCount = 0;

    // point to the query portion
    qname = (unsigned char *)&buf[sizeof(struct dns_header)];

    DnsFormat(qname, host);
    qinfo = (struct question *)&buf[sizeof(struct dns_header) + (strlen((const char *)qname) + 1)]; // fill it

    qinfo->qtype = htons(1); // type of the query , here A, i.e 1
    qinfo->qclass = htons(1);

    std::cout << ("\nSending Packet...");
    if (sendto(s, (char *)buf, sizeof(struct dns_header) + (strlen((const char *)qname) + 1) + sizeof(struct question), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
    {
        std::cout << "Error: sendto failed" << std::endl;
    }
    std::cout << "Done" << std::endl;

    // Receive the answer
    i = sizeof dest;
    std::cout << ("\nReceiving answer...");
    if (recvfrom(s, (char *)buf, 65536, 0, (struct sockaddr *)&dest, (socklen_t *)&i) < 0)
    {
        std::cout << "Error: recvfrom failed" << std::endl;
    }
    std::cout << "Done" << std::endl;

    dns = (struct dns_header *)buf;

    // move ahead of the dns header and the query field
    reader = &buf[sizeof(struct dns_header) + (strlen((const char *)qname) + 1) + sizeof(struct question)];

    std::cout << "The response contains : " << std::endl;
    std::cout << ntohs(dns->qCount) << " Questions" << std::endl;
    std::cout << ntohs(dns->ansCount) << " Answers" << std::endl;

    // Start reading answers
    stop = 0;

    for (i = 0; i < ntohs(dns->ansCount); i++)
    {
        answers[i].name = ReadName(reader, buf, &stop);
        reader = reader + stop;

        answers[i].resource = (struct r_data *)(reader);
        reader = reader + sizeof(struct r_data);

        if (ntohs(answers[i].resource->type) == 1) // if its an ipv4 address
        {
            answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len));

            for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
            {
                answers[i].rdata[j] = reader[j];
            }

            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = ReadName(reader, buf, &stop);
            reader = reader + stop;
        }
    }

    // print answers
    printf("\nAnswer Records : %d \n", ntohs(dns->ansCount));
    std::string domain(reinterpret_cast<char *>(host));
    domain.pop_back();
    for (i = 0; i < ntohs(dns->ansCount); i++)
    {
        printf("Name : %s ", answers[i].name);
        if (ntohs(answers[i].resource->type) == 1) // IPv4 address
        {
            long *p;
            p = (long *)answers[i].rdata;
            a.sin_addr.s_addr = (*p); // working without ntohl
            std::string Ip(reinterpret_cast<char *>(inet_ntoa(a.sin_addr)));
            printf("has IPv4 address : %s", inet_ntoa(a.sin_addr));
            dnsCache[domain].push_back(Ip);
        }
        std::cout << std::endl;
    }

    return;
}

// Function to execute DNS queries, return IP from cache if present else sends a query to the DNS server
void dnsQuery(u_char *hostname)
{
    std::string domain(reinterpret_cast<char *>(hostname));
    // check if present in cache
    if (dnsCache.count(domain))
    {
        std::cout << "Served from Cache\n";
        for (auto i : dnsCache[domain])
        {
            std::cout << domain << ": " << i << std::endl;
        }
    }
    // send to DNS server if not in cache
    else
    {
        std::cout << "Not in cache" << std::endl;
        FetchIpFromServer(hostname);
    }
}

int main()
{
    unsigned char hostname[100];

    while (1)
    {
        // Get the domain name from the terminal
        std::cout << ("Enter Hostname to Lookup : ");
        std::cin >> hostname;

        // Now get the ip of this hostname , A record
        dnsQuery(hostname);
        std::cout << std::endl;
    }
    return 0;
}
