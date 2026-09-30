/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * test_lib.c - minimal example: both tracks on an in-memory buffer
 *
 * The buffer comes from rand(), a deterministic generator: it only shows
 * how to call the API and says nothing about a real noise source.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sp80090b.h"

int main(void)
{
	const size_t n = SP80090B_MIN_SAMPLES;
	sp80090b_result_t iid, non_iid;
	uint8_t *buffer = (uint8_t *)malloc(n);
	int rc;

	if (!buffer) {
		fprintf(stderr, "Error: allocation failed\n");
		return 1;
	}
	srand(42);
	for (size_t i = 0; i < n; ++i)
		buffer[i] = (uint8_t)(rand() % 256);

	printf("%s\n", sp80090b_version());

	printf("\n--- IID track ---\n");
	rc = sp80090b_iid_evaluate(buffer, n, 8, true, 0, &iid);
	if (rc == SP80090B_OK) {
		printf("  H_original : %.5f bits per symbol\n", iid.h_original);
		printf("  H_bitstring: %.5f bits per bit\n", iid.h_bitstring);
		printf("  H_assessed : %.5f bits per symbol\n", iid.h_assessed);
		printf("  Passed IID : %s\n", iid.passed_iid ? "yes" : "no");
	} else {
		fprintf(stderr, "Error: IID assessment failed with status %d: %s\n", rc, iid.error_msg);
	}

	printf("\n--- Non-IID track ---\n");
	rc = sp80090b_non_iid_evaluate(buffer, n, 8, true, 0, &non_iid);
	if (rc == SP80090B_OK) {
		printf("  H_original : %.5f bits per symbol\n", non_iid.h_original);
		printf("  H_bitstring: %.5f bits per bit\n", non_iid.h_bitstring);
		printf("  H_assessed : %.5f bits per symbol\n", non_iid.h_assessed);
	} else {
		fprintf(stderr, "Error: non-IID assessment failed with status %d: %s\n", rc, non_iid.error_msg);
	}

	free(buffer);
	return 0;
}
