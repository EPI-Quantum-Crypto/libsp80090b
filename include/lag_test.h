/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef LAG_TEST_H
#define LAG_TEST_H

#include "utils.h"

#define D_LAG 128U
#define LAGMASK (D_LAG-1)

typedef struct lagBuf {
	uint8_t start;
	uint8_t end;
	long buf[D_LAG];
} lagBuf;

static inline double lag_test(uint8_t *S, long L, int k, const int verbose, const char *label) {
	long scoreboard[D_LAG] = {0};
	int winner = 0;
	long curRunOfCorrects = 0;
	long maxRunOfCorrects = 0;
	long correctCount = 0;
	lagBuf *ringBuffers;
	long highScore = 0;

	assert(S != NULL);
	assert(L > 2);
	assert(k >= 2);

	ringBuffers = (lagBuf*)calloc(k, sizeof(lagBuf));
	if (!ringBuffers) return -1.0;

	for (int j = 0; j < k; j++) {
		ringBuffers[j].start = 0;
		ringBuffers[j].end = 0;
	}

	ringBuffers[S[0]].buf[0] = 0;
	ringBuffers[S[0]].start = 0;
	ringBuffers[S[0]].end = 1;

	for (long i = 1; i < L; i++) {
		const uint8_t curSymbol = S[i];
		lagBuf *const curRingBuffer = &(ringBuffers[curSymbol]);

		if (curSymbol == S[i - winner - 1]) {
			correctCount++;
			curRunOfCorrects++;
			if (curRunOfCorrects > maxRunOfCorrects) {
				maxRunOfCorrects = curRunOfCorrects;
			}
		} else {
			curRunOfCorrects = 0;
		}

		if (curRingBuffer->start != curRingBuffer->end) {
			uint8_t counterIndex = curRingBuffer->end;
			const long cutoff = (i >= D_LAG) ? (i - D_LAG) : 0;

			do {
				counterIndex--;
				if (curRingBuffer->buf[counterIndex&LAGMASK] >= cutoff) {
					long curScore;
					long curOffset;
					curOffset = i - curRingBuffer->buf[counterIndex&LAGMASK] - 1;
					assert(curOffset < D_LAG);
					curScore = ++scoreboard[curOffset];

					if (curScore >= highScore) {
						winner = curOffset;
						highScore = curScore;
					}
				} else {
					curRingBuffer->start = (uint8_t)(counterIndex + 1U);
					break;
				}
			} while (counterIndex != curRingBuffer->start);
		}

		if ((uint8_t)(curRingBuffer->end - curRingBuffer->start) == (uint8_t)D_LAG) {
			curRingBuffer->start++;
		}
		curRingBuffer->buf[(curRingBuffer->end++)&LAGMASK] = i;
	}

	free(ringBuffers);

	return predictionEstimate(correctCount, L-1, maxRunOfCorrects, k, "Lag", verbose, label);
}

#endif // LAG_TEST_H
