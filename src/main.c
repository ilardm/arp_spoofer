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
#include <string.h>

#include "pf.h"

int main(int argc, char** argv)
{
	/* copyrights and version info */
	printf("ARP spoofer");
	#ifdef _VERSION
	printf(" v %s", _VERSION);
	#endif
	printf(", built on %s %s \n", __DATE__, __TIME__);
	printf("(c) 2011 Ilya Arefiev <arefiev.id@gmail.com>, license: BSD license\n");
	printf("\n");
	/* copyright and version info */

	#ifdef _DEBUG
	printf("startup\n");
	#endif

	if ( argc < 2 )
	{
		printf("not enought params\n");
		return 1;
	}

	// TODO
	PF_PROPERTIES* prop = pf_init( argv[1] );
	
	if ( !prop )
	{
		fprintf(stderr, "-- PF init error\n");
		return 1;	// TODO: exit code enum
	}

	// console
	// TODO: init, start, stop here
	char line[128];
	memset( line, 0, 128 );
	do
	{
		printf("> ");
		scanf( "%s", line );

		if ( strcmp( line, "start" ) == 0 )
		{
			if ( pf_start(prop) != 0 )
			{
				fprintf(stderr, "-- PF start error\n");
				pf_deinit(prop);

				return 1;	// TODO: exit code enum
			}

			continue;
		}
	} while ( strcmp( line, "stop" ) != 0 );

	/* if ( pf_start(prop) != 0 ) */
	/* { */
	/* 	fprintf(stderr, "-- PF start error\n"); */
	/* 	pf_deinit(prop); */

	/* 	return 1;	// TODO: exit code enum */
	/* } */

	if ( pf_stop(prop) != 0 )
	{
		fprintf(stderr, "-- PF stop error\n");
		pf_deinit(prop);

		return 1;	// TODO: exit code enum
	}

	if ( pf_deinit(prop) != 0 )
	{
		fprintf( stderr, "-- PF deinit error\n" );
		return 1;
	}

	#ifdef _DEBUG
	printf("shutdown\n");
	#endif

	return 0;	// TODO: exit code enum
}
