/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * test_conditioning.c - conditioning components (SP 800-90B 3.1.5) through the API
 *
 * Usage: test_conditioning <n_in> <n_out> <nw> <h_in> [h']
 *   without h': vetted function (3.1.5.1.2); with h': non-vetted (3.1.5.2)
 * For the ea_conditioning command line, use lab/lab_conditioning.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sp80090b.h"

int main(int argc, char *argv[])
{
	sp80090b_conditioning_result_t r;
	bool vetted;

	if (argc != 5 && argc != 6) {
		fprintf(stderr, "Usage: %s <n_in> <n_out> <nw> <h_in> [h']\n", argv[0]);
		return 1;
	}
	vetted = (argc == 5);
	if (sp80090b_conditioning_evaluate(vetted, (unsigned)strtoul(argv[1], NULL, 10), (unsigned)strtoul(argv[2], NULL, 10),
	                                   (unsigned)strtoul(argv[3], NULL, 10), strtold(argv[4], NULL),
	                                   vetted ? 0.0L : strtold(argv[5], NULL), &r) != SP80090B_OK) {
		fprintf(stderr, "Error: %s\n", r.error_msg);
		return 1;
	}
	printf("%s conditioning component\n", vetted ? "Vetted" : "Non-vetted");
	printf("Output_Entropy = %.22Lg\n", r.output_entropy);
	printf("h_out = %.22Lg\n", r.h_out);
	printf("epsilon = 2^(%.6Lf)%s\n", r.epsilon_log2,
	       r.full_entropy_candidate ? " (full entropy candidate, FIPS 140-3 IG D.K)" : "");
	return 0;
}
