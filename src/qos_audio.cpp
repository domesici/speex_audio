/*------------------------------------------------------------------------
 *---------------------           RT-WMP              --------------------
 *------------------------------------------------------------------------
 *                                                         V7.0B  11/05/10
 *
 *
 *  File: ./example/linux_us/qos_audio/speex.c                            
 *  Authors: Danilo Tardioli                                              
 *  ----------------------------------------------------------------------
 *  Copyright (C) 2000-2010, Universidad de Zaragoza, SPAIN
 *
 *  Contact Addresses: Danilo Tardioli                   dantard@unizar.es
 *
 *  RT-WMP is free software; you can  redistribute it and/or  modify it
 *  under the terms of the GNU General Public License  as published by the
 *  Free Software Foundation;  either  version 2, or (at  your option) any
 *  later version.
 *
 *  RT-WMP  is distributed  in the  hope  that  it will be   useful, but
 *  WITHOUT  ANY  WARRANTY;     without  even the   implied   warranty  of
 *  MERCHANTABILITY  or  FITNESS FOR A  PARTICULAR PURPOSE.    See the GNU
 *  General Public License for more details.
 *
 *  You should have received  a  copy of  the  GNU General Public  License
 *  distributed with RT-WMP;  see file COPYING.   If not,  write to the
 *  Free Software  Foundation,  59 Temple Place  -  Suite 330,  Boston, MA
 *  02111-1307, USA.
 *
 *  As a  special exception, if you  link this  unit  with other  files to
 *  produce an   executable,   this unit  does  not  by  itself cause  the
 *  resulting executable to be covered by the  GNU General Public License.
 *  This exception does  not however invalidate  any other reasons why the
 *  executable file might be covered by the GNU Public License.
 *
 *----------------------------------------------------------------------*/

#include <speex/speex.h>
#include <stdio.h>
#include <stdlib.h>

/*The frame size in hardcoded for this sample code but it doesn't have to be*/

struct speexFrame{
	int len;
	char data;
};

SpeexBits ebits,dbits;
void  * eState, * dState;
float *output, *input;
int FRAME_SIZE;
int quality, enh;

void cmp_init(int frame_size, int q, int e){
	quality=q;
	enh=e;
	dState = speex_decoder_init(&speex_nb_mode);
	speex_decoder_ctl(dState, SPEEX_SET_ENH, &enh);
	speex_bits_init(&dbits);

	eState = speex_encoder_init(&speex_nb_mode);
	speex_encoder_ctl(eState, SPEEX_SET_QUALITY, &quality);       
	speex_bits_init(&ebits);

        output = (float*)calloc(frame_size, sizeof(float));    //cast
        input = (float*)calloc(frame_size, sizeof(float));

	FRAME_SIZE = frame_size;

}

int cmp_encode(char * inbuf, char * outbuf) {

	struct speexFrame *p =(struct speexFrame *) outbuf;
	short * in = (short*) inbuf;
	int i;

	for (i = 0; i < FRAME_SIZE; i++)
		input[i] = in[i];
	speex_bits_reset(&ebits);
	speex_encode(eState, input, &ebits);
	p->len = speex_bits_write(&ebits, &p->data, FRAME_SIZE);
	return (p->len + sizeof(int));
}


int cmp_decode(char * inbuf, char * outbuf) {
	struct speexFrame * p =(struct speexFrame *) inbuf;
	short * out= (short*) outbuf;
	int i;

	speex_bits_read_from(&dbits, &p->data, p->len);
	speex_decode(dState, &dbits, output);

	for (i = 0; i < FRAME_SIZE; i++)
		out[i] = (short) output[i];
	return FRAME_SIZE*2;
}


void cmp_close(){
	free(output);
	free(input);
}




