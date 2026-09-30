/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
/*
 * sp80090b_api.c - public API of libsp80090b
 *
 * Orchestration of the NIST SP 800-90B estimators, following the NIST
 * reference tool 1.1.8: iid_main.cpp, non_iid_main.cpp, restart_main.cpp
 * and conditioning_main.cpp.
 */
#include "sp80090b.h"
#include "utils.h"
#include "most_common.h"
#include "lrs_test.h"
#include "permutation_tests.h"
#include "chi_square_tests.h"
#include "collision_test.h"
#include "markov_test.h"
#include "compression_test.h"
#include "multi_mcw_test.h"
#include "multi_mmc_test.h"
#include "lz78y_test.h"
#include "lag_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fenv.h>

const char *sp80090b_version(void)
{
	return SP80090B_LIB_VERSION " (NIST SP800-90B_EntropyAssessment " SP80090B_NIST_VERSION ")";
}

sp80090b_options_t sp80090b_default_options(void)
{
	sp80090b_options_t o;

	memset(&o, 0, sizeof(o));
	o.bits_per_symbol = 0;
	o.initial_entropy = true;
	o.truncate_bitstring = false;
	o.verbose = 0;
	o.seed = NULL;
	return o;
}

static int set_error(int *status, char *msg, size_t msg_len, int rc, const char *text)
{
	*status = rc;
	snprintf(msg, msg_len, "%s", text);
	return rc;
}

#define FAIL(r, rc, text) set_error(&(r)->status, (r)->error_msg, sizeof((r)->error_msg), (rc), (text))

/*
 * Common front end of the IID and non-IID tracks: argument checks, reading
 * and translation of the samples (read_file_subset() of the NIST tool),
 * warnings of 3.1.1, and the -t truncation of the bitstring.
 */
static int load(const uint8_t *raw, size_t len, const sp80090b_options_t *opt, data_t *data, sp80090b_result_t *r)
{
	test_run_t run;

	memset(data, 0, sizeof(*data));
	memset(&run, 0, sizeof(run));

	if (!raw || len < 2 || !opt)
		return FAIL(r, SP80090B_EINVAL, "Invalid argument: data is NULL or holds fewer than 2 samples");
	if (opt->bits_per_symbol < 0 || opt->bits_per_symbol > 8)
		return FAIL(r, SP80090B_EINVAL, "Invalid bits per symbol: must be 0 (inferred) or 1 to 8");
	if (len > (size_t)INT32_MAX)
		return FAIL(r, SP80090B_EINVAL, "Invalid argument: more than 2^31 - 1 samples");

	data->word_size = opt->bits_per_symbol;
	if (!read_buffer(raw, len, data, &run))
		return FAIL(r, SP80090B_EDATA, run.errorMsg[0] ? run.errorMsg : "Error reading data");
	if (run.errorMsg[0]) {
		/* read_buffer() only leaves a message on success for this warning */
		r->warnings |= SP80090B_WARN_NARROW_DATA;
		snprintf(r->error_msg, sizeof(r->error_msg), "%s", run.errorMsg);
	}

	r->n_samples = data->len;
	r->bits_per_symbol = data->word_size;
	r->alphabet_size = data->alph_size;

	if (data->alph_size <= 1) {
		free_data(data);
		return FAIL(r, SP80090B_EALPHABET, "Symbol alphabet consists of 1 symbol. No entropy awarded...");
	}
	if (data->len < SP80090B_MIN_SAMPLES) {
		r->warnings |= SP80090B_WARN_FEW_SAMPLES;
		snprintf(r->error_msg, sizeof(r->error_msg), "Warning: data contains less than %d samples", SP80090B_MIN_SAMPLES);
	}
	if (opt->truncate_bitstring && (data->blen > SP80090B_MIN_SAMPLES)) {
		data->blen = SP80090B_MIN_SAMPLES;
		r->warnings |= SP80090B_WARN_TRUNCATED;
	}
	if (opt->verbose > 1) {
		printf("Loaded %ld samples of %d distinct %d-bit-wide symbols\n", data->len, data->alph_size, data->word_size);
		if ((data->alph_size > 2) || !opt->initial_entropy) printf("Number of Binary samples: %ld\n", data->blen);
		if (data->alph_size < (1 << data->word_size)) printf("\nSamples have been translated\n");
	}
	if ((opt->verbose > 0) && (data->len < SP80090B_MIN_SAMPLES))
		printf("\n*** Warning: data contains less than %d samples ***\n\n", SP80090B_MIN_SAMPLES);
	return SP80090B_OK;
}

static double combine(const data_t *data, bool initial, double h_original, double h_bitstring)
{
	double h = data->word_size;

	if ((data->alph_size > 2) || !initial) h = MIN(h, h_bitstring * data->word_size);
	if (initial) h = MIN(h, h_original);
	return h;
}

int sp80090b_iid_evaluate_ex(const uint8_t *raw, size_t len, const sp80090b_options_t *opt, sp80090b_result_t *r)
{
	data_t data;
	double rawmean, median, pvalue = 1.0;
	int verbose, lrs, perm, rc;

	if (!r)
		return SP80090B_EINVAL;
	memset(r, 0, sizeof(*r));
	if ((rc = load(raw, len, opt, &data, r)) != SP80090B_OK)
		return rc;
	verbose = opt->verbose;

	/* baseline statistics */
	calc_stats(&data, &rawmean, &median);
	if (verbose == 2) {
		printf("\tRaw Mean: %f\n", rawmean);
		printf("\tMedian: %f\n", median);
		printf("\tBinary: %s\n\n", (data.alph_size == 2 ? "true" : "false"));
	} else if (verbose > 2) {
		printf("Raw Mean = %.17g\n", rawmean);
		printf("Median = %.17g\n", median);
		printf("Binary = %s\n", (data.alph_size == 2 ? "true" : "false"));
	}

	/* min-entropy: most common value estimate, 6.3.1 */
	r->h_original = data.word_size;
	r->h_bitstring = 1.0;
	if (opt->initial_entropy)
		r->h_original = most_common(data.symbols, data.len, data.alph_size, verbose, "Literal");
	if ((data.alph_size > 2) || !opt->initial_entropy)
		r->h_bitstring = most_common(data.bsymbols, data.blen, 2, verbose, "Bitstring");
	r->h_assessed = combine(&data, opt->initial_entropy, r->h_original, r->h_bitstring);
	if (verbose > 2) {
		if ((data.alph_size > 2) || !opt->initial_entropy) {
			printf("H_bitstring = %.17g\n", r->h_bitstring);
			printf("H_bitstring Per Symbol = %.17g\n", r->h_bitstring * data.word_size);
		}
		if (opt->initial_entropy) printf("H_original = %.17g\n", r->h_original);
		printf("Assessed min entropy: %.17g\n", r->h_assessed);
	}

	/* IID tests, Section 5: all on the translated literal symbols */
	r->passed_chi_square = chi_square_tests(data.symbols, data.len, data.alph_size, verbose, &pvalue);
	r->p_value = pvalue;
	if (verbose > 2) printf("Chi square tests: %s\n", r->passed_chi_square ? "Passed" : "Failed");

	lrs = len_LRS_test(data.symbols, data.len, data.alph_size, verbose, "Literal");
	if (lrs < 0) {
		free_data(&data);
		return FAIL(r, SP80090B_EINTERNAL, "Error in the longest repeated substring test (memory or precision)");
	}
	r->passed_lrs = (lrs == 1);
	if (verbose > 2) printf("Length of longest repeated substring test: %s\n", r->passed_lrs ? "Passed" : "Failed");

	perm = permutation_tests(&data, rawmean, median, verbose, opt->seed, r->perm_counts);
	if (perm < 0) {
		free_data(&data);
		return FAIL(r, SP80090B_EINTERNAL, "Error in the permutation tests (memory or random seed)");
	}
	r->passed_permutation = (perm == 1);

	r->passed_iid = r->passed_chi_square && r->passed_lrs && r->passed_permutation;
	if (verbose > 2) printf("IID permutation tests: %s\n", r->passed_permutation ? "Passed" : "Failed");
	r->status = SP80090B_OK;
	free_data(&data);
	return SP80090B_OK;
}

/*
 * The ten estimators of 6.3, applied as in non_iid_main.cpp: on the
 * bitstring when the alphabet is not binary (or with -c), on the literal
 * symbols with -i; collision, Markov and compression are for binary
 * alphabets only on the literal symbols.
 */
static void non_iid_estimates(const data_t *data, bool initial, bool literal_only, int verbose, double *h_original, double *h_bitstring)
{
	const bool bits = !literal_only && ((data->alph_size > 2) || !initial);
	const bool lit_bin = initial && (data->alph_size == 2);
	double e, t1, t2;
	double ho = data->word_size, hb = 1.0;

	/* 6.3.1 most common value */
	if (bits) hb = MIN(most_common(data->bsymbols, data->blen, 2, verbose, "Bitstring"), hb);
	if (initial) ho = MIN(most_common(data->symbols, data->len, data->alph_size, verbose, "Literal"), ho);

	/* 6.3.2 collision, 6.3.3 Markov, 6.3.4 compression */
	if (bits) {
		hb = MIN(collision_test(data->bsymbols, data->blen, verbose, "Bitstring"), hb);
		hb = MIN(markov_test(data->bsymbols, data->blen, verbose, "Bitstring"), hb);
		if ((e = compression_test(data->bsymbols, data->blen, verbose, "Bitstring")) >= 0) hb = MIN(e, hb);
	}
	if (lit_bin) {
		ho = MIN(collision_test(data->symbols, data->len, verbose, "Literal"), ho);
		ho = MIN(markov_test(data->symbols, data->len, verbose, "Literal"), ho);
		if ((e = compression_test(data->symbols, data->len, verbose, "Literal")) >= 0) ho = MIN(e, ho);
	}

	/* 6.3.5 t-tuple, 6.3.6 LRS */
	if (bits) {
		t1 = t2 = -1.0;
		SAalgs(data->bsymbols, data->blen, 2, &t1, &t2, verbose, "Bitstring");
		if (t1 >= 0.0) hb = MIN(t1, hb);
		if (t2 >= 0.0) hb = MIN(t2, hb);
	}
	if (initial) {
		t1 = t2 = -1.0;
		SAalgs(data->symbols, data->len, data->alph_size, &t1, &t2, verbose, "Literal");
		if (t1 >= 0.0) ho = MIN(t1, ho);
		if (t2 >= 0.0) ho = MIN(t2, ho);
	}

	/* 6.3.7 MultiMCW, 6.3.8 lag, 6.3.9 MultiMMC, 6.3.10 LZ78Y */
	if (bits) {
		if ((e = multi_mcw_test(data->bsymbols, data->blen, 2, verbose, "Bitstring")) >= 0) hb = MIN(e, hb);
		if ((e = lag_test(data->bsymbols, data->blen, 2, verbose, "Bitstring")) >= 0) hb = MIN(e, hb);
		if ((e = multi_mmc_test(data->bsymbols, data->blen, 2, verbose, "Bitstring")) >= 0) hb = MIN(e, hb);
		if ((e = LZ78Y_test(data->bsymbols, data->blen, 2, verbose, "Bitstring")) >= 0) hb = MIN(e, hb);
	}
	if (initial) {
		if ((e = multi_mcw_test(data->symbols, data->len, data->alph_size, verbose, "Literal")) >= 0) ho = MIN(e, ho);
		if ((e = lag_test(data->symbols, data->len, data->alph_size, verbose, "Literal")) >= 0) ho = MIN(e, ho);
		if ((e = multi_mmc_test(data->symbols, data->len, data->alph_size, verbose, "Literal")) >= 0) ho = MIN(e, ho);
		if ((e = LZ78Y_test(data->symbols, data->len, data->alph_size, verbose, "Literal")) >= 0) ho = MIN(e, ho);
	}

	*h_original = ho;
	*h_bitstring = hb;
}

int sp80090b_non_iid_evaluate_ex(const uint8_t *raw, size_t len, const sp80090b_options_t *opt, sp80090b_result_t *r)
{
	data_t data;
	int rc;

	if (!r)
		return SP80090B_EINVAL;
	memset(r, 0, sizeof(*r));
	if ((rc = load(raw, len, opt, &data, r)) != SP80090B_OK)
		return rc;

	non_iid_estimates(&data, opt->initial_entropy, false, opt->verbose, &r->h_original, &r->h_bitstring);
	r->h_assessed = combine(&data, opt->initial_entropy, r->h_original, r->h_bitstring);
	if (opt->verbose > 2) {
		if ((data.alph_size > 2) || !opt->initial_entropy) {
			printf("H_bitstring = %.17g\n", r->h_bitstring);
			printf("H_bitstring Per Symbol = %.17g\n", r->h_bitstring * data.word_size);
		}
		if (opt->initial_entropy) printf("H_original = %.17g\n", r->h_original);
		printf("Assessed min entropy: %.17g\n", r->h_assessed);
	}
	r->passed_iid = false;
	r->status = SP80090B_OK;
	free_data(&data);
	return SP80090B_OK;
}

int sp80090b_iid_evaluate(const uint8_t *data, size_t len, int bits_per_symbol, bool initial_entropy, int verbose, sp80090b_result_t *result)
{
	sp80090b_options_t o = sp80090b_default_options();

	o.bits_per_symbol = bits_per_symbol;
	o.initial_entropy = initial_entropy;
	o.verbose = verbose;
	return sp80090b_iid_evaluate_ex(data, len, &o, result);
}

int sp80090b_non_iid_evaluate(const uint8_t *data, size_t len, int bits_per_symbol, bool initial_entropy, int verbose, sp80090b_result_t *result)
{
	sp80090b_options_t o = sp80090b_default_options();

	o.bits_per_symbol = bits_per_symbol;
	o.initial_entropy = initial_entropy;
	o.verbose = verbose;
	return sp80090b_non_iid_evaluate_ex(data, len, &o, result);
}

/* ------------------------------------------------------------------ */
/* Restart tests, 3.1.4                                               */
/* ------------------------------------------------------------------ */

/*
 * Largest count among 1000 draws of the "inverted near-uniform"
 * distribution whose most likely symbol has probability p (Hagerty-Draper),
 * as simulateCount() of the NIST tool.
 */
static uint16_t simulate_count(int k_effective, double p, uint64_t *state)
{
	uint16_t counts[257] = { 0 }, max_count = 0;

	for (int j = 0; j < 1000; j++)
		counts[(int)floor(randomUnit(state) / p)]++;
	for (int j = 0; j < k_effective; j++)
		if (max_count < counts[j])
			max_count = counts[j];
	return max_count;
}

/*
 * Cutoff of the sanity check (simulateBound()): the (1 - alpha) quantile
 * of simulate_count() over `rounds` rounds. A histogram of the maxima
 * (0..1000) replaces the sort of the NIST tool; the selected order
 * statistic is the same. The NIST tool uses one jumped stream per thread;
 * here each block of rounds has its own stream (reproducible with a seed).
 */
static int simulate_bound(double alpha, double h_i, unsigned long rounds, const uint64_t *user_seed, int *cutoff)
{
	unsigned long hist[1001] = { 0 }, cum = 0, idx;
	uint64_t main_seed[4];
	const double p = pow(2.0, -h_i);
	const int k_eff = (int)ceil(1.0 / p);

	if (!xoshiro_seed(main_seed, user_seed))
		return -1;
	idx = (unsigned long)floor((1.0 - alpha) * (double)rounds) - 1;

	/* blocks of 65536 rounds, block b driven by xoshiro_stream(seed, b):
	 * the cutoff depends only on the seed, not on the number of threads */
	#pragma omp parallel
	{
		unsigned long local[1001] = { 0 };
		const long nblocks = (long)((rounds + 65535UL) / 65536UL);
		uint64_t s[4];

		#pragma omp for schedule(dynamic, 1)
		for (long b = 0; b < nblocks; b++) {
			unsigned long end = (unsigned long)(b + 1) * 65536UL;

			if (end > rounds) end = rounds;
			xoshiro_stream(main_seed, (uint64_t)b, s);
			for (unsigned long i = (unsigned long)b * 65536UL; i < end; i++)
				local[simulate_count(k_eff, p, s)]++;
		}
		#pragma omp critical(restartHist)
		for (int v = 0; v <= 1000; v++)
			hist[v] += local[v];
	}
	for (int v = 0; v <= 1000; v++) {
		cum += hist[v];
		if (cum > idx) {
			*cutoff = v;
			return 0;
		}
	}
	*cutoff = 1000;
	return 0;
}

/* Largest count of a single value in any row of a 1000 x 1000 matrix. */
static int max_row_count(const uint8_t *m)
{
	int best = 0;

	for (int i = 0; i < SP80090B_RESTART_ROWS; i++) {
		int counts[256] = { 0 };

		for (int j = 0; j < SP80090B_RESTART_COLS; j++)
			if (++counts[m[i * SP80090B_RESTART_COLS + j]] > best)
				best = counts[m[i * SP80090B_RESTART_COLS + j]];
	}
	return best;
}

int sp80090b_restart_evaluate(const uint8_t *raw, size_t len, int bits, double h_i, bool iid,
                              unsigned long rounds, const uint64_t *seed, int verbose,
                              sp80090b_restart_result_t *r)
{
	const size_t n = (size_t)SP80090B_RESTART_ROWS * SP80090B_RESTART_COLS;
	sp80090b_options_t opt = sp80090b_default_options();
	sp80090b_result_t tmp;
	data_t data, col;
	uint8_t *cdata = NULL, *crawdata = NULL;
	int rc;

	if (!r)
		return SP80090B_EINVAL;
	memset(r, 0, sizeof(*r));
	r->h_i = h_i;
	r->iid = iid;
	r->h_r = r->h_c = NAN;

	if (!raw)
		return FAIL(r, SP80090B_EINVAL, "Invalid argument: data is NULL");
	if (len != n)
		return FAIL(r, SP80090B_EINVAL, "Restart data must contain 1000 x 1000 = 1000000 samples");
	if (!(h_i >= 0.0) || !isfinite(h_i))
		return FAIL(r, SP80090B_EINVAL, "H_I must be nonnegative");
	if (rounds == 0)
		rounds = SP80090B_DEFAULT_SIMULATION_ROUNDS;
	if (rounds < SP80090B_DEFAULT_SIMULATION_ROUNDS)
		return FAIL(r, SP80090B_EINVAL, "Simulation rounds must be at least 5000000");

	opt.bits_per_symbol = bits;
	opt.verbose = verbose;
	memset(&tmp, 0, sizeof(tmp));
	if ((rc = load(raw, len, &opt, &data, &tmp)) != SP80090B_OK) {
		r->bits_per_symbol = tmp.bits_per_symbol;
		r->alphabet_size = tmp.alphabet_size;
		return FAIL(r, rc, tmp.error_msg);
	}
	r->bits_per_symbol = data.word_size;
	r->alphabet_size = data.alph_size;
	if (h_i > data.word_size) {
		free_data(&data);
		return FAIL(r, SP80090B_EINVAL, "H_I must be at most bits_per_symbol");
	}
	if ((int)ceil(1.0 / pow(2.0, -h_i)) > data.alph_size) {
		/* the NIST tool asserts k_effective <= k */
		free_data(&data);
		return FAIL(r, SP80090B_EINVAL, "H_I exceeds log2 of the number of distinct symbols observed");
	}

	/* column dataset: column j = sample j of every restart */
	cdata = (uint8_t *)malloc(n);
	crawdata = (uint8_t *)malloc(n);
	if (!cdata || !crawdata) {
		free(cdata);
		free(crawdata);
		free_data(&data);
		return FAIL(r, SP80090B_EINTERNAL, "Error: failure to initialize memory for columns");
	}
	for (int i = 0; i < SP80090B_RESTART_ROWS; i++)
		for (int j = 0; j < SP80090B_RESTART_COLS; j++) {
			cdata[j * SP80090B_RESTART_ROWS + i] = data.symbols[i * SP80090B_RESTART_COLS + j];
			crawdata[j * SP80090B_RESTART_ROWS + i] = data.rawsymbols[i * SP80090B_RESTART_COLS + j];
		}

	/* 3.1.4.3 sanity check */
	r->alpha = 1.0 - exp(log(0.99) / (SP80090B_RESTART_ROWS + SP80090B_RESTART_COLS));
	if (simulate_bound(r->alpha, h_i, rounds, seed, &r->x_cutoff) != 0) {
		rc = FAIL(r, SP80090B_EINTERNAL, "Error: cannot seed the simulation");
		goto out;
	}
	r->x_r = max_row_count(data.symbols);
	r->x_c = max_row_count(cdata);
	r->x_max = MAX(r->x_r, r->x_c);
	if (verbose > 0) {
		printf("H_I: %f\n", h_i);
		printf("ALPHA: %.17g, X_cutoff: %d\n", r->alpha, r->x_cutoff);
		printf("X_max: %d\n", r->x_max);
	}
	r->sanity_passed = (r->x_max <= r->x_cutoff);
	if (!r->sanity_passed) {
		if (verbose > 0) printf("\n*** Restart Sanity Check Failed ***\n");
		snprintf(r->error_msg, sizeof(r->error_msg), "Restart Sanity Check Failed.");
		rc = SP80090B_OK;
		goto out;
	}

	/* 3.1.4.2 validation: literal estimates on the rows and on the columns */
	col = data;
	col.symbols = cdata;
	col.rawsymbols = crawdata;
	if (iid) {
		double rawmean, median;
		int C[SP80090B_PERM_TESTS][3];
		int pr, pc, lr, lc;
		bool xr, xc;

		/* ea_restart -i: most common value only, plus the IID tests */
		r->h_r = MIN(most_common(data.symbols, data.len, data.alph_size, verbose, "Literal"), (double)data.word_size);
		r->h_c = MIN(most_common(cdata, data.len, data.alph_size, verbose, "Literal"), (double)data.word_size);
		calc_stats(&data, &rawmean, &median);
		xr = chi_square_tests(data.symbols, data.len, data.alph_size, verbose, NULL);
		xc = chi_square_tests(cdata, data.len, data.alph_size, verbose, NULL);
		lr = len_LRS_test(data.symbols, data.len, data.alph_size, verbose, "Literal");
		lc = len_LRS_test(cdata, data.len, data.alph_size, verbose, "Literal");
		pr = permutation_tests(&data, rawmean, median, verbose, seed, C);
		pc = permutation_tests(&col, rawmean, median, verbose, seed, C);
		if (lr < 0 || lc < 0 || pr < 0 || pc < 0) {
			rc = FAIL(r, SP80090B_EINTERNAL, "Error in the IID tests of the restart data");
			goto out;
		}
		r->passed_iid_rows = xr && lr == 1 && pr == 1;
		r->passed_iid_cols = xc && lc == 1 && pc == 1;
	} else {
		double unused;

		/* ea_restart: literal symbols only, no bitstring */
		non_iid_estimates(&data, true, true, verbose, &r->h_r, &unused);
		non_iid_estimates(&col, true, true, verbose, &r->h_c, &unused);
	}
	if (verbose > 0)
		printf("\nH_r: %f\nH_c: %f\nH_I: %f\n\n", r->h_r, r->h_c, h_i);

	r->validation_passed = (MIN(r->h_r, r->h_c) >= h_i / 2.0);
	r->passed = r->validation_passed;
	if (r->passed) {
		r->h_restart = MIN(MIN(r->h_r, r->h_c), h_i);
		if (verbose > 0) printf("Validation Test Passed...\n\nmin(H_r, H_c, H_I): %f\n\n", r->h_restart);
	} else {
		snprintf(r->error_msg, sizeof(r->error_msg), "min(H_r, H_c) < H_I/2, Validation Testing Failed.");
		if (verbose > 0) printf("*** min(H_r, H_c) < H_I/2, Validation Testing Failed ***\n");
	}
	rc = SP80090B_OK;

out:
	r->status = rc;
	free(cdata);
	free(crawdata);
	free_data(&data);
	return rc;
}

/* ------------------------------------------------------------------ */
/* Conditioning components, 3.1.5                                     */
/* ------------------------------------------------------------------ */

/*
 * Output_Entropy(n_in, n_out, nw, h_in) of 3.1.5.1.2, with n = min(n_out, nw),
 * P_high = 2^-h_in, P_low = (1 - P_high) / (2^n_in - 1),
 * psi = 2^(n_in - n) P_low + P_high, U = 2^(n_in - n) + sqrt(2 n 2^(n_in - n) ln 2),
 * omega = U P_low, Output_Entropy = -log2(max(psi, omega)).
 *
 * The NIST tool evaluates it with MPFR. It is rewritten here in closed form
 * around n, which needs only long double and log1p:
 *   A = log2(1 - 2^-h_in) - log2(1 - 2^-n_in)          (A <= 0)
 *   s = sqrt(2 n ln 2) 2^(-(n_in - n)/2)
 *   Output_Entropy = n - A - log2(1 + max(s, 2^(n - h_in - A)))
 * so that n - Output_Entropy, hence epsilon, stays exact even near 2^-130.
 */
static const long double LN2L = 0.693147180559945309417232121458176568L;

static long double log2_1m_pow2(long double e)   /* log2(1 - 2^-e), e > 0 */
{
	return log1pl(-exp2l(-e)) / LN2L;
}

static long double output_deficit(unsigned int n_in, unsigned int n, long double h_in)
{
	long double a = log2_1m_pow2(h_in) - log2_1m_pow2((long double)n_in);
	long double s = sqrtl(2.0L * n * LN2L) * exp2l(-((long double)n_in - n) / 2.0L);
	long double t = exp2l((long double)n - h_in - a);

	return -a + log1pl(s > t ? s : t) / LN2L;
}

int sp80090b_conditioning_evaluate(bool vetted, unsigned int n_in, unsigned int n_out, unsigned int nw,
                                   long double h_in, long double h_prime, sp80090b_conditioning_result_t *r)
{
	unsigned int n;
	long double delta, h_oe;
	int round_mode;

	if (!r)
		return SP80090B_EINVAL;
	memset(r, 0, sizeof(*r));
	if (n_in == 0 || n_out == 0 || nw == 0)
		return FAIL(r, SP80090B_EINVAL, "n_in, n_out and nw must be positive");
	if (!(h_in > 0.0L) || h_in > (long double)n_in || !isfinite(h_in))
		return FAIL(r, SP80090B_EINVAL, "h_in must be in ]0, n_in]");
	if (!vetted && (!(h_prime > 0.0L) || h_prime > 1.0L))
		return FAIL(r, SP80090B_EINVAL, "h' must be in ]0, 1] for a non-vetted function");

	/* Rounding as ea_conditioning: Output_Entropy is computed accurately
	 * and rounded to nearest (MPFR then mpfr_get_ld(RNDN) in the NIST tool),
	 * the other long double operations round toward zero so that no bound
	 * is overestimated. The caller's rounding mode is restored on return. */
	round_mode = fegetround();
	fesetround(FE_TONEAREST);

	if (nw > n_in)
		nw = n_in;                       /* Appendix E */
	r->nw = nw;
	n = n_out < nw ? n_out : nw;

	delta = output_deficit(n_in, n, h_in);
	h_oe = (long double)n - delta;
	if (h_oe < 0.0L) h_oe = 0.0L;
	if (h_oe > h_in) h_oe = h_in;
	r->output_entropy = h_oe;

	/* epsilon such that h_out = (1 - epsilon) n_out; exact when n = n_out */
	r->epsilon_log2 = (n == n_out) ? log2l(delta / n_out) : log2l(1.0L - h_oe / n_out);
	r->epsilon_nw_log2 = (n == nw) ? log2l(delta / nw) : log2l(1.0L - h_oe / nw);

	fesetround(FE_TOWARDZERO);
	r->h_out = h_oe;
	if (!vetted) {
		long double b90 = 0.999L * n_out, bstat = h_prime * n_out;

		if (b90 < r->h_out) r->h_out = b90;
		if (bstat < r->h_out) r->h_out = bstat;
	}
	r->full_entropy_candidate = vetted && (r->h_out > 0.999L * n_out) && (h_in >= (long double)n_out + 64.0L);
	fesetround(round_mode);
	r->status = SP80090B_OK;
	return SP80090B_OK;
}
