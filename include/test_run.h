/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef TEST_RUN_H
#define TEST_RUN_H

#include <stdbool.h>

typedef struct {
    int errorLevel;
    char errorMsg[256];
} test_run_t;

#endif
