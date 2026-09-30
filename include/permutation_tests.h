/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef PERMUTATION_TESTS_H
#define PERMUTATION_TESTS_H

/*
 * Permutation testing, NIST SP 800-90B Section 5.1.
 *
 * C port of SP800-90B_EntropyAssessment 1.1.8, cpp/iid/permutation_tests.h:
 *   - the 19 test statistics of 5.1.1 to 5.1.11, including the bzip2
 *     compression statistic;
 *   - Conversion I (directional runs, periodicity, covariance) and
 *     Conversion II (collisions) for binary data; median 0.5 for binary data;
 *   - excursion, covariance (non-binary) and compression on the raw values;
 *   - 10 000 Fisher-Yates shuffles driven by xoshiro256** (one stream per
 *     permutation, reproducible with a caller seed);
 *   - counters C[i][0] (T > T_i), C[i][1] (T = T_i), C[i][2] (T < T_i); the
 *     IID assumption is rejected when C[i][0] + C[i][1] <= 5 or
 *     C[i][1] + C[i][2] <= 5 (Figure 4, step 3). A statistic stops being
 *     computed once both sums exceed 5, since its verdict can no longer change.
 */

#include <stdlib.h>
#include <bzlib.h>
#include "utils.h"
#include "test_case.h"

#define NUM_TESTS 19

static const char *test_names_c[NUM_TESTS] = {
	"excursion","numDirectionalRuns","lenDirectionalRuns","numIncreasesDecreases",
	"numRunsMedian","lenRunsMedian","avgCollision","maxCollision","periodicity(1)",
	"periodicity(2)","periodicity(8)","periodicity(16)","periodicity(32)",
	"covariance(1)","covariance(2)","covariance(8)","covariance(16)",
	"covariance(32)","compression"
};

static const unsigned int perm_lags[5] = { 1, 2, 8, 16, 32 };

/* Per-thread work buffers, allocated once per evaluation. */
typedef struct {
	uint8_t *cs1;       /* Conversion I output, ceil(n/8) bytes */
	uint8_t *cs2;       /* Conversion II output, ceil(n/8) bytes */
	bool dups[256];     /* collision search */
	char *msg;          /* compression: textual encoding */
	size_t msg_cap;
	char *dest;         /* compression: bzip2 output */
	unsigned int dest_cap;
} perm_work_t;

static inline bool perm_work_init(perm_work_t *w, long n, uint8_t max_symbol) {
	long nb = (n / 8) + ((n % 8 == 0) ? 0 : 1);

	memset(w, 0, sizeof(*w));
	w->cs1 = (uint8_t*)malloc(nb > 0 ? nb : 1);
	w->cs2 = (uint8_t*)malloc(nb > 0 ? nb : 1);
	/* "worst case" size of the NIST tool: n * (floor(log10(max)) + 2) + 1 */
	w->msg_cap = (size_t)(floor(log10(max_symbol > 0 ? max_symbol : 1)) + 2.0) * (size_t)n + 1;
	w->msg = (char*)malloc(w->msg_cap);
	w->dest_cap = (unsigned int)ceil(1.01 * (double)w->msg_cap) + 600;
	w->dest = (char*)malloc(w->dest_cap);
	return w->cs1 && w->cs2 && w->msg && w->dest;
}

static inline void perm_work_free(perm_work_t *w) {
	free(w->cs1);
	free(w->cs2);
	free(w->msg);
	free(w->dest);
}

/* 5.1 Conversion I: number of ones in each 8-bit block (zero padded). */
static inline long conversion1(const uint8_t data[], const long n, uint8_t *out) {
	long nb = (n / 8) + ((n % 8 == 0) ? 0 : 1);

	memset(out, 0, nb);
	for (long i = 0; i < n; ++i) out[i / 8] += data[i];
	return nb;
}

/* 5.1 Conversion II: integer value of each 8-bit block (zero padded). */
static inline long conversion2(const uint8_t data[], const long n, uint8_t *out) {
	long nb = (n / 8) + ((n % 8 == 0) ? 0 : 1);

	memset(out, 0, nb);
	for (long i = 0; i < n; ++i) out[i / 8] += data[i] << (7 - i % 8);
	return nb;
}

/* 5.1.1 Excursion test statistic (raw values). */
static inline double excursion_c(const uint8_t data[], const double rawmean, const long n) {
	double d_i, max = 0, running_sum = 0;

	for (long i = 0; i < n; ++i) {
		running_sum += data[i];
		d_i = fabs(running_sum - ((i + 1) * rawmean));
		if (d_i > max) max = d_i;
	}
	return max;
}

/* 5.1.2 to 5.1.4 on S' = (s'_1..s'_{m-1}), s'_i = -1 if s_i > s_{i+1}, else +1.
 * Mirrors num_directional_runs(), len_directional_runs() and
 * num_increases_decreases() without building S'. */
static inline void directional_c(const uint8_t x[], const long m, long double *num_runs, long double *len_runs, long double *incdec) {
	long size = m - 1, runs = 0, pos = 0;
	unsigned long max_run = 0, run = 1;
	int prev = 0, cur;

	if (size < 0) size = 0;
	if (size > 0) runs = 1;
	for (long i = 0; i < size; ++i) {
		cur = (x[i] > x[i + 1]) ? -1 : 1;
		if (cur == 1) pos++;
		if (i > 0) {
			if (cur != prev) {
				runs++;
				if (run > max_run) max_run = run;
				run = 1;
			} else {
				run++;
			}
		}
		prev = cur;
	}
	if (run > max_run) max_run = run;

	if (num_runs) *num_runs = runs;
	if (len_runs) *len_runs = max_run;
	if (incdec) *incdec = (pos > size - pos) ? pos : size - pos;
}

/* 5.1.5, 5.1.6 on s'_i = -1 if s_i < median, else +1. */
static inline void median_runs_c(const uint8_t x[], const double median, const long n, long double *num_runs, long double *len_runs) {
	long runs = (n > 0) ? 1 : 0;
	unsigned long max_run = 0, run = 1;
	int prev = 0, cur;

	for (long i = 0; i < n; ++i) {
		cur = (x[i] < median) ? -1 : 1;
		if (i > 0) {
			if (cur != prev) {
				runs++;
				if (run > max_run) max_run = run;
				run = 1;
			} else {
				run++;
			}
		}
		prev = cur;
	}
	if (run > max_run) max_run = run;

	if (num_runs) *num_runs = runs;
	if (len_runs) *len_runs = max_run;
}

/* 5.1.7, 5.1.8: find_collisions() then average (sum / count, NaN when no
 * collision, as divide() in the NIST tool) and maximum. */
static inline void collisions_c(const uint8_t data[], const long n, const int k, bool *dups, long double *avg, long double *max) {
	unsigned long i = 0, j = 0, count = 0, max_c = 0;
	unsigned int total = 0;

	while (i + j < (unsigned long)n) {
		for (int l = 0; l < k; l++) dups[l] = false;
		while (i + j < (unsigned long)n) {
			if (dups[data[i + j]]) {
				total += (unsigned int)(j + 1);
				count++;
				if (j + 1 > max_c) max_c = j + 1;
				i += j;
				j = 0;
				break;
			} else {
				dups[data[i + j]] = true;
				++j;
			}
		}
		++i;
	}
	if (avg) *avg = (double)(int)total / (double)(int)count;
	if (max) *max = max_c;
}

/* 5.1.9 Periodicity test statistic. */
static inline unsigned int periodicity_c(const uint8_t data[], const unsigned int p, const unsigned long n) {
	unsigned int T = 0;

	if (n < p) return 0;
	for (unsigned long i = 0; i < n - p; ++i)
		if (data[i] == data[i + p]) ++T;
	return T;
}

/* 5.1.10 Covariance test statistic: T = sum s_i * s_{i+p}. */
static inline unsigned long covariance_c(const uint8_t data[], const unsigned int p, const unsigned long n) {
	unsigned long T = 0;

	if (n < p) return 0;
	for (unsigned long i = 0; i < n - p; ++i)
		T += (unsigned long)data[i] * data[i + p];
	return T;
}

/* 5.1.11 Compression test statistic: length of the bzip2 compression
 * (block size 5) of the values written in decimal, separated by spaces. */
static inline unsigned int compression_c(const uint8_t data[], const long n, perm_work_t *w) {
	char *cur = w->msg;
	unsigned int curlen = 0, dest_len = w->dest_cap;
	int rc;

	for (long i = 0; i < n; ++i) {
		uint8_t v = data[i];
		/* sprintf("%u ") without the formatting overhead */
		if (v >= 100) { *cur++ = (char)('0' + v / 100); *cur++ = (char)('0' + (v / 10) % 10); *cur++ = (char)('0' + v % 10); curlen += 4; }
		else if (v >= 10) { *cur++ = (char)('0' + v / 10); *cur++ = (char)('0' + v % 10); curlen += 3; }
		else { *cur++ = (char)('0' + v); curlen += 2; }
		*cur++ = ' ';
	}
	if (curlen > 0) {
		cur--;
		*cur = '\0';
		curlen--;
	}

	rc = BZ2_bzBuffToBuffCompress(w->dest, &dest_len, w->msg, curlen, 5, 0, 0);
	return (rc == BZ_OK) ? dest_len : 0;
}

/* run_tests() of the NIST tool: computes the statistics still in play. */
static inline void run_tests_c(const data_t *dp, const uint8_t data[], const uint8_t rawdata[], const double rawmean, const double median,
                               long double *stats, const bool *st, perm_work_t *w) {
	const long n = dp->len;
	const bool binary = (dp->alph_size == 2);
	long m;

	if (st[0]) stats[0] = excursion_c(rawdata, rawmean, n);

	if (st[1] || st[2] || st[3]) {
		long double a, b, c;
		if (binary) {
			m = conversion1(data, n, w->cs1);
			directional_c(w->cs1, m, &a, &b, &c);
		} else {
			directional_c(data, n, &a, &b, &c);
		}
		if (st[1]) stats[1] = a;
		if (st[2]) stats[2] = b;
		if (st[3]) stats[3] = c;
	}

	if (st[4] || st[5]) {
		long double a, b;
		median_runs_c(data, binary ? 0.5 : median, n, &a, &b);
		if (st[4]) stats[4] = a;
		if (st[5]) stats[5] = b;
	}

	if (st[6] || st[7]) {
		long double a, b;
		if (binary) {
			m = conversion2(data, n, w->cs2);
			collisions_c(w->cs2, m, 256, w->dups, &a, &b);
		} else {
			collisions_c(data, n, dp->alph_size, w->dups, &a, &b);
		}
		if (st[6]) stats[6] = a;
		if (st[7]) stats[7] = b;
	}

	if (st[8] || st[9] || st[10] || st[11] || st[12] || st[13] || st[14] || st[15] || st[16] || st[17]) {
		const uint8_t *per = data, *cov = rawdata;
		long pn = n;
		if (binary) {
			/* covariance uses the converted data only when the two symbols are 0 and 1 */
			pn = conversion1(data, n, w->cs1);
			per = w->cs1;
			cov = w->cs1;
		}
		for (int l = 0; l < 5; l++) {
			if (st[8 + l]) stats[8 + l] = periodicity_c(per, perm_lags[l], (unsigned long)pn);
			if (st[13 + l]) stats[13 + l] = covariance_c(cov, perm_lags[l], (unsigned long)pn);
		}
	}

	if (st[18]) stats[18] = compression_c(rawdata, n, w);
}

static inline void print_perm_results(int C[][3]) {
	printf("\n\n");
	printf("                statistic  C[i][0]  C[i][1]  C[i][2]\n");
	printf("----------------------------------------------------\n");
	for (int i = 0; i < NUM_TESTS; ++i) {
		if ((C[i][0] + C[i][1] <= 5) || (C[i][1] + C[i][2] <= 5)) printf("%24s*", test_names_c[i]);
		else printf("%25s", test_names_c[i]);
		printf("%8d%8d%8d\n", C[i][0], C[i][1], C[i][2]);
	}
	printf("(* denotes failed test)\n\n");
}

/*
 * Returns 1 if the IID assumption holds, 0 if it is rejected, -1 on error
 * (allocation or seeding failure). C receives the counters.
 */
static inline int permutation_tests(const data_t *dp, const double rawmean, const double median, const int verbose,
                                    const uint64_t *user_seed, int C[NUM_TESTS][3]) {
	uint64_t main_seed[4];
	long double t[NUM_TESTS];
	bool test_status[NUM_TESTS];
	perm_work_t w0;
	int error = 0;

	if (!dp || dp->len < 2) return -1;

	for (int i = 0; i < NUM_TESTS; ++i) {
		C[i][0] = C[i][1] = C[i][2] = 0;
		t[i] = -1;
		test_status[i] = true;
	}

	if (!xoshiro_seed(main_seed, user_seed)) return -1;
	if (!perm_work_init(&w0, dp->len, dp->maxsymbol)) {
		perm_work_free(&w0);
		return -1;
	}
	run_tests_c(dp, dp->symbols, dp->rawsymbols, rawmean, median, t, test_status, &w0);
	perm_work_free(&w0);

	if (verbose == 2) {
		printf("\nInitial test results\n");
		for (int i = 0; i < NUM_TESTS; i++) printf("%23s: %Lg\n", test_names_c[i], t[i]);
		printf("\n");
	} else if (verbose >= 3) {
		for (int i = 0; i < NUM_TESTS; i++)
			printf("Permutation testing: Unpermuted result %s = %.22Lg\n", test_names_c[i], t[i]);
	}

	/*
	 * The 10 000 permutations are computed in blocks, in parallel. Permutation
	 * i shuffles the original data with its own stream xoshiro_stream(seed, i),
	 * and the counters are updated in the order of i, a statistic leaving the
	 * count as soon as its verdict is settled. The counters and the verdict
	 * therefore depend only on the seed, not on the number of threads.
	 * (The NIST tool shuffles the previous permutation again with one stream
	 * per thread; both give uniformly random permutations, Figure 5.)
	 */
	{
		const int nthreads = omp_get_max_threads();
		const int block = (2 * nthreads > 16) ? 2 * nthreads : 16;
		signed char (*res)[NUM_TESTS] = malloc(sizeof(*res) * block);
		bool done = false;

		if (!res) return -1;

		#pragma omp parallel
		{
			uint8_t *data = (uint8_t*)malloc(dp->len);
			uint8_t *rawdata = (uint8_t*)malloc(dp->len);
			long double tp[NUM_TESTS];
			uint64_t seed[4];
			perm_work_t w;
			bool ok = perm_work_init(&w, dp->len, dp->maxsymbol) && data && rawdata;

			if (!ok) {
				#pragma omp atomic write
				error = 1;
			}
			#pragma omp barrier

			for (int base = 0; !done; base += block) {
				#pragma omp for schedule(dynamic, 1)
				for (int k = 0; k < block; k++) {
					int idx = base + k;
					int err;

					for (int j = 0; j < NUM_TESTS; j++) res[k][j] = -1;
					#pragma omp atomic read
					err = error;
					if (!ok || err || idx >= PERMS) continue;

					memcpy(data, dp->symbols, dp->len);
					memcpy(rawdata, dp->rawsymbols, dp->len);
					xoshiro_stream(main_seed, (uint64_t)idx, seed);
					FYshuffle(data, rawdata, dp->len, seed);
					run_tests_c(dp, data, rawdata, rawmean, median, tp, test_status, &w);
					for (int j = 0; j < NUM_TESTS; j++) {
						if (!test_status[j]) continue;
						res[k][j] = (tp[j] > t[j]) ? 0 : ((tp[j] == t[j]) ? 1 : 2);
					}
				}
				/* implicit barrier: the block is complete */
				#pragma omp single
				{
					int active = 0;

					for (int k = 0; k < block; k++) {
						for (int j = 0; j < NUM_TESTS; j++) {
							if (!test_status[j] || res[k][j] < 0) continue;
							C[j][res[k][j]]++;
							if ((C[j][0] + C[j][1] > 5) && (C[j][1] + C[j][2] > 5)) test_status[j] = false;
						}
					}
					for (int j = 0; j < NUM_TESTS; j++) if (test_status[j]) active++;
					if (error || active == 0 || base + block >= PERMS) done = true;
				}
				/* implicit barrier: every thread sees the same `done` */
			}
			perm_work_free(&w);
			free(data);
			free(rawdata);
		}
		free(res);
	}

	if (error) return -1;
	if (verbose > 1) print_perm_results(C);

	for (int i = 0; i < NUM_TESTS; ++i)
		if ((C[i][0] + C[i][1] <= 5) || (C[i][1] + C[i][2] <= 5)) return 0;
	return 1;
}

#endif // PERMUTATION_TESTS_H
