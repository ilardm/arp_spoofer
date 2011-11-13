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
#include <memory.h>
#include <time.h>

#include "spoofer.h"
#include "utils.h"

int spf_arp_callback(unsigned char* _packet, int _len)
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

int spf_ip_callback(unsigned char* _packet, int _len)
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

	return 0;
}
