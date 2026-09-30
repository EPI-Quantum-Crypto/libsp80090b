/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef MOST_COMMON_H
#define MOST_COMMON_H

#include "utils.h"
#include "test_case.h"

static inline double most_common_tc(uint8_t* data, const long len, const int alph_size, const int verbose, const char *label, test_case_t *tc){
	long i, mode;
	double pmax, ubound;
	double entEst;

	assert(len > 1);

	long *counts = (long*)calloc(alph_size, sizeof(long));
	if (!counts) return 0.0;

	for (i = 0; i < len; i++) counts[data[i]]++;

	mode = 0;
	for(i = 0; i < alph_size; i++){
		if(counts[i] > mode) mode = counts[i];
	}

	free(counts);

	pmax = mode / (double)len;

	ubound = MIN(1.0, pmax + ZALPHA * sqrt(pmax * (1.0 - pmax) / (len - 1.0)));
	entEst = -log2(ubound);
	if(verbose == 2) printf("%s MCV Estimate: mode = %ld, p-hat = %.17g, p_u = %.17g\n", label, mode, pmax, ubound);
	else if(verbose == 3) {
		printf("%s Most Common Value Estimate: Mode count = %ld\n", label, mode);
		printf("%s Most Common Value Estimate: p-hat = %.17g\n", label, pmax);
		printf("%s Most Common Value Estimate: p_u = %.17g\n", label, ubound);
		printf("%s Most Common Value Estimate: min entropy = %.17g\n", label, entEst);
	}

	if (tc) {
		tc->mcv_estimate_mode = mode;
		tc->mcv_estimate_p_hat = pmax;
		tc->mcv_estimate_p_u = ubound;
		tc->literal_mcv_estimate = (strcmp(label, "Literal") == 0);
	}

	return entEst;
}

static inline double most_common(uint8_t* data, const long len, const int alph_size, const int verbose, const char *label){
	return most_common_tc(data, len, alph_size, verbose, label, NULL);
}

#endif // MOST_COMMON_H
