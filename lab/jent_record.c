/* SPDX-License-Identifier: MIT
 * Part of libsp80090b. Copyright (c) 2026 EPI Quantum Crypto team (see NOTICE). */
/*
 * jent_record.c - raw noise of jitterentropy for the SP 800-90B labs
 *
 * Records the CPU execution-jitter noise source of the Jitter RNG
 * (jitterentropy) and writes one sample per byte - the low bits of each time
 * delta - as the NIST tools and libsp80090b expect. It replaces the chain
 * jitterentropy-hashtime + extractlsb of the jitterentropy test suite: same
 * measurement as jitterentropy-hashtime (collector allocated without the
 * startup health test, SP 800-90B health tests on, one priming measurement),
 * without the intermediate text files.
 *
 * Usage:
 *   jent_record [options] <out.bin> [samples]                  test 1 and 2
 *   jent_record -r [options] <out.bin> [restarts [samples]]    test 3
 *
 *   samples     samples per instance (default 1000000, or 1000 with -r)
 *   restarts    fresh collector instances with -r (default 1000); the file
 *               holds instance i at offset samples * i, as lab_restart expects
 *   -b <bits>   bits kept from each delta, 1 to 8 (default 8: mask 0xFF)
 *   -o <osr>    oversampling rate of the collector (default: library value)
 *   -n          no JENT_FORCE_FIPS (default: forced, as --force-fips)
 *   -q          no progress output
 *
 * Build (see the Makefile target lab/jent_record):
 *   gcc -O0 -DJENT_CONF_ENABLE_INTERNAL_TIMER -I$J -I$J/src -I$J/arch -I$J/tests \
 *       lab/jent_record.c -o lab/jent_record -lrt -lpthread
 * with J the jitterentropy-library directory. The noise source must be
 * compiled with -O0, as in the jitterentropy build: an optimizing compiler
 * removes the work whose timing is measured.
 */
#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* The library sources, absorbed as jitterentropy-hashtime does, to reach the
 * noise source below the public API. Atomic accessors first: nearly everything
 * else depends on them. */
#include "jitterentropy-arch-atomic.c"

#include "jitterentropy-sha3.c"
#include "jitterentropy-gcd.c"
#include "jitterentropy-health.c"
#include "jitterentropy-noise.c"
#include "jitterentropy-timer.c"
#include "jitterentropy-base.c"
#include "jitterentropy-uuid.c"
#include "jitterentropy-status.c"

#include "jitterentropy-arch-cache.c"
#include "jitterentropy-arch-fips.c"
#include "jitterentropy-arch-memory.c"
#include "jitterentropy-arch-ncpu.c"
#include "jitterentropy-arch-sched.c"
#include "jitterentropy-arch-thread.c"
#include "jitterentropy-arch-timer.c"
#include "jitterentropy-arch-random.c"

#include "jitterentropy-memlock.h"

static int quiet;

static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [-b <bits>] [-o <osr>] [-n] [-q] <out.bin> [samples]\n"
		"   or: %s -r [-b <bits>] [-o <osr>] [-n] [-q] <out.bin> [restarts [samples]]\n"
		"  -r         restart dataset: a fresh collector per restart (default 1000 x 1000)\n"
		"  -b <bits>  bits kept from each time delta, 1 to 8 (default 8)\n"
		"  -o <osr>   oversampling rate of the collector\n"
		"  -n         do not force the FIPS mode\n"
		"  -q         no progress output\n", prog, prog);
}

static int parse_ulong(const char *str, const char *name, unsigned long min,
		       unsigned long max, unsigned long *val)
{
	char *end;

	errno = 0;
	*val = strtoul(str, &end, 10);
	if (end == str || *end != '\0' || errno || *val < min || *val > max) {
		fprintf(stderr, "Invalid %s \"%s\" (%lu to %lu)\n", name, str, min, max);
		return -1;
	}
	return 0;
}

/*
 * One collector instance: n samples of the noise source into buf. Mirrors
 * jent_one_test() of jitterentropy-hashtime with the default options.
 */
static int record_instance(uint8_t *buf, unsigned long n, unsigned int flags,
			   unsigned int osr, uint8_t mask)
{
	struct rand_data *ec;
	unsigned int failure;
	uint64_t delta;

	/* Cryptographic self tests only: the startup health test could disable
	 * the collector, and a weak source is precisely what we want to see. */
	jent_entropy_init_common_pre(flags);

	/* Internal allocation: keeps the OSR, memory size and loop counts as given. */
	ec = jent_entropy_collector_alloc_internal(osr, flags | JENT_INT_MEASURE_CLOCK);
	if (!ec) {
		fprintf(stderr, "Allocation of the entropy collector failed\n");
		return -1;
	}
	if (ec->enable_notime)
		jent_notime_settick(ec);
	ec->is_fips_enabled = 1;	/* full SP 800-90B health tests */

	jent_measure_jitter(ec, 0, NULL);	/* prime the collector */
	for (unsigned long i = 0; i < n; i++) {
		jent_measure_jitter(ec, 0, &delta);	/* stuck indicator ignored */
		buf[i] = (uint8_t)(delta & mask);
	}

	failure = jent_health_failure(ec);
	if (failure)
		fprintf(stderr, "Health test failure:%s%s%s\n",
			(failure & JENT_RCT_FAILURE) ? " RCT" : "",
			(failure & JENT_APT_FAILURE) ? " APT" : "",
			(failure & JENT_LAG_FAILURE) ? " Lag" : "");

	jent_notime_unsettick(ec);
	jent_entropy_collector_free(ec);
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned long bits = 8, osr = 0, restarts = 1, samples = 1000000;
	unsigned int flags = JENT_FORCE_FIPS;
	int restart = 0, opt, ret = 1;
	const char *path, *prog = argv[0];
	uint8_t *buf = NULL, mask;
	FILE *out = NULL;

	while ((opt = getopt(argc, argv, "rb:o:nqh")) != -1) {
		switch (opt) {
		case 'r':
			restart = 1;
			restarts = 1000;
			samples = 1000;
			break;
		case 'b':
			if (parse_ulong(optarg, "bit count", 1, 8, &bits))
				return 1;
			break;
		case 'o':
			if (parse_ulong(optarg, "OSR", 1, UINT_MAX, &osr))
				return 1;
			break;
		case 'n':
			flags &= ~JENT_FORCE_FIPS;
			break;
		case 'q':
			quiet = 1;
			break;
		default:
			usage(prog);
			return opt == 'h' ? 0 : 1;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc < 1 || argc > (restart ? 3 : 2)) {
		usage(prog);
		return 1;
	}
	path = argv[0];
	if (restart && argc >= 2) {
		if (parse_ulong(argv[1], "restart count", 1, 1000000, &restarts))
			return 1;
		if (argc == 3 && parse_ulong(argv[2], "sample count", 1, 100000000, &samples))
			return 1;
	} else if (!restart && argc == 2) {
		if (parse_ulong(argv[1], "sample count", 1, 1000000000, &samples))
			return 1;
	}
	mask = (uint8_t)((1U << bits) - 1);

	/* Process-wide set-up, once: memory lock limit and secure memory arena. */
	if (jent_raise_memlock_limit(flags))
		fprintf(stderr, "Cannot raise the memory lock limit, allocation may fail\n");
	if (jent_init_secure_memory(flags))
		fprintf(stderr, "Cannot create the secure memory arena, allocation will fail\n");

	buf = malloc(samples);
	out = fopen(path, "wb");
	if (!buf || !out) {
		fprintf(stderr, "%s: %s\n", buf ? path : "malloc", strerror(errno));
		goto end;
	}
	if (!quiet)
		printf("jitterentropy %u.%u.%u, %s mode, %lu x %lu samples, %lu bit(s) each -> %s\n",
		       JENT_MAJVERSION, JENT_MINVERSION, JENT_PATCHLEVEL,
		       (flags & JENT_FORCE_FIPS) ? "FIPS" : "default",
		       restarts, samples, bits, path);

	for (unsigned long r = 0; r < restarts; r++) {
		if (record_instance(buf, samples, flags, (unsigned int)osr, mask))
			goto end;
		/* A short file would be assessed as if it were complete. */
		if (fwrite(buf, 1, samples, out) != samples) {
			fprintf(stderr, "%s: write error\n", path);
			goto end;
		}
		if (!quiet && restart && (r + 1) % 100 == 0)
			printf("  %lu / %lu restarts\n", r + 1, restarts);
	}
	ret = 0;

end:
	free(buf);
	if (out && fclose(out) != 0) {
		fprintf(stderr, "%s: close error\n", path);
		ret = 1;
	}
	return ret;
}
