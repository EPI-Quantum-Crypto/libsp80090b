/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef COMPRESSION_TEST_H
#define COMPRESSION_TEST_H

#include "utils.h"

static inline void kahan_add(double *sum, double *comp, double in){
	double y, t; 
	y = in - *comp;
	t = *sum + y;
	*comp = (t - *sum) - y;
	*sum = t;
}

static inline double G_comp(double z, int d, long num_blocks){
	double Ai=0.0, Ai_comp=0.0;
	double firstSum=0.0, firstSum_comp=0.0;
	long v = num_blocks - d;
	double Ad1;

	long double Bi;
	long double Bterm;
	long double ai;
	long double aiScaled;
	bool underflowTruncate;

	assert(d>0);
	assert(num_blocks>d);

	Bterm = (1.0L-(long double)z);
	Bi = Bterm;

	for(int i=2; i<=d; i++) {
		kahan_add(&Ai, &Ai_comp, log2l((long double)i)*Bi);
		Bi *= Bterm;
	}

	Ad1 = Ai;
	underflowTruncate = false;

	for(long i=d+1; i<=num_blocks-1; i++) {
		ai = log2l((long double)i)*Bi;
		kahan_add(&Ai, &Ai_comp, (double)ai);

		aiScaled = (long double)(num_blocks-i) * ai;
		if((double)aiScaled > 0.0) {
			kahan_add(&firstSum, &firstSum_comp, (double)aiScaled);
		} else {
			underflowTruncate = true;
			break;
		}
		Bi *= Bterm;
	}

	kahan_add(&firstSum, &firstSum_comp, ((double)(num_blocks-d))*Ad1);

	if(!underflowTruncate) {
		ai = log2l((long double)num_blocks)*Bi;
		kahan_add(&Ai, &Ai_comp, (double)ai);
	}

	return 1/(double)v * z*(z*firstSum + (Ai - Ad1));
}

static inline double com_exp(double p, unsigned int alph_size, int d, long num_blocks){
	double q = (1.0-p)/((double)alph_size-1.0);
	return G_comp(p, d, num_blocks) + ((double)alph_size-1.0) * G_comp(q, d, num_blocks);
}

static inline double compression_test(uint8_t* data, long len, const int verbose, const char *label){
	int j, d, b = 6;
	long i, num_blocks, v;
	unsigned int block, alph_size = 1 << b;
	unsigned int dict[64]; // alph_size = 64 for b = 6
	double X=0.0, X_comp=0.0;
	double sigma=0.0, sigma_comp=0.0;
	double p, entEst;
	double ldomain, hdomain, lbound, hbound, lvalue, hvalue, pVal, lastP;

	assert(b == 6);
	d = 1000;
	num_blocks = len/b;

	if(num_blocks <= d){
		if(verbose > 0) printf("%s Compression Estimate: Dataset was not long enough to run compression test (%ld <= %d).\n", label, num_blocks, d);
		return -1.0;
	}

	// create dictionary
	for(i = 0; i < alph_size; i++) dict[i] = 0;
	for(i = 0; i < d; i++){
		block = 0;
		for(j = 0; j < b; j++) block |= (data[i*b + j] & 0x1) << (b-j-1);
		dict[block] = i+1;
	}

	// test data against dictionary
	v = num_blocks - d;
	for(i = d; i < num_blocks; i++){
		block = 0;
		for(j = 0; j < b; j++) block |= (data[i*b + j] & 0x1) << (b-j-1);
		kahan_add(&X, &X_comp, log2(i+1-dict[block]));
		kahan_add(&sigma, &sigma_comp, log2(i+1-dict[block])*log2(i+1-dict[block]));
		dict[block] = i+1;
	}

	// compute mean and stdev
	X /= v;
	sigma = 0.5907 * sqrt(sigma/(v-1.0) - X*X);

	if(verbose == 2) {
		printf("%s Compression Estimate: X-bar = %.17g, ", label, X);
		printf("sigma-hat = %.17g, ", sigma);
	} else if(verbose == 3) {
		printf("%s Compression Estimate: X-bar = %.17g\n", label, X);
		printf("%s Compression Estimate: sigma-hat = %.17g\n", label, sigma);
	}

	// binary search for p
	X -= ZALPHA * sigma/sqrt(v);

	if(verbose == 3) printf("%s Compression Estimate: X-bar' = %.17g\n", label, X);

	if(com_exp(1.0/(double)alph_size, alph_size, d, num_blocks) > X) {
		ldomain = 1.0 / (double)alph_size;
		hdomain = 1.0;

		lbound = ldomain;
		hbound = hdomain;

		lvalue = DBL_INFINITY;
		hvalue = -DBL_INFINITY;

		//Note that the bounds are in [0,1], so overflows aren't an issue
		//But underflows are.
		p = (lbound + hbound) / 2.0;
		pVal = com_exp(p, alph_size, d, num_blocks);

		//We don't need the initial pVal invariant, as our initial bounds are infinite.
		//We don't need the initial bounds, as they are set to the domain bounds
		for(j=0; j<ITERMAX; j++) {
			//Have we reached "equality"?
			if(relEpsilonEqual(pVal, X, ABSEPSILON, RELEPSILON, 4)) break;

			//Now update based on the found pVal
			if(X < pVal) {
				lbound = p;
				lvalue = pVal;
			} else {
				hbound = p;
				hvalue = pVal;
			}

			//We now verify that ldomain <= lbound < p < hbound <= hdomain
			//and that target in [ lvalue, hvalue ]
			if(lbound >= hbound) {
				p = fmin(fmax(lbound, hbound),hdomain);
				break;
			}

			//invariant. If this isn't true, then we can't evaluate here.
			if(!(INCLOSEDINTERVAL(lbound, ldomain, hdomain) && INCLOSEDINTERVAL(hbound,  ldomain, hdomain))) {
				//This is a search failure. We need to return "full entropy"  (as directed in step #8).
				p = ldomain;
				break;
			}

			//invariant. If this isn't true, then seeking the value within this interval doesn't make sense.
			if(!INCLOSEDINTERVAL(X, lvalue, hvalue)) {
				//This is a search failure. We need to return "full entropy"  (as directed in step #8).
				p = ldomain;
				break;
			}

			//Update p
			lastP = p;
			p = (lbound + hbound) / 2.0;

			//invariant. If this isn't true, then further calculation isn't really meaningful.
			if(!INOPENINTERVAL(p,  lbound, hbound)) {
				p = hbound;
				break;
			}

			//Look for a cycle
			if(lastP == p) {
				p = hbound;
				break;
			}

			pVal = com_exp(p, alph_size, d, num_blocks);

			//invariant: If this isn't true, then this isn't loosely monotonic
			if(!INCLOSEDINTERVAL(pVal, lvalue, hvalue)) {
				p = hbound;
				break;
			}
		}//for loop
	} else {
		p = -1.0;
	}

	if(p > 1.0 / (double)alph_size) {
		entEst = -log2(p)/b;

		if(verbose == 3) printf("%s Compression Estimate: Found p.\n", label);
	} else {
		p = 1.0 / (double)alph_size;
		entEst = 1.0;
		if(verbose == 3) printf("%s Compression Estimate: Could Not Find p. Proceeding with the lower bound for p.\n", label);
	}

	if(verbose == 2) printf("p = %.17g\n", p);
	else if(verbose == 3) {
		printf("%s Compression Estimate: p = %.17g\n", label, p);
		printf("%s Compression Estimate: min entropy = %.17g\n", label, entEst);
	}

	return entEst;
}

#endif // COMPRESSION_TEST_H
