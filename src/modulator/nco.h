#ifndef INCLUDE_NCO_H
#define INCLUDE_NCO_H

#include "complex.h"

class NCO {
private:
	float m_phInc;
	float m_ph;

public:
	NCO();

	void setFreq(float freq, float sampleRate);
	Complex nextIQ();
};

#endif // INCLUDE_NCO_H
