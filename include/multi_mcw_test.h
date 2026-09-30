/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef MULTI_MCW_TEST_H
#define MULTI_MCW_TEST_H

#include "utils.h"

#define NUM_WINS 4

static inline double multi_mcw_test(uint8_t *data, long len, int alph_size, const int verbose, const char *label){
	int winner;
	int W[NUM_WINS] = {63, 255, 1023, 4095};
	long i, j, k, N, C, run_len, max_run_len, max_pos; 
	long scoreboard[NUM_WINS] = {0};
	long max_cnts[NUM_WINS] = {0};
	
	long **win_cnts = (long**)malloc(NUM_WINS * sizeof(long*));
	long **win_poses = (long**)malloc(NUM_WINS * sizeof(long*));
	for(i = 0; i < NUM_WINS; i++) {
		win_cnts[i] = (long*)calloc(alph_size, sizeof(long));
		win_poses[i] = (long*)calloc(alph_size, sizeof(long));
	}

	uint8_t frequent[NUM_WINS] = {0};

	if(len < W[NUM_WINS-1]+1){	
		if (verbose > 0) printf("\t*** Warning: not enough samples to run multiMCW test (need more than %d) ***\n", W[NUM_WINS-1]+1);
		for(i = 0; i < NUM_WINS; i++) { free(win_cnts[i]); free(win_poses[i]); }
		free(win_cnts); free(win_poses);
		return -1.0;
	}

	N = len-W[0];
	winner = 0;
	C = 0;
	run_len = 0;
	max_run_len = 0;

	for(i = 0; i < W[NUM_WINS-1]; i++){
		for(j = 0; j < NUM_WINS; j++){
			if(i < W[j]){
				if(max_cnts[j] <= ++win_cnts[j][data[i]]){
					max_cnts[j] = win_cnts[j][data[i]];
					frequent[j] = data[i];
				}
				win_poses[j][data[i]] = i;
			}
		}
	}

	for (i = W[0]; i < len; i++){
		if(frequent[winner] == data[i]){
			C++;
			if(++run_len > max_run_len) max_run_len = run_len;
		}
		else run_len = 0;

		for(j = 0; j < NUM_WINS; j++){
			if((i >= W[j]) && (frequent[j] == data[i])){
				if(++scoreboard[j] >= scoreboard[winner]) winner = j;
			}
		}

		for(j = 0; j < NUM_WINS; j++){
			if(i >= W[j]){
				win_cnts[j][data[i-W[j]]]--;
				win_cnts[j][data[i]]++;
				win_poses[j][data[i]] = i;
				if((data[i-W[j]] != frequent[j]) && (max_cnts[j] <= win_cnts[j][data[i]])){
					max_cnts[j] = win_cnts[j][data[i]];
					frequent[j] = data[i];
				}
				else if(data[i-W[j]] == frequent[j]){
					max_cnts[j]--;
					max_pos = i-W[j];
					for(k = 0; k < alph_size; k++){
						if((max_cnts[j] < win_cnts[j][k]) || ((max_cnts[j] == win_cnts[j][k]) && (max_pos <= win_poses[j][k]))){
							max_cnts[j] = win_cnts[j][k];
							frequent[j] = k;
							max_pos = win_poses[j][k];
						}
					}
				}
			}
		}
	}

	double result = predictionEstimate(C, N, max_run_len, alph_size, "MultiMCW", verbose, label);

	for(i = 0; i < NUM_WINS; i++) { free(win_cnts[i]); free(win_poses[i]); }
	free(win_cnts); free(win_poses);

	return result;
}

#endif // MULTI_MCW_TEST_H
