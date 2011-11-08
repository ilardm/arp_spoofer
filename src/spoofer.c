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
#include <memory.h>

#include "spoofer.h"
#include "utils.h"

int spf_arp_callback(unsigned char* _packet, int _len)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	ARP_PACKET* pack = (ARP_PACKET*)_packet;

	char shw[BUFSZ];
	char thw[BUFSZ];
	char sip[BUFSZ];
	char tip[BUFSZ];
	memset( shw, 0, sizeof(shw) );
	memset( thw, 0, sizeof(thw) );
	memset( sip, 0, sizeof(sip) );
	memset( tip, 0, sizeof(tip) );

	printf("eth: %s -> %s (%04x)\n",
		u_hw2str( pack->eth_hdr.ether_shost, ETH_ALEN, shw, BUFSZ ),
		u_hw2str( pack->eth_hdr.ether_dhost, ETH_ALEN, thw, BUFSZ ),
		ntohs( pack->eth_hdr.ether_type )
		);
	
	memset( shw, 0, sizeof(shw) );
	memset( thw, 0, sizeof(thw) );

	printf("arp: ");
	if ( ntohs(pack->arp_hdr.ar_op) == ARPOP_REQUEST )
	{
		// ARP request
		printf("who has %s (%s) tell %s (%s)\n",
			u_ip2str( pack->arp_data.tip, 4, tip, BUFSZ ),
			u_hw2str( pack->arp_data.thw, ETH_ALEN, thw, BUFSZ ),
			u_ip2str( pack->arp_data.sip, 4, sip, BUFSZ ),
			u_hw2str( pack->arp_data.shw, ETH_ALEN, shw, BUFSZ )
			);
	}
	else if ( ntohs(pack->arp_hdr.ar_op) == ARPOP_REPLY )
	{
		// ARP reply
		printf("%s is at %s -> %s (%s)\n",
			u_ip2str( pack->arp_data.sip, 4, sip, BUFSZ ),
			u_hw2str( pack->arp_data.shw, ETH_ALEN, shw, BUFSZ ),
			u_ip2str( pack->arp_data.tip, 4, tip, BUFSZ ),
			u_hw2str( pack->arp_data.thw, ETH_ALEN, thw, BUFSZ )
			);

	}
	else
	{
		printf("unknown opcode\n");

		#ifdef _DEBUG
		printf("hex:");
		u_hexout( _packet, _len );
		#endif
	}

	printf("\n");

	return 1;
}
