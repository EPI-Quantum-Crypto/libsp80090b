/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef LRS_TEST_H
#define LRS_TEST_H

#include "utils.h"
#include <limits.h>
#include <divsufsort.h>
#include <divsufsort64.h>

#define SAINDEX_MAX INT32_MAX
#define SAINDEX64_MAX INT64_MAX

static inline void sa2lcp32(const uint8_t text[], long int n, const saidx_t *sa, saidx_t *lcp) {
	saidx_t h;
	saidx_t *rank = (saidx_t*)malloc((n+1) * sizeof(saidx_t));
	if (!rank) return;

	assert(n>1);

	lcp[0] = -1;
	lcp[1] = 0;

	for(saidx_t i=0; i<=(saidx_t)n; i++) {
		rank[sa[i]] = i;
	}

	h=0;

	for(saidx_t i=0; i<(saidx_t)n; i++) {
		saidx_t k = rank[i];
		if(k>1) {
			saidx_t j = sa[k-1];
			while((i+h<(saidx_t)n) && (j+h<(saidx_t)n) && (text[i+h]==text[j+h])) {
				h++;
			}
			lcp[k] = h;
		}
		if(h>0) {
			h--;
		}
	}
	free(rank);
}

static inline void sa2lcp64(const uint8_t text[], long int n, const saidx64_t *sa, saidx64_t *lcp) {
	saidx64_t h;
	saidx64_t *rank = (saidx64_t*)malloc((n+1) * sizeof(saidx64_t));
	if (!rank) return;

	assert(n>1);

	lcp[0] = -1;
	lcp[1] = 0;

	for(saidx64_t i=0; i<=(saidx64_t)n; i++) {
		rank[sa[i]] = i;
	}

	h=0;

	for(saidx64_t i=0; i<(saidx64_t)n; i++) {
		saidx64_t k = rank[i];
		if(k>1) {
			saidx64_t j = sa[k-1];
			while((i+h<(saidx64_t)n) && (j+h<(saidx64_t)n) && (text[i+h]==text[j+h])) {
				h++;
			}
			lcp[k] = h;
		}
		if(h>0) {
			h--;
		}
	}
	free(rank);
}

/* text/n/sa are as in the NIST reference; lcp must have room for n+1 entries
 * (indices 0..n), which the caller (SAalgs32/64) provides via an offset into
 * a larger buffer to reproduce the reference's "erase(begin()); L[n] = 0;"
 * re-indexing (see below). */
static inline void calcSALCP32(const uint8_t text[], long int n, saidx_t *sa, saidx_t *lcp) {
	int32_t res;

	assert(n < SAINDEX_MAX);
	assert(n > 0);

	sa[0] = (saidx_t)n;
	res = divsufsort((const sauchar_t *)text, (saidx_t *)(sa+1), (saidx_t)n);
	assert(res==0);
	sa2lcp32(text, n, sa, lcp);
}

static inline void calcSALCP64(const uint8_t text[], long int n, saidx64_t *sa, saidx64_t *lcp) {
	int32_t res;

	assert(n < SAINDEX64_MAX);
	assert(n > 0);

	sa[0] = (saidx64_t)n;
	res = divsufsort64((const sauchar_t *)text, (saidx64_t *)(sa+1), (saidx64_t)n);
	assert(res==0);
	sa2lcp64(text, n, sa, lcp);
}

/* Based on the algorithm outlined by Aaron Kaufer:
 * http://www.untruth.org/~josh/sp80090b/Kaufer%20Further%20Improvements%20for%20SP%20800-90B%20Tuple%20Counts.pdf
 * This is a direct C port of the NIST reference implementation (SP800-90B_EntropyAssessment/cpp/shared/lrs_test.h).
 */
static inline void SAalgs32(const uint8_t text[], long int n, int k, double *t_tuple_res, double *lrs_res, const int verbose, const char *label) {
	saidx_t *sa = (saidx_t*)malloc((n+1) * sizeof(saidx_t));
	/* Lraw holds indices 0..n+1; L = Lraw+1 reproduces the reference's
	 * "L.erase(L.begin()); L[n] = 0;" re-indexing below. */
	saidx_t *Lraw = (saidx_t*)malloc((n+2) * sizeof(saidx_t));
	saidx_t *L;

	long int u, v, c, j, i;
	saidx_t t;
	long double Pmax, pu;
	double t_tuple_entropy = -1.0, lrs_entropy = -1.0;

	if (t_tuple_res) *t_tuple_res = -1.0;
	if (lrs_res) *lrs_res = -1.0;

	if (!sa || !Lraw) { if (sa) free(sa); if (Lraw) free(Lraw); return; }

	assert(n>0);
	assert(k>0);
	assert(n < SAINDEX_MAX);

	calcSALCP32(text, n, sa, Lraw);

	L = Lraw + 1;
	L[n] = 0;
	assert(L[0] == 0);

	v = 0;
	for(i = 0; i < n; i++) {
		if(L[i] > v) v = L[i];
	}

	assert((v>0) && (v < n));

	saidx_t *Q = (saidx_t*)malloc((v+1) * sizeof(saidx_t));
	saidx_t *A = (saidx_t*)malloc((v+2) * sizeof(saidx_t));
	saidx_t *I = (saidx_t*)malloc((v+3) * sizeof(saidx_t));

	for(i=0; i<=v; i++) Q[i] = 1;
	for(i=0; i<=v+1; i++) A[i] = 0;
	for(i=0; i<=v+2; i++) I[i] = 0;

	j = 0;
	for(i = 1; i <= n; i++) {
		c = 0;
		assert(L[i] >= 0);

		if(L[i] < L[i-1]) {
			t = L[i-1];
			assert(j>0);
			j--;
			assert(j<=v);

			while(t > L[i]) {
				assert((t>0) && (t <= v));
				if((j > 0) && (I[j] == t)) {
					A[I[j]] += A[I[j+1]];
					A[I[j+1]] = 0;
					j--;
				}

				if(Q[t] >= A[I[j+1]]+1) {
					if(j > 0) {
						t = I[j];
					} else {
						t = L[i];
					}
				} else {
					Q[t--] = A[I[j+1]]+1;
				}
			}

			c = A[I[j+1]]; /* store carry over count */
			A[I[j+1]] = 0;
		}

		if(L[i] > 0) {
			if((j < 1) || (I[j] < L[i])) {
				assert(j<v);
				I[++j] = L[i];
			}
			A[I[j]] += c+1; /* update count for t = I[j] = L[i] */
		}
	}

	/* Calculate u */
	for(u=1; (u<=v) && (Q[u] >= 35); u++);

	assert(u > 0);
	assert(((u == v+1) || ((u <= v) && (Q[u] < 35))));
	assert(((u == 1) || (Q[u-1] >= 35)));

	/* Calculate the various Pmax[i] values; only the largest is retained */
	Pmax = -1.0L;
	for(i=1; i<u; i++) {
		long double curP = ((long double)(Q[i]))/((long double)(n-i+1));
		long double curPMax = powl(curP, 1.0L/(long double)i);

		if(curPMax > Pmax) {
			Pmax = curPMax;
		}
	}

	if(Pmax > 0.0L) {
		pu = Pmax + ZALPHA_L*sqrtl(Pmax*(1.0L - Pmax)/((long double)(n - 1)));
		if(pu > 1.0L) pu = 1.0L;

		t_tuple_entropy = (double)-log2l(pu);

		if(verbose == 2) printf("%s t-Tuple Estimate: t = %ld, p-hat_max = %.22Lg, p_u = %.22Lg\n", label, u-1, Pmax, pu);
		else if(verbose == 3) {
			printf("%s t-Tuple Estimate: t = %ld\n", label, u-1);
			printf("%s t-Tuple Estimate: p-hat_max = %.22Lg\n", label, Pmax);
			printf("%s t-Tuple Estimate: p_u = %.22Lg\n", label, pu);
			printf("%s t-Tuple Estimate: min entropy = %.17g\n", label, t_tuple_entropy);
		}

		if (t_tuple_res) *t_tuple_res = t_tuple_entropy;
	} else {
		if(verbose > 1) printf("t-Tuple Estimate: No strings are repeated 35 times. t-Tuple estimate failed.\n");
	}

	/* Calculate the LRS estimate */
	if(v>=u) {
		uint64_t *S = (uint64_t*)calloc((size_t)v+1, sizeof(uint64_t));
		memset(A, 0, sizeof(saidx_t)*((size_t)v+2));

		for(i = 1; i <= n; i++) {
			if((L[i-1] >= u) && (L[i] < L[i-1])) {
				saidx_t b = L[i];

				if(b < u) b = u-1;

				for(t = L[i-1]; t > b; t--) {
					uint64_t priorS;
					uint64_t choices;
					A[t] += A[t+1];
					A[t+1] = 0;

					assert(A[t] >= 0);

					priorS = S[t];
					choices = ((((uint64_t)(A[t]+1) * (uint64_t)(A[t]))))>>1;
					S[t] = priorS + choices;
					assert(S[t] >= priorS);
				}

				if(b >= u) A[b] += A[b+1];
				A[b+1] = 0;
			}

			if(L[i] >= u) A[L[i]]++;
		}

		Pmax = 0.0L;
		for(i=u; i<=v; i++) {
			uint64_t choices = (((uint64_t)n-(uint64_t)i)*((uint64_t)n-(uint64_t)i+1U))>>1;
			long double curP = ((long double)S[i]) / (long double)choices;
			long double curPMax = powl(curP, 1.0L/((long double)i));

			if(Pmax < curPMax) {
				Pmax = curPMax;
			}
		}

		pu = Pmax + ZALPHA_L*sqrtl(Pmax*(1.0L - Pmax)/((long double)(n - 1)));
		if(pu > 1.0L) pu = 1.0L;

		lrs_entropy = (double)-log2l(pu);

		if(verbose == 2) printf("%s LRS Estimate: u = %ld, v = %ld, p-hat = %.22Lg, p_u = %.22Lg\n", label, u, v, Pmax, pu);
		else if(verbose == 3) {
			printf("%s LRS Estimate: u = %ld\n", label, u);
			printf("%s LRS Estimate: v = %ld\n", label, v);
			printf("%s LRS Estimate: p-hat = %.22Lg\n", label, Pmax);
			printf("%s LRS Estimate: p_u = %.22Lg\n", label, pu);
			printf("%s LRS Estimate: min entropy = %.17g\n", label, lrs_entropy);
		}

		if (lrs_res) *lrs_res = lrs_entropy;

		free(S);
	} else {
		printf("LRS Estimate: v<u. Can't Run LRS Test.\n");
	}

	free(sa); free(Lraw); free(Q); free(A); free(I);
}

static inline void SAalgs(const uint8_t text[], long int n, int k, double *t_tuple_res, double *lrs_res, const int verbose, const char *label) {
	SAalgs32(text, n, k, t_tuple_res, lrs_res, verbose, label);
}

/* Length of the longest repeated substring: maximum of the LCP array. */
static inline long len_LRS32(const uint8_t text[], const long n) {
	saidx_t *sa = (saidx_t*)malloc((n + 1) * sizeof(saidx_t));
	saidx_t *lcp = (saidx_t*)malloc((n + 1) * sizeof(saidx_t));
	saidx_t lrs_len = -1;

	if (!sa || !lcp) {
		free(sa);
		free(lcp);
		return -1;
	}
	for (long j = 0; j <= n; j++) lcp[j] = -1;
	calcSALCP32(text, n, sa, lcp);
	for (long j = 0; j <= n; j++) if (lcp[j] > lrs_len) lrs_len = lcp[j];
	free(sa);
	free(lcp);
	return (long)lrs_len;
}

static inline long len_LRS64(const uint8_t text[], const long n) {
	saidx64_t *sa = (saidx64_t*)malloc((n + 1) * sizeof(saidx64_t));
	saidx64_t *lcp = (saidx64_t*)malloc((n + 1) * sizeof(saidx64_t));
	saidx64_t lrs_len = -1;

	if (!sa || !lcp) {
		free(sa);
		free(lcp);
		return -1;
	}
	for (long j = 0; j <= n; j++) lcp[j] = -1;
	calcSALCP64(text, n, sa, lcp);
	for (long j = 0; j <= n; j++) if (lcp[j] > lrs_len) lrs_len = lcp[j];
	free(sa);
	free(lcp);
	return (long)lrs_len;
}

/*
 * Length of the longest repeated substring test, SP 800-90B Section 5.2.5
 * (NIST len_LRS_test()). With p_col = sum p_i^2 and W the LRS length, the
 * number of overlapping pairs of W-substrings is N = C(L - W + 1, 2); the
 * test passes iff Pr(X >= 1) = 1 - (1 - p_col^W)^N >= 1/1000.
 * Returns 1 (pass), 0 (fail) or -1 (error).
 */
static inline int len_LRS_test(const uint8_t *data, long len, const int alph_size, const int verbose, const char *label) {
	double p[256];
	long double p_col = 0.0L, p_colPower, logProbNoColsPerPair;
	long W, N;

	if (len < 2 || alph_size < 1 || alph_size > 256) return -1;

	for (int i = 0; i < alph_size; i++) p[i] = 0.0;
	for (long i = 0; i < len; i++) p[data[i]]++;
	for (int i = 0; i < alph_size; i++) {
		p[i] /= (double)len;
		p_col += powl((long double)p[i], 2.0L);
	}

	/* p_col = 1: all symbols identical, a collision of any length is certain */
	if (p_col > 1.0L - LDBL_EPSILON) {
		if (verbose == 2) {
			printf("\tPr(X >= 1) = 1.0\n");
		} else if (verbose > 2) {
			printf("%s Longest Repeated Substring: P_col = 1.0\n", label);
			printf("%s Longest Repeated Substring: Pr(X >= 1) = 1.0\n", label);
		}
		return 1;
	}

	W = (len < SAINDEX_MAX) ? len_LRS32(data, len) : len_LRS64(data, len);
	if (W < 0) return -1;

	p_colPower = powl(p_col, (long double)W);
	/* the NIST tool asserts p_col^W is representable and below 1 */
	if ((p_colPower < LDBL_MIN) || (p_colPower > 1.0L - LDBL_EPSILON)) return -1;
	logProbNoColsPerPair = log1pl(-p_colPower);
	N = ((len - W + 1) * (len - W + 1) - (len - W + 1)) / 2;

	if (verbose > 1) {
		long double probNoCols = expl(((long double)N) * logProbNoColsPerPair);
		if (verbose > 2) {
			printf("%s Longest Repeated Substring results: P_col = %.22Lg\n", label, p_col);
			printf("%s Longest Repeated Substring results: W = %ld\n", label, W);
			printf("%s Longest Repeated Substring results: Pr(X >= 1) = %.22Lg\n", label, 1.0L - probNoCols);
		} else {
			printf("%s Longest Repeated Substring results\n", label);
			printf("\tP_col: %Lf\n", p_col);
			printf("\tLength of LRS: %ld\n", W);
			printf("\tPr(X >= 1): %Lf\n", 1.0L - probNoCols);
		}
	}

	return (logl(0.999L) >= ((long double)N) * logProbNoColsPerPair) ? 1 : 0;
}

#endif // LRS_TEST_H
