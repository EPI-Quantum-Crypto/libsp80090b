/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * test_iid.c - IID assessment (SP 800-90B Section 5 and 6.3.1) through the API
 *
 * Usage: test_iid [-v] [-v] [file_name] [bits_per_symbol]
 *   file_name defaults to ../bin/truerand_8bit.bin; bits_per_symbol to 0
 *   (inferred from the data). -v and -vv print the details of the NIST
 *   ea_iid verbose modes. For the full ea_iid command line, use lab/lab_iid.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "sp80090b.h"

int main(int argc, char *argv[])
{
	const char *filename = "../bin/truerand_8bit.bin";
	int bits = 0, verbose = 1, opt;
	sp80090b_options_t o = sp80090b_default_options();
	sp80090b_result_t res;
	uint8_t *buffer;
	long filesize;
	size_t len;
	int status;
	FILE *f;

	while ((opt = getopt(argc, argv, "v")) != -1) {
		if (opt != 'v') {
			fprintf(stderr, "Usage: %s [-v] [-v] [file_name] [bits_per_symbol]\n", argv[0]);
			return 1;
		}
		verbose++;
	}
	if (optind < argc)
		filename = argv[optind];
	if (optind + 1 < argc)
		bits = atoi(argv[optind + 1]);

	if (!(f = fopen(filename, "rb"))) {
		fprintf(stderr, "Error: cannot open file '%s'\n", filename);
		return 1;
	}
	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = (uint8_t *)malloc(filesize > 0 ? (size_t)filesize : 1);
	if (!buffer) {
		fclose(f);
		fprintf(stderr, "Error: failed to allocate %ld bytes\n", filesize);
		return 1;
	}
	len = fread(buffer, 1, (size_t)filesize, f);
	fclose(f);

	printf("%s\n", sp80090b_version());
	printf("Loaded %zu samples from '%s'\n", len, filename);

	o.bits_per_symbol = bits;
	o.verbose = verbose > 1 ? verbose : 0;	/* as lab_iid: 2 = -v, 3 = -vv */
	status = sp80090b_iid_evaluate_ex(buffer, len, &o, &res);
	free(buffer);
	if (status != SP80090B_OK) {
		fprintf(stderr, "Error %d: %s\n", status, res.error_msg);
		return 1;
	}
	if (res.warnings)
		printf("Warning: %s\n", res.error_msg);

	printf("Chi-square tests (5.2.1-5.2.4): %s (smallest p-value %.6f)\n", res.passed_chi_square ? "passed" : "FAILED", res.p_value);
	printf("LRS test (5.2.5): %s\n", res.passed_lrs ? "passed" : "FAILED");
	printf("Permutation tests (5.1): %s\n", res.passed_permutation ? "passed" : "FAILED");
	printf("H_original: %.6f bits per symbol\n", res.h_original);
	if (res.alphabet_size > 2)
		printf("H_bitstring: %.6f bits per bit\n", res.h_bitstring);
	if (res.passed_iid)
		printf("IID assumption holds: H_I = %.6f bits per symbol\n", res.h_assessed);
	else
		printf("IID assumption rejected: use the non-IID track (test_non_iid)\n");
	return res.passed_iid ? 0 : 2;
}
