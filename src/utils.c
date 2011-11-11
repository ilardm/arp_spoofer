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
#include <sys/types.h>
#include <string.h>

#include "utils.h"

void u_hexout(unsigned char* _data, int _len)
{
	/* #ifdef _DEBUG */
	/* printf("== %s\n", */
	/* 	__PRETTY_FUNCTION__ */
	/* 	); */
	/* #endif */

	if ( !_data || _len <=0 )
	{
		return;
	}

	int i = 0;
	do
	{
		if ( i%16 == 0 )
		{
			printf("\t0x%04X: ", i);
		}

		if ( i%2 == 0 )
		{
			printf("%02x", _data[i]);
		}
		else
		{
			printf("%02x ", _data[i]);
		}

		if ( (i+1)%16 == 0 )
		{
			printf("\n");
		}

		i++;
	} while ( i<_len );

	printf("\n");
}

char* u_hw2str(u_int8_t* _addr, int _addrlen, char* _buf, size_t _bufsz)
{
	// bufsz: aa:bb:cc:dd:ee:ff => aa*2(symbols) + :*5 + 0x00(EOS)
	if ( !_addr || !_buf || _bufsz < ( _addrlen*2 + _addrlen-1 + 1 ) )
	{
		if ( _buf )
		{
			snprintf( _buf, 8, "(%s)", "error" );
		}
	}
	else
	{
		int i = 0;
		for ( i = 0; i < _addrlen; i++ )
		{
			if ( i != _addrlen-1 )
			{
				sprintf( _buf + (i*3), "%02x:", _addr[i] );
			}
			else
			{
				sprintf( _buf + (i*3), "%02x", _addr[i] );
			}
		}
	}

	return _buf;
}

char* u_ip2str(unsigned char* _addr, int _addrlen, char* _buf, size_t _bufsz)
{
	if ( !_addr || !_buf || _bufsz < ( _addrlen*3 + _addrlen-1 + 1 ) )
	{
		if ( _buf )
		{
			snprintf( _buf, 8, "(%s)", "error" );
		}
	}
	else
	{
		int i = 0;
		for ( i = 0; i < _addrlen; i++ )
		{
			if ( i != _addrlen-1 )
			{
				sprintf( _buf + (i*4), "%03d.", _addr[i] );
			}
			else
			{
				sprintf( _buf + (i*4), "%03d", _addr[i] );
			}
		}
	}
	return _buf;
}

u_int8_t* u_str2hw(char* _buf, size_t _buflen, u_int8_t* _addr, int _addrlen)
{
	// bufsz: aa:bb:cc:dd:ee:ff => aa*2(symbols) + :*5 + 0x00(EOS)
	if ( !_addr || !_buf || _addrlen < ETH_ALEN || _buflen < ( _addrlen*2 + _addrlen-1 ) )
	{
		return NULL;
	}
	else
	{
		int i = 0;
		int j = 0;
		unsigned int part = 0;
		for ( i = 0; i < _buflen; i+=3 )
		{
			if ( j > _addrlen )
			{
				break;
			}

			sscanf( _buf+i, "%02x", &part );
			*(_addr+j) = ( part & 0xFF );
			j++;
		}
	}

	return _addr;
}

unsigned char* u_str2ip(char* _buf, size_t _buflen, unsigned char* _addr, int _addrlen)
{
	// TODO: is buf len here valid ?
	if ( !_addr || !_buf ||
		_addrlen < sizeof(unsigned char)*4 ||
		_buflen < ( _addrlen + _addrlen-1 )
		)
	{
		return NULL;
	}
	else
	{
		char* pcur = _buf;
		char* pdot = strstr( _buf, "." );
		unsigned int part = 0;
		int p = 0;

		// FIXME: ugly cycle; no overflow chec
		while ( pcur )
		{
			if ( pdot )
			{
				*pdot = 0x00;
			}
			sscanf( pcur,  "%d", &part );
			*( _addr+p ) = ( part & 0xFF );

			if ( !pdot )
			{
				break;
			}
			pcur = pdot + 1;
			pdot = strstr( pcur, "." );
			p++;
		}
	}

	// FIXME:
	return _addr;
}
