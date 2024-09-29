/*
=========================================
  Author  : Bitswif
  GitHub  : https://github.com/PoorKnowladge
  IG      : @bitswif
  Project : Create TCP Connection Manually with c++ and API(POSIX)
  Date    : 7 - 09 - 2024
  Desc    : The purpose of this code is to inspire innovation and invite network experts to further examine and improve upon any shortcomings in the code
  =========================================
  
  "Innovation meets simplicity." - Bitswif
  
=========================================
*/


#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <fcntl.h>

using namespace std;

struct pseudo_header {
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

int get_random_id() {
	int urandom_fd = open("/dev/urandom", O_RDONLY);
	if(urandom_fd < 0){
		cerr << "Failed to open /dev/urandom" << endl;
		return rand() % 65535;
	}
	
	unsigned short random_value;
	read(urandom_fd, &random_value, sizeof(random_value));
	close(urandom_fd);
	return random_value;
}

int get_random_port() {
	int urandom_fd = open("/dev/urandom", O_RDONLY);
	if(urandom_fd < 0){
		cerr << "Failed to open /dev/urandom" << endl;
		return rand() % 65535;
	}
	
	unsigned short random_value;
	read(urandom_fd, &random_value, sizeof(random_value));
	close(urandom_fd);
	return 1024 + (random_value % (65535 - 1024 + 1));
}

unsigned short checksum(void *b, int len){
	unsigned short* buf = (unsigned short *)b;
	unsigned int sum = 0;
	unsigned short result;
	
	for (sum = 0; len > 1; len -= 2)
		sum += *buf++;
	if(len == 1)
		sum += *(unsigned char *)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

int SendPacketWithRawSocket(){
	int sock;
	struct sockaddr_in dest_addr;
	char packet[4096];
	struct iphdr *iph = (struct iphdr *)packet;
	struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));
	struct pseudo_header psh;
	
	const char *destination_ip = "192.168.100.1";
	const char *source_ip	   = "192.168.100.2";
	const int source_port 	   = get_random_port();
	const int dest_port        = 80;
	
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(sock < 0){
		cerr << "Failed to create socket" << endl;
		return 1;
	}
	
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(dest_port);
	dest_addr.sin_addr.s_addr = inet_addr(destination_ip);
	
	memset(packet, 0, sizeof(packet));
	
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
	iph->id = htons(get_random_id());
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->saddr = inet_addr(source_ip);
	iph->daddr = dest_addr.sin_addr.s_addr;
	iph->check = 0;
	iph->check = checksum((unsigned short *)packet, iph->tot_len);
	
	tcph->source = htons(source_port);
	tcph->dest = htons(dest_port);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;
	tcph->syn = 1;
	tcph->window = htons(get_random_port());
	tcph->check = 0;
	tcph->urg_ptr = 0;
	
	psh.source_address = inet_addr(source_ip);
	psh.dest_address   = dest_addr.sin_addr.s_addr;
	psh.placeholder    = 0;
	psh.protocol       = IPPROTO_TCP;
	psh.tcp_length     = htons(sizeof(struct tcphdr));
	
	char pseudo_packet[sizeof(struct pseudo_header) + sizeof(struct tcphdr)];
	memcpy(pseudo_packet, &psh, sizeof(struct pseudo_header));
	memcpy(pseudo_packet + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr));
	
	tcph->check = checksum((unsigned short *)pseudo_packet, sizeof(pseudo_packet));
	
	if(sendto(sock, packet, iph->tot_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
		cerr << "Send failed" << endl;
		return 1;
	}
	
	cout << "[ * ] TCP packet sent to " << destination_ip << " on port " << dest_port << endl;
	close(sock);
	
	return 0;
}

int main(){
	SendPacketWithRawSocket();
}
