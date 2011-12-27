/*
 * Complex Root Raised Cosine filter
 *
 * heavily inspired and parts simply ripped and reformatted from
 * GNU Radio source by Christian Daniel cd@maintech.de
 *
 * Copyright 2002,2007,2008 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <math.h>
#include "rootraisedcosinefilter.h"

void RootRaisedCosineFilter::free()
{
	if(m_h != NULL) {
		delete[] m_h;
		m_h = NULL;
	}
	if(m_filterI) {
		delete[] m_filterI;
		m_filterI = NULL;
	}
	if(m_filterQ) {
		delete[] m_filterQ;
		m_filterQ = NULL;
	}
}

RootRaisedCosineFilter::RootRaisedCosineFilter() :
	m_h(NULL),
	m_filterI(NULL),
	m_filterQ(NULL)
{
}

RootRaisedCosineFilter::~RootRaisedCosineFilter()
{
	free();
}

void RootRaisedCosineFilter::init(float gain, float samplingRate, float symbolRate, float alpha, int numTaps)
{
	float spb = samplingRate / symbolRate; // samples per bit/symbol
	float scale = 0.0;
	int i;

	free();

	if((numTaps & 1) == 0) {
		fprintf(stderr, "RootRaisedCosineFilter: order must be odd\n");
		numTaps++;
	}

	m_h = new float[numTaps];
	m_filterI = new float[numTaps];
	m_filterQ = new float[numTaps];
	m_len = numTaps;
	m_start = 0;
	for(i = 0; i < numTaps; i++) {
		m_filterI[i] = 0.0;
		m_filterQ[i] = 0.0;
	}

	for(i = 0; i < numTaps; i++) {
		float x1, x2, x3, num, den;
		float xindx = i - numTaps / 2;
		x1 = M_PI * xindx / spb;
		x2 = 4 * alpha * xindx / spb;
		x3 = x2 * x2 - 1;
		if(fabs(x3) >= 0.000001) { // Avoid Rounding errors...
			if(i != numTaps / 2)
				num = cos((1 + alpha) * x1) + sin((1 - alpha) * x1) / (4 * alpha * xindx / spb);
			else num = cos((1 + alpha) * x1) + (1 - alpha) * M_PI / (4 * alpha);
			den = x3 * M_PI;
		} else {
			if(alpha == 1) {
				m_h[i] = -1;
				continue;
			}
			x3 = (1 - alpha) * x1;
			x2 = (1 + alpha) * x1;
			num = (sin(x2) * (1 + alpha) * M_PI - cos(x3) * ((1 - alpha) * M_PI * spb) / (4 * alpha * xindx) + sin(x3) * spb * spb / (4 * alpha * xindx * xindx));
			den = -32 * M_PI * alpha * alpha * xindx / spb;
		}
		m_h[i] = 4 * alpha * num / den;
		scale += m_h[i];
	}

	for(i = 0; i < numTaps; i++)
		m_h[i] = m_h[i] * gain / scale;
}

void RootRaisedCosineFilter::dumpResponse()
{
	int i;

	if(m_h == NULL)
		return;

	for(i = 0; i < m_len; i++)
		printf("%d %f\n", i, m_h[i]);
}

Complex RootRaisedCosineFilter::filter(Complex sample)
{
	int i;
	int pos;
	float accI = 0.0;
	float accQ = 0.0;

	m_filterI[m_start] = sample.real();
	m_filterQ[m_start] = sample.imag();
	m_start++;
	if(m_start >= m_len)
		m_start = 0;

	pos = m_start;
	for(i = 0; i < m_len; i++) {
		accI += m_filterI[pos] * m_h[i];
		accQ += m_filterQ[pos] * m_h[i];
		pos++;
		if(pos >= m_len)
			pos = 0;
	}

	return Complex(accI, accQ);
}
