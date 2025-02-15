/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/* header includes */
#include "typedefs.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif
#ifdef WEBRTC_LINUX
#include <netinet/in.h>
#endif
#include <search.h>
#include <float.h>

#include "gtest/gtest.h"

/*********************/
/* Misc. definitions */
/*********************/

#define FIRSTLINELEN 40
#define CHECK_ZERO(a) {int errCode = a; if((errCode)!=0){fprintf(stderr,"\n %s \n line: %d \n error at %s\n Error Code = %d\n",__FILE__,__LINE__,#a, WebRtcNetEQ_GetErrorCode(inst)); exit(0);}}
#define CHECK_NOT_NULL(a) if((a)==NULL){fprintf(stderr,"\n %s \n line: %d \nerror at %s\n",__FILE__,__LINE__,#a );return(-1);}

/********************/
/* Global variables */
/********************/

FILE *in_file;
FILE *out_file;
FILE *dat_file;       


struct arr_time {
	float time;
	WebRtc_UWord32 ix;
};

int filelen(FILE *fid)
{
  fpos_t cur_pos;
  int len;

  if (!fid || fgetpos(fid, &cur_pos)) {
    return(-1);
  }

  fseek(fid, 0, SEEK_END);
  len = ftell(fid);

  fsetpos(fid, &cur_pos);

  return (len);
}

int compare_arr_time(const void *x, const void *y);

int main(int argc, char* argv[])
{
	unsigned int	dat_len, rtp_len, Npack, k;
	arr_time		*time_vec;
	char			firstline[FIRSTLINELEN];
	unsigned char	*rtp_vec = NULL, **packet_ptr, *temp_packet;
	const unsigned int kRtpDumpHeaderSize = 4 + 4 + 4 + 2 + 2;
	WebRtc_UWord16			len;
	WebRtc_UWord32			*offset;

/* check number of parameters */
	if (argc != 4) {
		/* print help text and exit */
		printf("Apply jitter on RTP stream.\n");
		printf("The program reads an RTP stream and packet timing from two files.\n");
		printf("The RTP stream is modified to have the same jitter as described in the timing files.\n");
		printf("The format of the RTP stream file should be the same as for rtpplay,\n");
		printf("and can be obtained e.g., from Ethereal by using\n");
		printf("Statistics -> RTP -> Show All Streams -> [select a stream] -> Save As\n\n");
		printf("Usage:\n\n");
		printf("%s RTP_infile dat_file RTP_outfile\n", argv[0]);
		printf("where:\n");

		printf("RTP_infile       : RTP stream input file\n\n");

		printf("dat_file         : file with packet arrival times in ms\n\n");

		printf("RTP_outfile      : RTP stream output file\n\n");

		return(0);
	}

	in_file=fopen(argv[1],"rb");
	CHECK_NOT_NULL(in_file);
	printf("Input file: %s\n",argv[1]);
	dat_file=fopen(argv[2],"rb");
	CHECK_NOT_NULL(dat_file);
	printf("Dat-file: %s\n",argv[2]);
	out_file=fopen(argv[3],"wb");
	CHECK_NOT_NULL(out_file);
	printf("Output file: %s\n\n",argv[3]);
	
	time_vec = (arr_time *) malloc(sizeof(arr_time)*(filelen(dat_file)/sizeof(float)) + 1000); // add 1000 bytes to avoid (rare) strange error
	if (time_vec==NULL) {
		fprintf(stderr, "Error: could not allocate memory for reading dat file\n");
		goto closing;
	}

	dat_len=0;
	while(fread(&(time_vec[dat_len].time),sizeof(float),1,dat_file)>0) {
		time_vec[dat_len].ix=dat_len;
		dat_len++;
	}
	
	qsort(time_vec,dat_len,sizeof(arr_time),compare_arr_time);


	rtp_vec = (unsigned char *) malloc(sizeof(unsigned char)*filelen(in_file));
	if (rtp_vec==NULL) {
		fprintf(stderr,"Error: could not allocate memory for reading rtp file\n");
		goto closing;
	}

	// read file header and write directly to output file
	EXPECT_TRUE(fgets(firstline, FIRSTLINELEN, in_file) != NULL);
	EXPECT_GT(fputs(firstline, out_file), 0);
	EXPECT_EQ(kRtpDumpHeaderSize, fread(firstline, 1, kRtpDumpHeaderSize,
	                                    in_file));
	EXPECT_EQ(kRtpDumpHeaderSize, fwrite(firstline, 1, kRtpDumpHeaderSize,
	                                     out_file));

	// read all RTP packets into vector
	rtp_len=0;
	Npack=0;
	len=(WebRtc_UWord16) fread(&rtp_vec[rtp_len], sizeof(unsigned char), 2, in_file); // read length of first packet
	while(len==2) {
		len = ntohs(*((WebRtc_UWord16 *)(rtp_vec + rtp_len)));
		rtp_len += 2;
		if(fread(&rtp_vec[rtp_len], sizeof(unsigned char), len-2, in_file)!=(unsigned) (len-2)) {
			fprintf(stderr,"Error: currupt packet length\n");
			goto closing;
		}
		rtp_len += len-2;
		Npack++;
		len=(WebRtc_UWord16) fread(&rtp_vec[rtp_len], sizeof(unsigned char), 2, in_file); // read length of next packet
	}

	packet_ptr = (unsigned char **) malloc(Npack*sizeof(unsigned char*));

	packet_ptr[0]=rtp_vec;
	k=1;
	while(k<Npack) {
		len = ntohs(*((WebRtc_UWord16 *) packet_ptr[k-1]));
		packet_ptr[k]=packet_ptr[k-1]+len;
		k++;
	}

	for(k=0; k<dat_len && k<Npack; k++) {
		if(time_vec[k].time < FLT_MAX && time_vec[k].ix < Npack){ 
			temp_packet = packet_ptr[time_vec[k].ix];
			offset = (WebRtc_UWord32 *) (temp_packet+4);
			if ( time_vec[k].time >= 0 ) {
				*offset = htonl((WebRtc_UWord32) time_vec[k].time);
			}
			else {
				*offset = htonl((WebRtc_UWord32) 0);
				fprintf(stderr, "Warning: negative receive time in dat file transformed to 0.\n");
			}

			// write packet to file
			fwrite(temp_packet, sizeof(unsigned char), ntohs(*((WebRtc_UWord16*) temp_packet)), out_file);
		}
	}


closing:
	free(time_vec);
	free(rtp_vec);
	fclose(in_file);
	fclose(dat_file);
	fclose(out_file);

	return(0);
}



int compare_arr_time(const void *xp, const void *yp) {

	if(((arr_time *)xp)->time == ((arr_time *)yp)->time)
		return(0);
	else if(((arr_time *)xp)->time > ((arr_time *)yp)->time)
		return(1);

	return(-1);
}
