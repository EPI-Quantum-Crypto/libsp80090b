/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef MULTI_MMC_TEST_H
#define MULTI_MMC_TEST_H

#include "utils.h"

#define D_MMC 16
#define MAX_ENTRIES 100000

static inline double binaryMultiMMCPredictionEstimate(const uint8_t *S, long L, const int verbose, const char *label)
{
   long scoreboard[D_MMC] = {0};
   long *binaryDict[D_MMC];
   long winner = 0;
   long curWinner;
   long curRunOfCorrects = 0;
   long maxRunOfCorrects = 0;
   long correctCount = 0;
   long j, d, i;
   uint32_t curPattern=0;
   long dictElems[D_MMC] = {0};

   assert(L>3);
   assert(D_MMC < 31);

   for(j=0; j< D_MMC; j++) {
      binaryDict[j] = (long*)calloc(1U<<(j+2), sizeof(long));
   }

   for(d=0; d<D_MMC; d++) {
      curPattern = ((curPattern << 1) | (S[d]&1));
      (BINARYDICTLOC(d+1, curPattern))[S[d+1]&1] = 1;
      dictElems[d] = 1;
   }

   for(i=2; i<L; i++) {
      bool found_x = false;
      curWinner = winner;
      curPattern = 0;

      for(d=0; (d<D_MMC) && (d<=i-2); d++) {
         uint8_t curPrediction = 2;
         long curCount;
         long *binaryDictEntry;

         curPattern = (curPattern | (((uint32_t)(S[i-d-1]&1))<<d));
         binaryDictEntry = BINARYDICTLOC(d+1, curPattern);

         if((d == 0) || found_x) {
            if((binaryDictEntry[0] > binaryDictEntry[1])) {
               curPrediction = 0;
               curCount = binaryDictEntry[0];
            } else {
               curPrediction = 1;
               curCount = binaryDictEntry[1];
            }

            if(curCount == 0) found_x = false;
            else found_x = true;
         }

         if(found_x) {
            if(curPrediction == S[i]) {
               scoreboard[d]++;
               if(scoreboard[d] >= scoreboard[winner]) winner = d;

               if(d == curWinner){
                  correctCount++;
                  curRunOfCorrects++;
                  if(curRunOfCorrects > maxRunOfCorrects) maxRunOfCorrects = curRunOfCorrects;
               }
            } else if(d == curWinner) {
               curRunOfCorrects = 0;
            }

            if(binaryDictEntry[S[i]&1] != 0) {
               binaryDictEntry[S[i]&1]++;
            } else if(dictElems[d] < MAX_ENTRIES) {
               binaryDictEntry[S[i]&1]=1;
               dictElems[d]++;
            }
         } else if(dictElems[d] < MAX_ENTRIES) {
            binaryDictEntry[S[i]&1]=1;
            dictElems[d]++;
         }
      }
   }

   for(j=0; j< D_MMC; j++) {
      free(binaryDict[j]);
   }

   return(predictionEstimate(correctCount, L-2, maxRunOfCorrects, 2, "MultiMMC", verbose, label));
}

static inline double multi_mmc_test(uint8_t *data, long len, int alph_size, const int verbose, const char *label){
	if(alph_size == 2) {
		return binaryMultiMMCPredictionEstimate(data, len, verbose, label);
	}

	long winner, cur_winner, i, d, N, C, run_len, max_run_len;
	long scoreboard[D_MMC] = {0};
	long entries[D_MMC] = {0};
	uint8_t x[PFDICT_KEYLEN];
	pfmap_t *M[D_MMC];

	if(len < 3){
		if(verbose > 0) printf("\t*** Warning: not enough samples to run multiMMC test (need more than %d) ***\n", 3);
		return -1.0;
	}

	for(d = 0; d < D_MMC; d++) M[d] = pfmap_create(PFMAP_BUCKETS);

	// Step 1
	N = len-2;

	// Step 3
	winner = 0;
	C = 0;
	run_len = 0;
	max_run_len = 0;

	// initialize MMC counts (step 4.a and 4.b for the () case)
	for(d = 0; d < D_MMC; d++){
		if(d < N){
			memset(x, 0, PFDICT_KEYLEN);
			memcpy(x, data, d+1);
			pfmap_node_t *node = pfmap_insert(M[d], x);
			pfd_increment(&node->pd, data[d+1], true);
			entries[d] = 1;
		}
	}

	// perform predictions
	// i is the index of the new symbol to be predicted
	for(i = 2; i < len; i++){
		bool found_x = false;
		cur_winner = winner;

		for(d = 0; (d < D_MMC) && (i-2 >= d); d++) {
			pfmap_node_t *curp = NULL;

			// check if x has been previously seen as a prefix. If the prefix x has not occurred,
			// then do not make a prediction for current d and larger d's, since it will not
			// occur for them either. Note that found_x is meaningless on the first round, but
			// for that round d==0.
			if((d == 0) || found_x) {
				memset(x, 0, PFDICT_KEYLEN);
				memcpy(x, data+i-d-1, d+1);
				curp = pfmap_find(M[d], x);
				found_x = (curp != NULL);
			}

			if(found_x){
				long predictCount;
				uint8_t prediction = pfd_predict(&curp->pd, &predictCount);

				if(prediction == data[i]){
					if(++scoreboard[d] >= scoreboard[winner]) winner = d;
					if(d == cur_winner){
						C++;
						if(++run_len > max_run_len) max_run_len = run_len;
					}
				} else if(d == cur_winner) {
					run_len = 0;
				}

				if(pfd_increment(&curp->pd, data[i], entries[d] < MAX_ENTRIES)) {
					entries[d]++;
				}
			} else if(entries[d] < MAX_ENTRIES) {
				memset(x, 0, PFDICT_KEYLEN);
				memcpy(x, data+i-d-1, d+1);
				pfmap_node_t *node = pfmap_insert(M[d], x);
				pfd_increment(&node->pd, data[i], true);
				entries[d]++;
			}
		}
	}

	for(d = 0; d < D_MMC; d++) pfmap_destroy(M[d]);

	return(predictionEstimate(C, N, max_run_len, alph_size, "MultiMMC", verbose, label));
}

#endif // MULTI_MMC_TEST_H
