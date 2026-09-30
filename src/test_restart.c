/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * test_restart.c - restart test (SP 800-90B 3.1.4) through the library API
 *
 * Usage: test_restart <restart_file> <H_I> [bits_per_symbol] [-i] [-v] [-v]
 *   restart_file: 1000 restarts x 1000 samples, restart i at offset 1000 * i
 *   H_I: initial entropy estimate from test_non_iid / test_iid
 *   -v, -vv: the details of the NIST ea_restart verbose modes
 * For the ea_restart command line, use lab/lab_restart.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "sp80090b.h"

int main(int argc, char *argv[])
{
	sp80090b_restart_result_t r;
	const size_t n = (size_t)SP80090B_RESTART_ROWS * SP80090B_RESTART_COLS;
	bool iid = false;
	int bits = 0, verbose = 1, rc;
	double h_i;
	uint8_t *buf;
	size_t got;
	FILE *f;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <restart_file> <H_I> [bits_per_symbol] [-i] [-v] [-v]\n", argv[0]);
		return 1;
	}
	h_i = atof(argv[2]);
	for (int i = 3; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0) iid = true;
		else if (strcmp(argv[i], "-v") == 0) verbose++;
		else if (strcmp(argv[i], "-vv") == 0) verbose += 2;
		else bits = atoi(argv[i]);
	}

	if (!(f = fopen(argv[1], "rb"))) {
		fprintf(stderr, "Error: cannot open '%s'\n", argv[1]);
		return 1;
	}
	buf = (uint8_t *)malloc(n + 1);
	got = buf ? fread(buf, 1, n + 1, f) : 0;
	fclose(f);
	if (!buf)
		return 1;

	printf("%s\n", sp80090b_version());
	rc = sp80090b_restart_evaluate(buf, got, bits, h_i, iid, 0, NULL, verbose, &r);
	free(buf);
	if (rc != SP80090B_OK) {
		fprintf(stderr, "Error %d: %s\n", rc, r.error_msg);
		return 1;
	}
	printf("Sanity check (3.1.4.3): %s (X_max %d, X_cutoff %d)\n", r.sanity_passed ? "passed" : "FAILED", r.x_max, r.x_cutoff);
	if (r.sanity_passed)
		printf("Validation (3.1.4.2): %s (H_r %.6f, H_c %.6f, H_I %.6f)\n",
		       r.validation_passed ? "passed" : "FAILED", r.h_r, r.h_c, r.h_i);
	if (r.passed)
		printf("Entropy after the restart test: min(H_r, H_c, H_I) = %.6f\n", r.h_restart);
	return r.passed ? 0 : 2;
}
