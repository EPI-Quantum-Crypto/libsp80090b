/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * test_selftest.c - self-test of libsp80090b
 *
 * 1. Non-IID track against values produced by the NIST tool (ea_non_iid).
 * 2. Conditioning against values produced by ea_conditioning.
 * 3. API contract: argument checks, warnings, reproducible IID permutations.
 *
 * Run from SP800-90B_lib/ (reads the files of bin/). Returns the number of failures.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "sp80090b.h"

static int failures = 0;

static void check(bool ok, const char *what)
{
	printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static uint8_t *read_file(const char *path, size_t max, size_t *len)
{
	FILE *f = fopen(path, "rb");
	uint8_t *buf;

	if (!f)
		return NULL;
	buf = (uint8_t *)malloc(max);
	*len = buf ? fread(buf, 1, max, f) : 0;
	fclose(f);
	return buf;
}

int main(void)
{
	static const struct { const char *file; int bits; double h; } noniid[] = {
		/* ea_non_iid, first 1 000 000 samples */
		{ "bin/truerand_8bit.bin", 8, 7.233861455178495 },
		{ "bin/biased-random-bytes.bin", 8, 0.25774087100648113 },
		{ "bin/normal.bin", 8, 4.1000957831103086 },
	};
	static const struct { bool vetted; unsigned n_in, n_out, nw; long double h_in, h_p, h_out; } cond[] = {
		/* ea_conditioning, printed with %.22Lg */
		{ true, 256, 256, 256, 256.0L, 0.0L, 251.689764568727055119L },
		{ true, 512, 256, 256, 400.0L, 0.0L, 256.0L },
		{ true, 256, 128, 256, 200.0L, 0.0L, 128.0L },
		{ false, 320, 256, 256, 320.0L, 0.9L, 230.3999999999999999944L },
	};
	sp80090b_options_t o = sp80090b_default_options();
	sp80090b_result_t r, r2;
	sp80090b_conditioning_result_t c;
	uint8_t small[1000], *buf;
	size_t len;
	int rc;

	printf("libsp80090b %s\n\n", sp80090b_version());

	printf("1. Non-IID track against ea_non_iid\n");
	for (size_t i = 0; i < sizeof noniid / sizeof noniid[0]; i++) {
		char what[160];

		buf = read_file(noniid[i].file, SP80090B_MIN_SAMPLES, &len);
		if (!buf) {
			printf("  [SKIP] %s not found (run from SP800-90B_lib/)\n", noniid[i].file);
			continue;
		}
		rc = sp80090b_non_iid_evaluate(buf, len, noniid[i].bits, true, 0, &r);
		free(buf);
		snprintf(what, sizeof what, "%s: H = %.12f, NIST %.12f", noniid[i].file, r.h_assessed, noniid[i].h);
		check(rc == 0 && fabs(r.h_assessed - noniid[i].h) < 1e-9, what);
	}

	printf("\n2. Conditioning against ea_conditioning\n");
	for (size_t i = 0; i < sizeof cond / sizeof cond[0]; i++) {
		char what[160];

		rc = sp80090b_conditioning_evaluate(cond[i].vetted, cond[i].n_in, cond[i].n_out, cond[i].nw, cond[i].h_in, cond[i].h_p, &c);
		snprintf(what, sizeof what, "%s %u %u %u: h_out = %.19Lg", cond[i].vetted ? "vetted" : "non-vetted",
			 cond[i].n_in, cond[i].n_out, cond[i].nw, c.h_out);
		check(rc == 0 && fabsl(c.h_out - cond[i].h_out) <= 1e-17L * cond[i].h_out, what);
	}

	printf("\n3. API contract\n");
	for (size_t i = 0; i < sizeof small; i++)
		small[i] = (uint8_t)((i * 7u + (i >> 3)) & 0x0f);
	check(sp80090b_non_iid_evaluate(NULL, 10, 8, true, 0, &r) == SP80090B_EINVAL, "NULL data rejected");
	check(sp80090b_non_iid_evaluate(small, sizeof small, 9, true, 0, &r) == SP80090B_EINVAL, "bits_per_symbol = 9 rejected");
	check(sp80090b_non_iid_evaluate(small, sizeof small, -1, true, 0, &r) == SP80090B_EINVAL, "bits_per_symbol = -1 rejected");
	check(sp80090b_non_iid_evaluate(small, sizeof small, 2, true, 0, &r) == SP80090B_EDATA, "4-bit data declared 2-bit rejected");
	rc = sp80090b_non_iid_evaluate(small, sizeof small, 8, true, 0, &r);
	check(rc == 0 && (r.warnings & SP80090B_WARN_FEW_SAMPLES), "fewer than 1 000 000 samples: warning");
	check(rc == 0 && (r.warnings & SP80090B_WARN_NARROW_DATA), "4-bit data declared 8-bit: warning");
	check(rc == 0 && r.bits_per_symbol == 8 && r.alphabet_size == 16, "bits_per_symbol and alphabet size reported");
	memset(small, 3, sizeof small);
	check(sp80090b_non_iid_evaluate(small, sizeof small, 0, true, 0, &r) == SP80090B_EALPHABET, "single symbol: no entropy");
	check(sp80090b_conditioning_evaluate(false, 512, 256, 256, 400.0L, 1.5L, &c) == SP80090B_EINVAL, "h' > 1 rejected");
	check(sp80090b_restart_evaluate(small, sizeof small, 8, 4.0, false, 0, NULL, 0, NULL) == SP80090B_EINVAL, "restart: NULL result rejected");

	buf = read_file("bin/truerand_4bit.bin", SP80090B_MIN_SAMPLES, &len);
	if (buf) {
		static const uint64_t seed[4] = { 0x0123456789abcdefULL, 1, 2, 3 };

		o.bits_per_symbol = 4;
		o.seed = seed;
		rc = sp80090b_iid_evaluate_ex(buf, len, &o, &r);
		if (rc == 0)
			rc = sp80090b_iid_evaluate_ex(buf, len, &o, &r2);
		check(rc == 0 && memcmp(r.perm_counts, r2.perm_counts, sizeof r.perm_counts) == 0,
		      "IID: same seed, same permutation counters");
		check(rc == 0 && r.passed_iid, "IID: truerand_4bit passes the IID tests (as with ea_iid)");
		free(buf);
	}

	printf("\n%s: %d failure(s)\n", failures ? "FAILURE" : "SUCCESS", failures);
	return failures;
}
