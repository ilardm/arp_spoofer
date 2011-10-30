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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>		// ETH_P_ARP
#include <arpa/inet.h>			// 
#include <netpacket/packet.h>	// sockaddr_ll

#include "pf.h"

PF_PROPERTIES* pf_init(char* _devname)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	// FIXME: memleaks (@ error return)

	// init properties
	PF_PROPERTIES* prop = (PF_PROPERTIES*)calloc(1, sizeof(PF_PROPERTIES));
	if ( !prop )
	{
		fprintf( stderr, "-- error in properties allocation\n" );
		return NULL;
	}

	// set up callbacks
	PF_CALLBACKS* clb_root = (PF_CALLBACKS*)calloc(1, sizeof(PF_CALLBACKS));
	if ( !clb_root )
	{
		fprintf( stderr, "-- error in callbacks allocation\n" );
		if ( prop )
		{
			free(prop);
		}
		free(clb_root);
		return NULL;
	}
	clb_root->packet_type = ETH_P_ARP;
	clb_root->callback = &pf_arp_callback;

	prop->hooks = clb_root;

	// create socket
	prop->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if ( prop->sock == -1 )
	{
		fprintf( stderr, "create socket error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}

	// prepare to bind
	struct ifreq iface;
	memset( &iface, 0, sizeof(struct ifreq) );

	strncpy( iface.ifr_name, _devname, IFNAMSIZ );
	if ( ioctl( prop->sock, SIOCGIFINDEX, &iface ) == -1 )
	{
		fprintf( stderr, "iocl error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}

	// bind to iface
	struct sockaddr_ll sll;
	memset( &sll, 0, sizeof(struct sockaddr_ll) );

	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = iface.ifr_ifindex;
	sll.sll_protocol = htons( ETH_P_ALL );

	if ( bind( prop->sock, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll) ) == -1 )
	{
		fprintf( stderr, "bind error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}

	return prop;
}

int pf_deinit(PF_PROPERTIES* _properties)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif
	
	if ( !_properties )
	{
		return 1;
	}

	PF_CALLBACKS *clb_root, *clb_cur;
	clb_root = _properties->hooks;
	clb_cur = _properties->hooks;
	while ( clb_root )
	{
		clb_root = clb_root->next;
		free(clb_cur);
		clb_cur = clb_root;
	}

	free(_properties);
	
	return 0;
}

int pf_start(PF_PROPERTIES* _properties)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	// TODO

	return 1;
}

int pf_stop()
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	// TODO

	return 1;
}

int pf_arp_callback(char* _packet, int _len)
{
#ifdef _DEBUG
	printf("== %s\n",
		__PRETTY_FUNCTION__
		);
#endif

	// TODO: 

	return 1;
}
