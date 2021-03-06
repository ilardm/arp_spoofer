// Copyright (c) 2011, Ilya Arefiev <arefiev.id@gmail.com>
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//  * Neither the name of the author nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <net/ethernet.h>
#include <net/if_arp.h>

#include <arpa/inet.h>		// inet_ntoa()
#include <netinet/ether.h>	//ether_ntoa()

#define BUFSZ 256

typedef struct ARP_PACKET_STRUCT
{
	struct ether_header	eth_hdr;
	struct arphdr		arp_hdr;
	struct arp_data	// FIXME: currently IPv4 only
	{
		unsigned char shw[ETH_ALEN];
		unsigned char sip[4];
		unsigned char thw[ETH_ALEN];
		unsigned char tip[4];
	} arp_data;
} ARP_PACKET;

void u_hexout(void*, int);

// inet_ntoa (3) ether_ntoa (3) uses statically allocated char buffers
// following functions copying this buffer into passed as argument
char* u_hw2str(struct ether_addr*, char*, size_t);
char* u_ip2str(struct in_addr*, char*, size_t);

#endif
