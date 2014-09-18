/* ------------------------------------------------------------------------- *
 * afhba-llcontrol-example.c
 * ------------------------------------------------------------------------- *
 *   Copyright (C) 2014 Peter Milne, D-TACQ Solutions Ltd
 *                      <peter dot milne at D hyphen TACQ dot com>
 *                         www.d-tacq.com
 *   Created on: 18 September 2014
 *    Author: pgm
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of Version 2 of the GNU General Public License        *
 *  as published by the Free Software Foundation;                            *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program; if not, write to the Free Software              *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                */
/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>

#define HB_FILE "/dev/afhba.0.loc"
#define HB_LEN	0x1000

#define LOG_FILE	"afhba.log"

void* host_buffer;
int nsamples		10000000		/* 10s at 1MSPS */

FILE* fp_log;

#define CH01 (((volatile short*)host_buffer)[0])
#define CH02 (((volatile short*)host_buffer)[1])
#define CH03 (((volatile short*)host_buffer)[2])
#define CH04 (((volatile short*)host_buffer)[3])
#define TLATCH (((volatile unsigned*)host_buffer)[2])	/* actually, sample counter */
#define SPAD1	(((volatile unsigned*)host_buffer)[3])   /* user signal from ACQ */

void get_mapping() {
	void *va = mmap(0, HB_LEN, PROT_READ|PROT_WRITE, MAP_SHARED, HB_FILE, 0);
	if (va == (caddr_t)-1 ){
		perror( "mmap" );
	        _exit(errno);
	}else{
		host_buffer = va;
	}
}

void goRealTime(void)
{
	struct sched_param p;
	p.sched_priority = sched_fifo_priority;


	int rc = sched_setscheduler(0, SCHED_FIFO, &p);

	if (rc){
		perror("failed to set RT priority");
	}
}

void ui(int argc, char* argv[])
{
	// this ui is kind of basic ..
}

void setup()
{
	get_mapping();
	//goRealTime();
}

void run()
{
	unsigned tl0 = TLATCH;
	unsigned spad1_0 = SPAD1;
	unsigned spad1_1;
	unsigned tl1;
	int sample;

	for (sample = 0; sample < nsamples; ++sample, tl0 = tl1){
		while((tl1 = TLATCH) == tl0){
			yield();
		}
		if (sample%10000 == 0){
			printf("[%d] %u\n", sample, tl1);
		}
		if (spad1_1 != spad1_0){
			printf("[%d] %u => %u\n", sample, spad1_0, spad1_1);
			spad1_0 = spad1_1;
		}
		fwrite(host_buffer, sizeof(short), 8, fp_log);
	}
}
int main(int argc, char* argv[])
{
	ui(argc, argv[]);
	setup();
	run();
}