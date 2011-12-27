/*
 * Textbook implementation of a PI/4 DQPSK modulator for TETRA
 *
 * USRP is set to 50k below TX frequency and a prerotated signal is
 * transmitted. This moves the I/Q imbalance out of the main signal.
 * Please beware: I/Q imbalance is still there and can contain
 * considerable energy - most probably somewhere, where you do not want
 * to get caught transmitting.
 *
 * USRP is running at 250kSamples/s and the modulator produces 14 samples
 * per symbol. This results in 17857 symbols/s -- which is not 18000 as
 * required by TETRA. If this doesn't work with actual hardware, I will
 * add a fractional resampler
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <usrp/usrp_bytesex.h>
#include "modulator.h"

// possible constellation values (QPSK and QPSK+PI/4 interleaved)
const float Modulator::m_constellations[8][2] = {
	{ 1.0f, 0.0f, },
	{ 0.707107f, 0.707107f },
	{ 0.0f, 1.0f, },
	{ -0.707107f, 0.707107f },
	{ -1.0f, 0.0f },
	{ -0.707107f, -0.707107f },
	{ 0.0f, -1.0f },
	{ 0.707107f, -0.707107f }
};

// constellation transitions:
// be in constellation 3 and send a 1 -> new constellation is in [3][1] == 6 == 0.0/-1.0
#if 1
const uint8_t Modulator::m_transitions[8][4] = {
	{ 1, 3, 7, 5 }, // 0
	{ 2, 4, 0, 6 }, // 1
	{ 3, 5, 1, 7 }, // 2
	{ 4, 6, 2, 0 }, // 3
	{ 5, 7, 3, 1 }, // 4
	{ 6, 0, 4, 2 }, // 5
	{ 7, 1, 5, 3 }, // 6
	{ 0, 2, 6, 4 }, // 7
};
#endif
#if 0
const uint8_t Modulator::m_transitions[8][4] = {
	{ 1, 3, 5, 7 },
	{ 2, 4, 6, 0 },
	{ 3, 5, 7, 1 },
	{ 4, 6, 0, 2 },
	{ 5, 7, 1, 3 },
	{ 6, 0, 2, 4 },
	{ 7, 1, 3, 5 },
	{ 0, 2, 4, 6 },
};
#endif

void Modulator::getNextBurst()
{
	// read data for next burst
	int res = read(STDIN_FILENO, m_burst, BurstSymbols);
	if(res != BurstSymbols) {
		memset(m_burst, 0xff, BurstSymbols);
	}
}

uint8_t Modulator::getNextSymbol()
{
	// get next symbol from burst buffer
	if((m_burstIndex < 0) || (m_burstIndex >= BurstSymbols)) {
		m_burstIndex = 0;
		getNextBurst();
	}
	return m_burst[m_burstIndex++];
}

Complex Modulator::getNextConstellation()
{
	int symbol = getNextSymbol();

	// 0xff == silence == TX off
	if(symbol == 0xff) {
		return Complex(0.0f, 0.0f);
	} else {
		m_currentConstellation = m_transitions[m_currentConstellation][symbol];
		return Complex(m_constellations[m_currentConstellation][0], m_constellations[m_currentConstellation][1]);
	}
}

Complex Modulator::getNextSample()
{
	Complex sample;

	// upsampler: converts 1 symbol to 14 samples
	if(m_symbolSampleNum == 0)
		sample = m_rrcf.filter(getNextConstellation());
	else sample = m_rrcf.filter(Complex(0.0f, 0.0f));

	m_symbolSampleNum++;
	if(m_symbolSampleNum >= 14)
		m_symbolSampleNum = 0;

	// frequency shift by 50kHz
	sample *= m_carrier.nextIQ();

	return sample;
}

/* stay long in office and the madness blossoms */

/* was ist orange und läuft den berg hoch? - eine wanderine */
/* was ist rot und steht am straßenrand? - eine hagenutte */
/* was hüpft durch den wald und raucht? - ein kaminchen */
/* was ist grün und trägt kopftuch? - eine gürkin */
/* was ist schwarz und weiß und springt von eisscholle zu eisscholle - ein springuin */
/* was ist süß und steht in der wüste - ein karamel */

Modulator::Modulator(float freq) :
	m_burstIndex(-1),
	m_currentConstellation(0),
	m_symbolSampleNum(0),
	m_sampleNum(0)
{
	// RRC as pulse shaper
	m_rrcf.init(9.0f, 14.0f, 1.0f, 0.35f, 11 * 14 + 1);
	// 50kHz offset - USRP gets set to 50k below the wanted frequency
	m_carrier.setFreq(50000.0f, 250000.0f);

	// configure USRP to 250kSamples/s
	m_usrpTX = usrp_standard_tx::make(0, 512, 1, -1, 1024, 4);
	if(m_usrpTX == NULL) {
		fprintf(stderr, "USRP error\n");
		return;
	}
	usrp_subdev_spec txSpec(0, 0);
	m_txBoard = m_usrpTX->selected_subdev(txSpec);

	// tune the monster
	usrp_tune_result txTuneResult;
	if(!m_usrpTX->tune(m_txBoard->which(), m_txBoard, freq - 50000.0f, &txTuneResult)) {
		fprintf(stderr, "USRP boom!\n");
		return;
	}
	fprintf(stderr, "Tune result: %f MHz, (BB: %f MHz, DXC: %f MHz, residual: %f MHz\n",
		freq - 50000.0f,
		txTuneResult.baseband_freq,
		txTuneResult.dxc_freq,
		txTuneResult.residual_freq);

	// configure signal routing
	m_usrpTX->set_mux(m_usrpTX->determine_tx_mux_value(txSpec));
	m_txBoard->set_gain(m_txBoard->gain_max());
	m_txBoard->set_auto_tr(true);
	m_txBoard->set_atr_tx_delay(0);
	m_txBoard->set_atr_rx_delay(50);

	// I/Q calibration...
//	m_usrpTX->set_dac_offset(0, 51, 0);
//	m_usrpTX->set_dac_offset(1, 59, 0);

	// alloc space for samples
	m_txBlockSize = m_usrpTX->block_size();
	m_txBlock = new uint8_t[m_txBlockSize];
	m_txBlockFill = 0;
	m_txStarted = false;

	fprintf(stderr, "USRP TX block size: %d Bytes, Fs = %ld (I/Q %s)\n",
		m_usrpTX->block_size(),
		m_usrpTX->converter_rate(),
		m_txBoard->i_and_q_swapped() ? "swapped" : "normal");
}

void Modulator::run()
{
	uint64_t start;
	uint64_t now;
	timespec ts;
	int blocks;

	if(m_usrpTX == NULL)
		return;

	// get the time for sample rate measurement
	clock_gettime(CLOCK_MONOTONIC, &ts);
	start = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	blocks = 0;

	while(true) {
		Complex sample = getNextSample();

		((short*)(m_txBlock + m_txBlockFill))[0] = host_to_usrp_short(sample.real() * 30000.0 + 0.5);
		((short*)(m_txBlock + m_txBlockFill))[1] = host_to_usrp_short(sample.imag() * 30000.0 + 0.5);

		m_txBlockFill += 2 * sizeof(short);

		if(m_txBlockFill >= m_txBlockSize) {
			// a complete block is full -> send it to USRP
			bool underflow = false;
			if(!m_txStarted) {
				m_usrpTX->start();
				m_txStarted = true;
			}
			m_usrpTX->write(m_txBlock, m_txBlockFill, &underflow);
			blocks++;
			m_txBlockFill = 0;
			if(underflow)
				fprintf(stderr, "underflow TX\n");

			// calculate sample rate
			clock_gettime(CLOCK_MONOTONIC, &ts);
			now = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
			if(now - start >= 1000) {
				fprintf(stderr, "TX: Rate: %f samples/sec\n", (blocks * m_txBlockSize) / (2 * sizeof(short) * ((now - start) / 1000.0)));
				start = now;
				blocks = 0;
			}

		}
	}

}
