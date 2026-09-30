/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef MARKOV_TEST_H
#define MARKOV_TEST_H

#include "utils.h"

static inline double markov_test(uint8_t* data, long len, const int verbose, const char *label){
	long i, C_0, C_1, C_00, C_10;
	double H_min, tmp_min_entropy, P_0, P_1, P_00, P_01, P_10, P_11, entEst;

	C_0 = 0;
	C_00 = 0;
	C_10 = 0;

	assert(len > 1);

	for(i = 0; i < len-1; i++){
		if(data[i] == 0){
			C_0++;
			if(data[i+1] == 0) C_00++;
		}
		else if(data[i+1] == 0) C_10++;
	}

	C_1 = len - 1 - C_0;

	if(C_0 > 0) {
		P_00 = ((double)C_00) / ((double)C_0);
		P_01 = 1.0 - P_00;
	} else {
		P_00 = 0.0;
		P_01 = 0.0;
	}

	if(C_1 > 0) {
		P_10 = ((double)C_10) / ((double)C_1);
		P_11 = 1.0 - P_10;
	} else {
		P_10 = 0.0;
		P_11 = 0.0;
	}

	if(data[len-1] == 0) C_0++;

	P_0 = C_0 / (double)len;
	P_1 = 1.0 - P_0;

	if(verbose == 2) printf("%s Markov Estimate: P_0 = %.17g, P_1 = %.17g, P_0,0 = %.17g, P_0,1 = %.17g, P_1,0 = %.17g, P_1,1 = %.17g, ", label, P_0, P_1, P_00, P_01, P_10, P_11);
	else if(verbose == 3) {
		printf("%s Markov Estimate: P_0 = %.17g\n", label, P_0);
		printf("%s Markov Estimate: P_1 = %.17g\n", label, P_1);
		printf("%s Markov Estimate: P_{0,0} = %.17g\n", label, P_00);
		printf("%s Markov Estimate: P_{0,1} = %.17g\n", label, P_01);
		printf("%s Markov Estimate: P_{1,0} = %.17g\n", label, P_10);
		printf("%s Markov Estimate: P_{1,1} = %.17g\n", label, P_11);
	}

	H_min = 128.0;

	if(P_00 > 0.0){
		tmp_min_entropy = -log2(P_0) - 127.0*log2(P_00); 
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;
	}

	if((P_01 > 0.0) && (P_10 > 0.0)){
		tmp_min_entropy = -log2(P_0) - 64.0*log2(P_01) - 63.0*log2(P_10);
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;
	}

	if((P_01 > 0.0) && (P_11 > 0.0)){
		tmp_min_entropy = -log2(P_0) - log2(P_01) - 126.0*log2(P_11);
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;	
	}

	if((P_10 > 0.0) && (P_00 > 0.0)){
		tmp_min_entropy = -log2(P_1) - log2(P_10) - 126.0*log2(P_00);
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;
	}

	if((P_10 > 0.0) && (P_01 > 0.0)){
		tmp_min_entropy = -log2(P_1) - 64.0*log2(P_10) - 63.0*log2(P_01);
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;
	}

	if(P_11 > 0.0){
		tmp_min_entropy = -log2(P_1) - 127.0*log2(P_11);
		if(tmp_min_entropy < H_min) H_min = tmp_min_entropy;
	}

	entEst = fmin(H_min / 128.0, 1.0);

	if(verbose == 2) printf("p_max = %.17g\n", pow(2.0, -H_min));
	else if(verbose == 3) {
		printf("%s Markov Estimate: p-hat_max = %.17g\n", label, pow(2.0, -H_min));
		printf("%s Markov Estimate: min entropy = %.17g\n", label, entEst);
	}

	return entEst;
}

#endif // MARKOV_TEST_H
