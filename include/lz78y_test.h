/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef LZ78Y_TEST_H
#define LZ78Y_TEST_H

#include "utils.h"

#define B_len 16
#define MAX_DICTIONARY_SIZE 65536

static inline double binaryLZ78YPredictionEstimate(const uint8_t *S, long L, const int verbose, const char *label)
{
   long *binaryDict[B_len];
   long curRunOfCorrects=0;
   long maxRunOfCorrects=0;
   long correctCount=0;
   long i, j;
   uint32_t curPattern=0;
   long dictElems=0;

   assert(L>B_len);
   assert(L-B_len > 2);
   assert(B_len < 32);

   for(j=0; j< B_len; j++) {
      binaryDict[j] = (long*)calloc(1U<<(j+2), sizeof(long));
   }

   for(j=0; j<B_len; j++) {
      curPattern = curPattern | (((uint32_t)(S[B_len - j - 1]&1)) << j);
      (BINARYDICTLOC(j+1, curPattern))[S[B_len]&0x1] = 1;
      dictElems++;
   }

   for(i=B_len+1; i<L; i++) {
      bool found_x;
      bool havePrediction = false;
      uint8_t roundPrediction=2;
      uint8_t curPrediction=2;
      long maxCount = 0;

      uint32_t retPattern = 0;
      for(j=0; j<B_len; j++) retPattern = (retPattern << 1) | S[i-B_len+j];
      curPattern = retPattern;

      for(j=B_len; j>0; j--) {
         long curCount;
         long *binaryDictEntry;

         curPattern = curPattern & ((1U<<j)-1);
         binaryDictEntry = BINARYDICTLOC(j, curPattern);

         if((binaryDictEntry[0] > binaryDictEntry[1])) {
            roundPrediction = 0;
            curCount = binaryDictEntry[0];
         } else {
            roundPrediction = 1;
            curCount = binaryDictEntry[1];
         }

         if(curCount == 0) found_x = false;
         else found_x = true;

         if(found_x) {
            if(curCount > maxCount) {
               maxCount = curCount;
               havePrediction = true;
               curPrediction = roundPrediction;
            }

            binaryDictEntry[S[i]&1]++;
         } else if(dictElems < MAX_DICTIONARY_SIZE) {
            binaryDictEntry[S[i]&1]=1;
            dictElems++;
         }
      }

      if(havePrediction && (curPrediction == S[i])) {
            correctCount++;
            curRunOfCorrects++;
            if(curRunOfCorrects > maxRunOfCorrects) maxRunOfCorrects = curRunOfCorrects;
      } else {
         curRunOfCorrects = 0;
      }
   }

   for(j=0; j< B_len; j++) free(binaryDict[j]);

   return(predictionEstimate(correctCount, L-B_len-1, maxRunOfCorrects, 2, "LZ78Y", verbose, label));
}

static inline double LZ78Y_test(uint8_t* data, long len, int alph_size, const int verbose, const char *label){
	if(alph_size == 2) {
		return binaryLZ78YPredictionEstimate(data, len, verbose, label);
	}

	long i, j, N, C, run_len, max_run_len;
	long dict_size;
	uint8_t x[PFDICT_KEYLEN];
	pfmap_t *D[B_len];

	if(len < B_len+2){
		if(verbose > 0) printf("\t*** Warning: not enough samples to run LZ78Y test (need more than %d) ***\n", B_len+2);
		return -1.0;
	}

	for(j = 0; j < B_len; j++) D[j] = pfmap_create(PFMAP_BUCKETS);

	N = len-B_len-1;
	C = 0;
	run_len = 0;
	max_run_len = 0;

	// initialize dictionary counts with {(S[15]), S[16]}, {(S[14], S[15]), S[16]}, ..., {(S[0]), S[1], ..., S[15]), S[16]}
	dict_size = 0;
	for(j = 1; j <= B_len; j++){
		memset(x, 0, PFDICT_KEYLEN);
		memcpy(x, data+B_len-j, j);
		pfmap_node_t *node = pfmap_insert(D[j-1], x);
		pfd_increment(&node->pd, data[B_len], true);
		dict_size++;
	}

	// perform predictions
	for(i = B_len+1; i < len; i++) {
		bool have_prediction = false;
		uint8_t prediction = 0;
		long max_count = 0;

		for(j = B_len; j > 0; j--) {
			// check if x has been previously seen.
			// The prefix string should contain the j-tuple (S[i-j] ... S[i-1])
			memset(x, 0, PFDICT_KEYLEN);
			memcpy(x, data+i-j, j);
			pfmap_node_t *curp = pfmap_find(D[j-1], x);

			if(curp) {
				long count;
				// x has occurred, find max (x,y) pair across all y's
				uint8_t y = pfd_predict(&curp->pd, &count);

				if(count > max_count){
					max_count = count;
					prediction = y;
					have_prediction = true;
				}
				// x exists as a prefix, so we always increment (and perhaps add a new postfix)
				pfd_increment(&curp->pd, data[i], true);
			} else if(dict_size < MAX_DICTIONARY_SIZE) {
				// We didn't find the x prefix, so (x,y) surely can't have occurred.
				// We're allowed to make a new entry. Do so.
				pfmap_node_t *node = pfmap_insert(D[j-1], x);
				pfd_increment(&node->pd, data[i], true);
				dict_size++;
			}
		}

		// test prediction of maximum (x,y) pair
		if(have_prediction && (prediction == data[i])){
			C++;
			if(++run_len > max_run_len) max_run_len = run_len;
		}
		else run_len = 0;
	}

	for(j = 0; j < B_len; j++) pfmap_destroy(D[j]);

	return(predictionEstimate(C, N, max_run_len, alph_size, "LZ78Y", verbose, label));
}

#endif // LZ78Y_TEST_H
