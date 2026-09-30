/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
/*
 * libsp80090b - NIST SP 800-90B entropy assessment on in-memory buffers.
 *
 * C port of the NIST reference tool SP800-90B_EntropyAssessment 1.1.8
 * (ea_iid, ea_non_iid, ea_restart, ea_conditioning). Section numbers refer
 * to NIST SP 800-90B (January 2018).
 *
 * Data layout: one sample per byte; the bits_per_symbol least significant
 * bits form the symbol.
 */
#ifndef SP800_90B_H
#define SP800_90B_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SP80090B_LIB_VERSION   "2.0.0"
#define SP80090B_NIST_VERSION  "1.1.8"   /* reference tool reproduced */

#define SP80090B_MIN_SAMPLES   1000000   /* 3.1.1: at least 1 000 000 samples */
#define SP80090B_RESTART_ROWS  1000      /* 3.1.4.1: r = 1000 restarts */
#define SP80090B_RESTART_COLS  1000      /* 3.1.4.1: c = 1000 samples each */
#define SP80090B_DEFAULT_SIMULATION_ROUNDS 5000000UL
#define SP80090B_PERM_TESTS    19        /* 5.1.1 to 5.1.11 */

/* Return codes */
#define SP80090B_OK             0
#define SP80090B_EINVAL        -1   /* invalid argument */
#define SP80090B_EDATA         -2   /* data wider than bits_per_symbol, or out of memory */
#define SP80090B_EALPHABET     -3   /* a single symbol: no entropy */
#define SP80090B_EINTERNAL     -4   /* allocation, seeding or numerical failure */

/* Warning flags (result.warnings) */
#define SP80090B_WARN_FEW_SAMPLES  0x1u  /* len < 1 000 000 (3.1.1) */
#define SP80090B_WARN_NARROW_DATA  0x2u  /* symbols narrower than bits_per_symbol */
#define SP80090B_WARN_TRUNCATED    0x4u  /* H_bitstring on the first 1 000 000 bits only (-t) */

/*
 * Options of an IID or non-IID assessment (ea_iid / ea_non_iid options).
 * Initialise with sp80090b_default_options().
 */
typedef struct {
    int bits_per_symbol;       /* 1..8, or 0 to infer from the data */
    bool initial_entropy;      /* true: -i, H_I (3.1.3); false: -c, h' of conditioned data (3.1.5.2) */
    bool truncate_bitstring;   /* true: -t, H_bitstring on the first 1 000 000 bits */
    int verbose;               /* 0 silent; 1..3 as the NIST tool (-v, -vv) on stdout */
    const uint64_t *seed;      /* IID only: 4 words for reproducible permutations; NULL = /dev/urandom */
} sp80090b_options_t;

typedef struct {
    double h_original;    /* H_original, bits per symbol (literal estimators) */
    double h_bitstring;   /* H_bitstring, bits per bit; h' when initial_entropy is false */
    double h_assessed;    /* min(H_original, bits_per_symbol x H_bitstring); h' with -c */
    bool passed_iid;      /* IID track: chi-square AND LRS AND permutation tests passed */
    double p_value;       /* IID track: smaller chi-square p-value (5.2); 0 for non-IID */
    int status;           /* return code of the call */
    char error_msg[256];  /* error text, or last warning text when status == 0 */
    /* since 2.0.0 */
    unsigned int warnings;        /* SP80090B_WARN_* flags */
    long n_samples;               /* samples assessed */
    int bits_per_symbol;          /* width used (inferred when 0 was given) */
    int alphabet_size;            /* distinct symbols observed (k) */
    bool passed_chi_square;       /* IID: 5.2.1-5.2.4 */
    bool passed_lrs;              /* IID: 5.2.5 */
    bool passed_permutation;      /* IID: 5.1 */
    int perm_counts[SP80090B_PERM_TESTS][3]; /* IID: C[i][0] (>), C[i][1] (=), C[i][2] (<) */
} sp80090b_result_t;

/* Restart test result (3.1.4) */
typedef struct {
    double h_i;           /* H_I given by the caller */
    double h_r;           /* entropy of the row dataset (NaN if the sanity check failed) */
    double h_c;           /* entropy of the column dataset (NaN if the sanity check failed) */
    double alpha;         /* 1 - 0.99^(1/2000) */
    int x_cutoff;         /* simulated cutoff of the sanity check (3.1.4.3) */
    int x_max;            /* max(X_r, X_c): largest row or column count of one value */
    int x_r, x_c;
    bool sanity_passed;   /* 3.1.4.3 */
    bool validation_passed; /* 3.1.4.2: min(H_r, H_c) >= H_I / 2 */
    bool passed;          /* sanity_passed && validation_passed */
    double h_restart;     /* min(H_r, H_c, H_I) when passed, else 0 */
    bool iid;             /* IID mode: MCV estimates + IID tests on rows and columns */
    bool passed_iid_rows, passed_iid_cols; /* IID mode only (informative, as ea_restart -i) */
    int bits_per_symbol;
    int alphabet_size;
    int status;
    char error_msg[256];
} sp80090b_restart_result_t;

/* Conditioning component result (3.1.5) */
typedef struct {
    long double output_entropy; /* Output_Entropy(n_in, n_out, nw, h_in), 3.1.5.1.2 */
    long double h_out;          /* vetted: Output_Entropy; non-vetted: min(Output_Entropy, 0.999 n_out, h' n_out) */
    long double epsilon_log2;   /* log2(epsilon) with Output_Entropy = (1 - epsilon) n_out */
    long double epsilon_nw_log2;/* log2(epsilon) relative to nw when n = nw */
    unsigned int nw;            /* nw used, min(nw, n_in) (Appendix E) */
    bool full_entropy_candidate;/* vetted and h_out > 0.999 n_out and h_in >= n_out + 64 (IG D.K) */
    int status;
    char error_msg[256];
} sp80090b_conditioning_result_t;

/* Library version string, e.g. "2.0.0 (NIST SP800-90B_EntropyAssessment 1.1.8)" */
const char *sp80090b_version(void);

/* Defaults: bits 0 (inferred), initial entropy, all bits, silent, random seed. */
sp80090b_options_t sp80090b_default_options(void);

/*
 * IID track (ea_iid): 6.3.1 most common value estimate, then the IID tests
 * of Section 5 (5.1 permutation tests, 5.2 chi-square and LRS tests).
 * h_assessed is meaningful ONLY if passed_iid is true; otherwise use the
 * non-IID track.
 */
int sp80090b_iid_evaluate_ex(const uint8_t *data, size_t len, const sp80090b_options_t *opt, sp80090b_result_t *result);

/* Non-IID track (ea_non_iid): the ten estimators of Section 6.3. */
int sp80090b_non_iid_evaluate_ex(const uint8_t *data, size_t len, const sp80090b_options_t *opt, sp80090b_result_t *result);

/* 1.x compatible entry points (random seed, all bits). */
int sp80090b_iid_evaluate(const uint8_t *data, size_t len, int bits_per_symbol, bool initial_entropy, int verbose, sp80090b_result_t *result);
int sp80090b_non_iid_evaluate(const uint8_t *data, size_t len, int bits_per_symbol, bool initial_entropy, int verbose, sp80090b_result_t *result);

/*
 * Restart tests (ea_restart), Section 3.1.4. data is the 1000 x 1000 restart
 * matrix in row order: sample j of restart i at data[1000 * i + j].
 * h_i is the initial entropy estimate H_I (3.1.3). iid selects the IID
 * track (as ea_restart -i). rounds is the number of simulation rounds of
 * the sanity check (0 = 5 000 000, the NIST default; lower values are
 * refused). seed: NULL = /dev/urandom.
 * Returns SP80090B_OK when the test could be run; read result->passed.
 */
int sp80090b_restart_evaluate(const uint8_t *data, size_t len, int bits_per_symbol, double h_i, bool iid,
                              unsigned long rounds, const uint64_t *seed, int verbose,
                              sp80090b_restart_result_t *result);

/*
 * Conditioning components (ea_conditioning), Section 3.1.5.
 * vetted: listed in 3.1.5.1.1 (h_prime ignored), else non-vetted (3.1.5.2)
 * and h_prime in ]0, 1] is the entropy per bit of the conditioned
 * sequential dataset (sp80090b_non_iid_evaluate_ex with initial_entropy
 * false, field h_bitstring).
 */
int sp80090b_conditioning_evaluate(bool vetted, unsigned int n_in, unsigned int n_out, unsigned int nw,
                                   long double h_in, long double h_prime, sp80090b_conditioning_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* SP800_90B_H */
