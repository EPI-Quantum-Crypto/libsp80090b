/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include "sp80090b.h"

/**
 * Non-IID assessment (SP 800-90B 6.3) of a file through the library API,
 * as ./ea_non_iid [-i|-c] [-v] [-v] <file_name> [bits_per_symbol].
 * Used by selftest/selftest to compare with the NIST reference outputs.
 *
 * -i: initial entropy estimate H_I (3.1.3), the default
 * -c: conditioned sequential dataset, h' = H_bitstring (3.1.5.2)
 *
 * Verbosity follows the NIST tool:
 * 1 = normal, 2 = NIST verbose mode (-v), 3 = extra verbose (-vv).
 */
int main(int argc, char* argv[]) {
    int verbose = 1;
    bool initial_entropy = true;
    int opt;

    while ((opt = getopt(argc, argv, "icv")) != -1) {
        switch (opt) {
            case 'i':
                initial_entropy = true;
                break;
            case 'c':
                initial_entropy = false;
                break;
            case 'v':
                verbose++;
                break;
            default:
                fprintf(stderr, "Usage: %s [-i|-c] [-v] [-v] <file_name> [bits_per_symbol]\n", argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Usage: %s [-i|-c] [-v] [-v] <file_name> [bits_per_symbol]\n", argv[0]);
        return 1;
    }

    const char* filename = argv[optind];
    int bits_per_symbol = 0; // 0 = inferred from the data, as ea_non_iid
    if (optind + 1 < argc) {
        bits_per_symbol = atoi(argv[optind + 1]);
    }

    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = (uint8_t*)malloc((size_t)filesize);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "Error: failed to allocate %ld bytes\n", filesize);
        return 1;
    }
    size_t read_bytes = fread(buffer, 1, (size_t)filesize, f);
    fclose(f);

    if (verbose > 1) {
        printf("Opening file: '%s'\n", filename);
    }

    sp80090b_result_t res;
    int status = sp80090b_non_iid_evaluate(buffer, read_bytes, bits_per_symbol, initial_entropy, verbose, &res);
    free(buffer);

    if (status != 0) {
        fprintf(stderr, "Error %d: %s\n", status, res.error_msg);
        return status;
    }

    printf("H_bitstring = %.17g\n", res.h_bitstring);
    printf("H_original = %.17g\n", res.h_original);
    printf("Assessed min entropy: %.17g\n", res.h_assessed);

    return 0;
}
