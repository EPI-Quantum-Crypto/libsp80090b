/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef COLLISION_TEST_H
#define COLLISION_TEST_H

#include "utils.h"

static inline double F_col(double q){
   return q*(2.0*q*q+2.0*q+1.0);
}

static inline double col_exp(double p){
	double q = 1.0 - p;
	return (p/(q*q))*(1.0 + 0.5*(1.0/p - 1.0/q))*F_col(q) - (p/q)*0.5*(1.0/p - 1.0/q);
}

static inline double collision_test(uint8_t* data, long len, const int verbose, const char *label){
	long v, i;
	int t_v;
	double X, s, p;
	double entEst;

	i = 0;
	v = 0;
	s = 0.0;

	while(i < len-1){
		if(data[i] == data[i+1]) t_v = 2;
		else if(i < len-2) t_v = 3;
		else break;
		
		v++;
		s += t_v*t_v;
		i += t_v;
	}

	X = i / (double)v;
	if(verbose == 2) printf("%s Collision Estimate: X-bar = %.17g, ", label, X);
	s = sqrt((s - (i*X)) / (v-1));
	if(verbose == 2) printf("sigma-hat = %.17g, ", s);

	if(verbose == 3) {
		printf("%s Collision Estimate: v = %ld\n", label, v);
		printf("%s Collision Estimate: Sum t_i = %ld\n", label, i);
		printf("%s Collision Estimate: X-bar = %.17g\n", label, X);
		printf("%s Collision Estimate: sigma-hat = %.17g\n", label, s);
	}

	X -= ZALPHA * s/sqrt(v);
	if(X < 2.0) X = 2.0;

	if(verbose == 3)
		printf("%s Collision Estimate: X-bar' = %.17g\n", label, X);

	if(X < 2.5) {
		p = 0.5 + sqrt(1.25 - 0.5 * X);
		entEst = -log2(p);
		if(verbose == 3) printf("%s Collision Estimate: Found p.\n", label);
	} else {
		if(verbose == 3) printf("%s Collision Estimate: Could Not Find p. Proceeding with the lower bound for p.\n", label);
		p = 0.5;
		entEst = 1.0;
	}

	if(verbose == 2) printf("p = %.17g\n", p);
	else if(verbose == 3) {
		printf("%s Collision Estimate: p = %.17g\n", label, p);
		printf("%s Collision Estimate: min entropy = %.17g\n", label, entEst);
	}

	return entEst;
}

#endif // COLLISION_TEST_H
