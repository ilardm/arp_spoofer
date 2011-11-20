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

#ifndef PF_H
#define PF_H

// FIXME: headers
#include <pthread.h>
#include <sys/types.h>

typedef struct PF_CALLBACKS
{
	u_int16_t packet_type;
	int (*callback)(unsigned char*, int, void*);

	struct PF_CALLBACKS *next;
} PF_CALLBACKS;

typedef struct PF_PROPERTIES
{
	int sock;
	int mtu;

	struct ether_addr own_hw;
	struct in_addr own_ip;

	PF_CALLBACKS* hooks;

	char shutdown;
	pthread_t pf_thrd;
} PF_PROPERTIES;
static pthread_mutex_t pfpropertiesMX = PTHREAD_MUTEX_INITIALIZER;

PF_PROPERTIES* pf_init(char*);
int pf_add_callback(PF_PROPERTIES*, u_int16_t, int (*_callback)(unsigned char*, int, void*));
int pf_deinit(PF_PROPERTIES*);
int pf_start(PF_PROPERTIES*);
int pf_stop(PF_PROPERTIES*);

void* pf_reciever(void*);

#endif
