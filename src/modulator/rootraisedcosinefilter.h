#ifndef INCLUDE_ROOTRAISEDCOSINEFILTER_H
#define INCLUDE_ROOTRAISEDCOSINEFILTER_H

#include "complex.h"

class RootRaisedCosineFilter {
private:
	int m_len;
	float* m_h;
	float* m_filterI;
	float* m_filterQ;
	int m_start;

	void free();

public:
	RootRaisedCosineFilter();
	~RootRaisedCosineFilter();

	void init(float gain, float samplingRate, float symbolRate, float alpha, int numTaps);
	void dumpResponse();

	Complex filter(Complex sample);
};

#endif // INCLUDE_ROOTRAISEDCOSINEFILTER_H
