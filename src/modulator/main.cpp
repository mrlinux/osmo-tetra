/*
 * Textbook implementation of a PI/4 DQPSK modulator for TETRA
 * *
 * Copyright 2010 maintech GmbH - cd@maintech.de
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "modulator.h"

static int usage()
{
	fprintf(stderr,
		"call with: tetramod freq-in-hz\n"
		"\n"
		"feed data via stdin in the following format:\n"
		"  255 bytes with values between 0x00 and 0x03 (all symbols for a full burst)\n"
		"  or 255 bytes with value 0xff (will transmit silence)\n"
		"always work with full bursts\n");

	return 1;
}

int main(int argc, char* argv[])
{
	float freq;

	if(argc != 2)
		return usage();

	freq = atof(argv[1]);

	Modulator modulator(freq);
	modulator.run();

	return 0;
}
