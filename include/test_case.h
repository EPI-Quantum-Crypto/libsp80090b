/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef TEST_CASE_H
#define TEST_CASE_H

#include <stdbool.h>

typedef struct {
    double mean;
    double median;
    bool binary;
    double h_original;
    double h_bitstring;
    double h_assessed;
    long mcv_estimate_mode;
    double mcv_estimate_p_hat;
    double mcv_estimate_p_u;
    int literal_mcv_estimate;
} test_case_t;

#endif
