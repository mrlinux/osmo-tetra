/*
 * Textbook implementation of a PI/4 DQPSK modulator for TETRA
 *
 * USRP is set to TXBW/4 below TX frequency and a prerotated signal is
 * transmitted. This moves the I/Q imbalance out of the main signal.
 * Please beware: I/Q imbalance is still there and can contain
 * considerable energy - most probably somewhere, where you do not want
 * to get caught transmitting.
 *
 * USRP is running at minimum TX rate and the modulator produces 14 samples
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
#include <fstream>
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
#if 1
	// read data for next burst
	int res = read(STDIN_FILENO, m_burst, BurstSymbols);
	if(res != BurstSymbols) {
		memset(m_burst, 0xff, BurstSymbols);
	}
#else // simulate bursts
	for (int i = 0; i < 255;)
	{
		int j = 0;
		for (; j <= 3; j++ )
			m_burst[i + j] = rand() % 3 + 1;
		i += j;
	}
#endif
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

	// frequency shift
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
	m_symbolSampleNum(0)
{
	srand ( time(NULL) );

	// RRC as pulse shaper
	m_rrcf.init(9.0f, 14.0f, 1.0f, 0.35f, 11 * 14 + 1);

	// create USRP object
	uhd::device_addr_t addr("");
	m_usrpTX = uhd::usrp::multi_usrp::make(addr);
	if(m_usrpTX == NULL) {
		fprintf(stderr, "USRP error\n");
		return;
	}

	// select lowest possible rate
	uhd::meta_range_t tx_rates = m_usrpTX->get_tx_rates();
	double lowest_possible_rate = tx_rates.start();
	m_usrpTX->set_tx_rate(lowest_possible_rate);
	double tx_rate = m_usrpTX->get_tx_rate();
	std::cout << "Selected TX rate: " << tx_rate << std::endl;

	// USRP gets set by TXBW/4 below the requested frequency
	const float carrier_offset = tx_rate / 4.0;

	m_carrier.setFreq(carrier_offset, tx_rate);

	uhd::usrp::subdev_spec_t subdev_spec("A:0");
	m_usrpTX->set_tx_subdev_spec(subdev_spec);

	// tune the monster
	uhd::tune_request_t tune_req(freq - carrier_offset, carrier_offset);
	uhd::tune_result_t tune_res = m_usrpTX->set_tx_freq(tune_req);
	fprintf(stderr, "%s\n", tune_res.to_pp_string().c_str());

	// set highest possible gain value
	uhd::meta_range_t tx_gains = m_usrpTX->get_tx_gain_range();
	double highest_possible_gain = tx_gains.stop();
	m_usrpTX->set_tx_gain(highest_possible_gain);
	double tx_gain = m_usrpTX->get_tx_gain();
	std::cout << "Selected TX gain: " << tx_gain << std::endl;

	//create a transmit streamer
	uhd::stream_args_t stream_args("fc32"); // complex floats
	m_tx_stream = m_usrpTX->get_tx_stream(stream_args);

	fprintf(stderr, "USRP TX block size: %d Bytes, Fs = %f\n",
		int(m_tx_stream->get_max_num_samps()),
		m_usrpTX->get_tx_rate());
}

void Modulator::run()
{
	uint64_t start;
	uint64_t now;
	timespec ts;
	int blocks;

	if(m_usrpTX == NULL)
		return;

	int txBlockFill = 0;
	int txBlockSize = m_tx_stream->get_max_num_samps() / (2 * sizeof(float));
	std::vector< Complex > buff(txBlockSize, Complex(0, 0));

	uhd::tx_metadata_t md;
	md.start_of_burst = false;
	md.end_of_burst = false;

	// get the time for sample rate measurement
	clock_gettime(CLOCK_MONOTONIC, &ts);
	start = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	blocks = 0;

	while(true) {
		buff[txBlockFill++] = getNextSample();

		if(txBlockFill >= txBlockSize) {
#if 0
			std::ofstream stream;
			stream.open("/tmp/tetramod.cfile", std::ios::out | std::ios::app);
			stream.write((const char *)buff.data(), txBlockFill * 2 * sizeof(float));
			stream.close();
#endif
			// a complete block is full -> send it to USRP
			m_tx_stream->send(&buff.front(), txBlockFill, md);
			blocks++;
			txBlockFill = 0;

			// calculate sample rate
			clock_gettime(CLOCK_MONOTONIC, &ts);
			now = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
			if(now - start >= 1000) {
				fprintf(stderr, "\rTX Rate: %f samples/sec ",
					(blocks * txBlockSize) /
					((now - start) / 1000.0));
				fflush(stderr);
				start = now;
				blocks = 0;
			}
		}
	}
}
