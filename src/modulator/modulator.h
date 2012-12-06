#ifndef INCLUDE_MODULATOR_H
#define INCLUDE_MODULATOR_H

#include <stdint.h>
#include <uhd/usrp/multi_usrp.hpp>
#include "nco.h"
#include "rootraisedcosinefilter.h"

class Modulator  {
private:
	static const float m_constellations[8][2];
	static const uint8_t m_transitions[8][4];

	enum {
		BurstSymbols = 255
	};

	uhd::usrp::multi_usrp::sptr m_usrpTX;
	uhd::tx_streamer::sptr m_tx_stream;

	uint8_t m_burst[BurstSymbols];
	int m_burstIndex;

	RootRaisedCosineFilter m_rrcf;
	NCO m_carrier;

	int m_currentConstellation;
	int m_symbolSampleNum;

	void getNextBurst();
	uint8_t getNextSymbol();
	Complex getNextConstellation();
	Complex getNextSample();

public:
	Modulator(float freq);

	void run();
};

#endif // INCLUDE_MODULATOR_H
