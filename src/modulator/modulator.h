#ifndef INCLUDE_MODULATOR_H
#define INCLUDE_MODULATOR_H

#include <stdint.h>
#include <usrp/usrp_standard.h>
#include "nco.h"
#include "rootraisedcosinefilter.h"

class Modulator  {
private:
	static const float m_constellations[8][2];
	static const uint8_t m_transitions[8][4];

	enum {
		BurstSymbols = 255,
	};

	usrp_standard_tx_sptr m_usrpTX;
	db_base_sptr m_txBoard;
	int m_txBlockSize;
	uint8_t* m_txBlock;
	int m_txBlockFill;
	bool m_txStarted;

	uint8_t m_burst[BurstSymbols];
	int m_burstIndex;

	RootRaisedCosineFilter m_rrcf;
	NCO m_carrier;

	int m_currentConstellation;
	int m_symbolSampleNum;
	int m_sampleNum;

	void getNextBurst();
	uint8_t getNextSymbol();
	Complex getNextConstellation();
	Complex getNextSample();

public:
	Modulator(float freq);

	void run();
};

#endif // INCLUDE_MODULATOR_H
