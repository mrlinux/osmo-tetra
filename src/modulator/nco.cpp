/*
 * Dead Simple Numerically Controlled Oscillator
 *
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
#include <math.h>
#include "nco.h"

NCO::NCO()
{
	m_ph = 0.0f;
}

void NCO::setFreq(float freq, float sampleRate)
{
	m_phInc = 2.0 * M_PI / (sampleRate / freq);
	//fprintf(stderr, "NCO phase inc %f\n", m_phInc);
}

Complex NCO::nextIQ()
{
	Complex res = Complex(cos(m_ph), sin(m_ph));
	m_ph += m_phInc;

	while(m_ph < -M_PI)
		m_ph += 2.0 * M_PI;
	while(m_ph >= M_PI)
		m_ph -= 2.0 * M_PI;

	return res;
}
