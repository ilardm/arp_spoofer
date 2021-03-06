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

// FIXME: headers

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
//#include <linux/if_ether.h>		// ETH_P_ARP
#include <net/ethernet.h>			// ETHERTYPE_ARP
#include <arpa/inet.h>			// 
#include <netpacket/packet.h>	// sockaddr_ll

#include "pf.h"
#include "utils.h"
#include "spoofer.h"

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

	// create socket
	prop->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if ( prop->sock == -1 )
	{
		fprintf( stderr, "create socket error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}
	// TODO: do we need this?
	/* int on = 1; */
	/* if ( setsockopt( prop->sock, SOL_SOCKET, SO_DONTROUTE, &on, sizeof(on) ) < 0 ) */
	/* { */
	/* 	fprintf(stderr, "setsockopt error %d %s\n", */
	/* 		errno, strerror(errno) ); */
	/* 	pf_deinit(prop); */
	/* 	return NULL; */
	/* } */

	// prepare to bind
	struct ifreq iface;
	memset( &iface, 0, sizeof(struct ifreq) );
	strncpy( iface.ifr_name, _devname, IFNAMSIZ );

	if ( ioctl( prop->sock, SIOCGIFMTU, &iface ) == -1 )
	{
		fprintf( stderr, "ioctl error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}
	prop->mtu = iface.ifr_mtu;

	// fill own hw && ip
	if ( ioctl( prop->sock, SIOCGIFHWADDR, &iface ) == -1 )
	{
		fprintf( stderr, "ioctl error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}
	struct sockaddr tmphw;
	memcpy( &(tmphw), &(iface.ifr_hwaddr), sizeof(tmphw) );
	memcpy( &(prop->own_hw), ((char*)&(tmphw))+2, sizeof(prop->own_hw) );	// FIXME: magicnumbers
 
	if ( ioctl( prop->sock, SIOCGIFADDR, &iface ) == -1 )
	{
		fprintf( stderr, "ioctl error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}
	struct sockaddr tmpip;
	memcpy( &(tmpip), &(iface.ifr_addr), sizeof(tmpip) );
	memcpy( &(prop->own_ip), ((char*)&(tmpip))+4, sizeof(prop->own_ip) );	// FIXME: magicnumbers

	if ( ioctl( prop->sock, SIOCGIFINDEX, &iface ) == -1 )
	{
		fprintf( stderr, "iocl error: (%d) %s\n",
			errno, strerror(errno) );
		pf_deinit(prop);
		return NULL;
	}
	prop->iface_idx = iface.ifr_ifindex;

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

int pf_add_callback(PF_PROPERTIES* _properties, u_int16_t _ptype, int (*_callback)(unsigned char*, int, void*))
{
#ifdef _DEBUG
	printf("== %s (0x%08x, %04x, 0x%08x)\n",
		__PRETTY_FUNCTION__,
		(unsigned int)_properties,
		_ptype,
		(unsigned int)_callback
		);
#endif

	if ( !_properties )
	{
		return 1;
	}

	PF_CALLBACKS* pcur = _properties->hooks;
	PF_CALLBACKS* pprev = _properties->hooks;

	int i=0;
	while ( pcur )
	{
		pprev = pcur;
		pcur = pcur->next;
		i++;
	}
	pcur = pprev;

	PF_CALLBACKS* clb_new = (PF_CALLBACKS*)calloc(1, sizeof(PF_CALLBACKS));
	if ( !clb_new )
	{
		fprintf( stderr, "-- error in new callback allocation\n" );
		return 1;
	}

	if ( !pcur )
	{
		// no root callback
		pcur = clb_new;
		_properties->hooks = pcur;
	}
	else
	{
		// append tail callback
		pcur->next = clb_new;
		pcur = clb_new;
	}

	printf("++ add %d callback\n", i);
	pcur->packet_type = _ptype;
	pcur->callback = _callback;

	return 0;
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

	if ( !_properties )
	{
		return 1;
	}

	if ( _properties->pf_thrd )
	{
		return 0;
	}

	pthread_mutex_lock( &pfpropertiesMX );
	_properties->shutdown = 0;
	pthread_mutex_unlock( &pfpropertiesMX );

	int es = pthread_create( &(_properties->pf_thrd), NULL, pf_reciever, (void*)_properties );
	if ( es != 0 )
	{
		fprintf(stderr, "-- error on pthread_create: (%d) %s\n",
			es, strerror(es) );

		_properties->pf_thrd = 0;

		return 1;
	}

	return 0;
}

int pf_stop(PF_PROPERTIES* _properties)
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

	// TODO
	if ( _properties->pf_thrd )
	{
		pthread_mutex_lock( &pfpropertiesMX );
		_properties->shutdown = 1;
		pthread_mutex_unlock( &pfpropertiesMX );

		pthread_join( _properties->pf_thrd, NULL );

		return 0;
	}

	return 1;
}

void* pf_reciever(void* args)
{
	PF_PROPERTIES* prop = (PF_PROPERTIES*)args;
	if ( !prop )
	{
		fprintf( stderr, "-- reciever args NULL\n");
		return NULL;
	}
	int sock = prop->sock;
	fd_set fds;
	struct timeval tv;
	int mtu = prop->mtu;
	char* shutdown = &(prop->shutdown);
	char shutdownLocal = 0;
	PF_CALLBACKS* clb_root = prop->hooks;
	PF_CALLBACKS* clb_cur = NULL;

	unsigned char* rcvbuf = (unsigned char*)calloc( 1, prop->mtu );
	if ( !rcvbuf )
	{
		fprintf( stderr, "-- can't allocate rcvbuf\n");
	}

	tv.tv_sec = 0;
	tv.tv_usec = 50;

	int rcvlen = 0;
	struct sockaddr pack_info;
	socklen_t pack_info_len;

	struct ether_header *ethhdr = NULL;
	u_int16_t packtype = ETHERTYPE_IP;

	// rcv loop

	while ( 1 )
	{
		shutdownLocal = 0;
		pthread_mutex_lock( &pfpropertiesMX );
		if ( *shutdown )
		{
			//break;
			shutdownLocal = 1;
		}
		pthread_mutex_unlock( &pfpropertiesMX );
		if ( shutdownLocal )
		{
			break;
		}

		FD_ZERO( &fds );
		FD_SET( sock, &fds );
		if ( select( (sock+1), &fds, NULL, NULL, &tv )>0 )
		{
			if ( FD_ISSET( sock, &fds ) )
			{
				// TODO: recieve packets and process 'em
				/* printf("++ %s\n", __PRETTY_FUNCTION__); */
				// TODO: select()
				memset( &pack_info, 0, sizeof(struct sockaddr) );
				pack_info_len = sizeof(pack_info);
				rcvlen = recvfrom( sock, rcvbuf, mtu,
									0, &pack_info, &pack_info_len );

				if ( rcvlen == -1 )
				{
					fprintf(stderr, "-- err on rcvfrom: (%d) %s\n",
						errno, strerror(errno) );
					break;
				}
				ethhdr = (struct ether_header *)rcvbuf;
				/* printf("----------\trecvfrom (%04x): %d\n", ethhdr->ether_type, rcvlen); */
				/* u_hexout(rcvbuf, rcvlen); */
				/* printf("\n"); */

				packtype = ntohs( ethhdr->ether_type );
				clb_cur = clb_root;
				int i=0;
				while ( clb_cur )
				{
					/* printf("++ checking %d callback (%04x == %04x)\n", i, packtype, clb_cur->packet_type); */
					if ( clb_cur->packet_type == packtype )
					{
						/* printf("++ using %d callback\n", i); */
						clb_cur->callback( rcvbuf, rcvlen, args );
						break;
					}
					
					clb_cur = clb_cur->next;
					i++;
				}
			}
		}
		usleep(50);
	}

	printf("++ leave pf thread\n");
	free( rcvbuf );

	return NULL;
}
