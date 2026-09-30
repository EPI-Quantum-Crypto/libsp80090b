/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef UTILS_H
#define UTILS_H

#define VERSION "1.1.8"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <float.h>
#include <omp.h>

#include "test_run.h"
#include "test_case.h"

#define SWAP(x, y) do { int s = x; x = y; y = s; } while(0)
#define INOPENINTERVAL(x, a, b) (((a)>(b))?(((x)>(b))&&((x)<(a))):(((x)>(a))&&((x)<(b))))
#define INCLOSEDINTERVAL(x, a, b) (((a)>(b))?(((x)>=(b))&&((x)<=(a))):(((x)>=(a))&&((x)<=(b))))

#define MIN_SIZE 1000000
#define PERMS 10000

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define RELEPSILON DBL_EPSILON
#define ABSEPSILON DBL_MIN
#define DBL_INFINITY __builtin_inf ()
#define ITERMAX 1076
#define ZALPHA 2.5758293035489008
#define ZALPHA_L 2.575829303548900384158L

#define BINARYDICTLOC(d, b) (binaryDict[(d)-1] + (((b) & ((1U << (d)) - 1))<<1))

#ifdef __SIZEOF_INT128__
typedef unsigned __int128 uint128_t;
typedef unsigned __int128 uint_least128_t;
# define UINT128_MAX         ((uint128_t)-1)
# define UINT128_WIDTH       128
# define UINT_LEAST128_WIDTH 128
# define UINT_LEAST128_MAX   UINT128_MAX
# define UINT128_C(N)        ((uint_least128_t)+N ## WBU)
#endif

typedef struct data_t {
	int word_size; 		// bits per symbol
	int alph_size; 		// symbol alphabet size
	uint8_t maxsymbol; 	// the largest symbol present in the raw data stream
	uint8_t *rawsymbols; 	// raw data words
	uint8_t *symbols; 		// data words
	uint8_t *bsymbols; 	// data words as binary string
	long len; 		// number of words in data
	long blen; 		// number of bits in data
} data_t;

static inline bool relEpsilonEqual(double A, double B, double maxAbsFactor, double maxRelFactor, uint32_t maxULP) {
   double diff;
   double absA, absB;

   assert(sizeof(uint64_t) == sizeof(double));
   assert(maxAbsFactor >= 0.0);
   assert(maxRelFactor >= 0.0);

   if(isnan(A) || isnan(B)) return false;
   if(A == B) return true;
   if(isinf(A) || isinf(B)) return false;

   absA = fabs(A);
   absB = fabs(B);
   if(absA > absB) {
      double tmp = B; B = A; A = tmp;
      tmp = absB; absB = absA; absA = tmp;
   }

   diff = fabs(B - A);

   if((absA < DBL_MIN) || (diff < DBL_MIN) || isinf(diff) || (absB * maxRelFactor < DBL_MIN)) {
      return diff <= maxAbsFactor;
   } else {
      if(diff <= absB * maxRelFactor) return true;
   }
   return false;
}

static inline void free_data(data_t *dp){
	if(dp->symbols != NULL) free(dp->symbols);
	if(dp->rawsymbols != NULL) free(dp->rawsymbols);
	if((dp->word_size > 1) && (dp->bsymbols != NULL)) free(dp->bsymbols);
}

static inline bool read_buffer(const uint8_t *buffer, size_t buf_len, data_t *dp, test_run_t *testRun) {
	long i;
	int j, mask, max_symbols;

	if (!buffer || buf_len == 0) {
		if (testRun) {
			testRun->errorLevel = -1;
			snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Error: buffer is empty or NULL");
		}
		return false;
	}

	dp->len = (long)buf_len;

	dp->symbols = (uint8_t*)malloc(sizeof(uint8_t)*dp->len);
	dp->rawsymbols = (uint8_t*)malloc(sizeof(uint8_t)*dp->len);
	if ((dp->symbols == NULL) || (dp->rawsymbols == NULL)) {
		if (testRun) {
			testRun->errorLevel = -1;
			snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Error: failure to initialize memory for symbols");
		}
		if (dp->symbols != NULL) free(dp->symbols);
		if (dp->rawsymbols != NULL) free(dp->rawsymbols);
		dp->symbols = NULL;
		dp->rawsymbols = NULL;
		return false;
	}

	memcpy(dp->symbols, buffer, sizeof(uint8_t) * dp->len);

	if (dp->word_size == 0) {
		uint8_t datamask = 0;
		uint8_t curbit = 0x80;

		for (i = 0; i < dp->len; i++) {
			datamask = datamask | dp->symbols[i];
		}

		for (i = 8; (i > 0) && ((datamask & curbit) == 0); i--) {
			curbit = curbit >> 1;
		}

		dp->word_size = i;
	} else {
		uint8_t datamask = 0;
		uint8_t curbit = 0x80;

		for (i = 0; i < dp->len; i++) {
			datamask = datamask | dp->symbols[i];
		}

		for (i = 8; (i > 0) && ((datamask & curbit) == 0); i--) {
			curbit = curbit >> 1;
		}

		if (i < dp->word_size) {
			if (testRun) snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Warning: Symbols appear to be narrower than described.");
		} else if (i > dp->word_size) {
			if (testRun) {
				testRun->errorLevel = -1;
				snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Error: Incorrect bit width specification.");
			}
			free(dp->symbols);
			dp->symbols = NULL;
			free(dp->rawsymbols);
			dp->rawsymbols = NULL;
			return false;
		}
	}

	memcpy(dp->rawsymbols, dp->symbols, sizeof(uint8_t) * dp->len);
	dp->maxsymbol = 0;

	max_symbols = 1 << dp->word_size;
	int *symbol_map_down_table = (int*)malloc(max_symbols * sizeof(int));
	if (!symbol_map_down_table) {
		if (testRun) {
			testRun->errorLevel = -1;
			snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Error: failure to initialize memory for symbol map");
		}
		free(dp->symbols);
		dp->symbols = NULL;
		free(dp->rawsymbols);
		dp->rawsymbols = NULL;
		return false;
	}

	dp->alph_size = 0;
	memset(symbol_map_down_table, 0, max_symbols * sizeof(int));
	mask = max_symbols - 1;
	for (i = 0; i < dp->len; i++) {
		dp->symbols[i] &= mask;
		if (dp->symbols[i] > dp->maxsymbol) dp->maxsymbol = dp->symbols[i];
		if (symbol_map_down_table[dp->symbols[i]] == 0) symbol_map_down_table[dp->symbols[i]] = 1;
	}

	for (i = 0; i < max_symbols; i++) {
		if (symbol_map_down_table[i] != 0) symbol_map_down_table[i] = (uint8_t)dp->alph_size++;
	}

	dp->blen = dp->len * dp->word_size;
	if (dp->word_size == 1) dp->bsymbols = dp->symbols;
	else {
		dp->bsymbols = (uint8_t*)malloc(dp->blen);
		if (dp->bsymbols == NULL) {
			if (testRun) {
				testRun->errorLevel = -1;
				snprintf(testRun->errorMsg, sizeof(testRun->errorMsg), "Error: failure to initialize memory for bitstring");
			}
			free(symbol_map_down_table);
			free(dp->symbols);
			dp->symbols = NULL;
			free(dp->rawsymbols);
			dp->rawsymbols = NULL;
			return false;
		}

		for (i = 0; i < dp->len; i++) {
			for (j = 0; j < dp->word_size; j++) {
				dp->bsymbols[i*dp->word_size+j] = (dp->symbols[i] >> (dp->word_size-1-j)) & 0x1;
			}
		}
	}

	if (dp->alph_size < dp->maxsymbol + 1) {
		for (i = 0; i < dp->len; i++) dp->symbols[i] = (uint8_t)symbol_map_down_table[dp->symbols[i]];
	}

	free(symbol_map_down_table);
	return true;
}

/* Baseline statistics of the IID track (NIST calc_stats()):
 * - the mean is taken over the raw sample values (5.1.1, 5.1.10);
 * - the median is the true median of the translated symbols, or 0.5 for
 *   binary data (5.1.5, 5.1.6). A counting sort replaces std::sort. */
static inline void calc_stats(const data_t *dp, double *rawmean, double *median) {
	long i, counts[256] = {0}, half, seen;
	long sum = 0;
	int v, lo = -1, hi = -1;

	for (i = 0; i < dp->len; i++) {
		sum += dp->rawsymbols[i];
		counts[dp->symbols[i]]++;
	}
	if (rawmean) *rawmean = sum / (double)dp->len;
	if (!median) return;

	if (dp->alph_size == 2) {
		*median = 0.5;
		return;
	}

	/* v[half] and v[half - 1] of the sorted sequence */
	half = dp->len / 2;
	seen = 0;
	for (v = 0; v < 256; v++) {
		if ((lo < 0) && (half - 1 >= 0) && (half - 1 < seen + counts[v])) lo = v;
		if ((hi < 0) && (half < seen + counts[v])) hi = v;
		seen += counts[v];
	}
	if ((dp->len & 1) == 1) *median = hi;
	else *median = (hi + lo) / 2.0;
}

/* xoshiro256** 1.0 (Blackman, Vigna, public domain), unbiased integer
 * range and uniform double: same generator as the NIST tool
 * (shared/utils.h), used for the permutations and the restart sanity-check
 * simulation. */
static inline uint64_t xoshiro_rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static inline uint64_t xoshiro256starstar(uint64_t *s) {
	const uint64_t result = xoshiro_rotl(s[1] * 5, 7) * 9;
	const uint64_t t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = xoshiro_rotl(s[3], 45);
	return result;
}

/* Seed from the caller when given (reproducible runs), else from
 * /dev/urandom as the NIST tool does. Returns false on failure. */
static inline bool xoshiro_seed(uint64_t *s, const uint64_t *user_seed) {
	FILE *f;
	bool ok;

	if (user_seed) {
		memcpy(s, user_seed, 4 * sizeof(uint64_t));
		return (s[0] | s[1] | s[2] | s[3]) != 0;
	}
	if ((f = fopen("/dev/urandom", "rb")) == NULL) return false;
	ok = (fread(s, sizeof(uint64_t), 4, f) == 4);
	fclose(f);
	return ok && ((s[0] | s[1] | s[2] | s[3]) != 0);
}

/* SplitMix64 (Steele, Lea, Flood): derives the xoshiro256** state of
 * stream `index` from the main seed, so that permutation i (or simulation
 * block i) always uses the same random numbers whatever the thread that
 * computes it. Streams are statistically independent. */
static inline uint64_t splitmix64(uint64_t *x) {
	uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline void xoshiro_stream(const uint64_t *main_seed, uint64_t index, uint64_t *s) {
	uint64_t x = index;

	for (int i = 0; i < 4; i++) s[i] = main_seed[i] ^ splitmix64(&x);
	if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
}

/* Integer in [0, s] without modular bias (Lemire 2018), as randomRange64(). */
static inline uint64_t randomRange64(uint64_t s, uint64_t *state) {
	uint64_t x = xoshiro256starstar(state);
	uint128_t m;
	uint64_t l;

	if (s == UINT64_MAX) return x;
	s++;
	m = (uint128_t)x * (uint128_t)s;
	l = (uint64_t)m;
	if (l < s) {
		uint64_t t = ((uint64_t)(-s)) % s;
		while (l < t) {
			x = xoshiro256starstar(state);
			m = (uint128_t)x * (uint128_t)s;
			l = (uint64_t)m;
		}
	}
	return (uint64_t)(m >> 64U);
}

/* Uniform double in [0, 1), as randomUnit(). */
static inline double randomUnit(uint64_t *state) {
	return (xoshiro256starstar(state) >> 11) * 1.1102230246251565e-16;
}

/* Fisher-Yates shuffle (SP 800-90B Figure 5) applied to the translated
 * and the raw symbols with the same permutation, as FYshuffle(). */
static inline void FYshuffle(uint8_t data[], uint8_t rawdata[], const long sample_size, uint64_t *state) {
	for (long i = sample_size - 1; i > 0; --i) {
		long r = (long)randomRange64((uint64_t)i, state);
		uint8_t t = data[r]; data[r] = data[i]; data[i] = t;
		t = rawdata[r]; rawdata[r] = rawdata[i]; rawdata[i] = t;
	}
}

static inline double prediction_estimate_function(long double p, long r, long N){
	long double q, x, xlast=0.0L;
	assert(p > 0.0L);
	assert(p < 1.0L);

	q = 1.0L-p;
	x = 1.0L;

	for(int i = 0; (i <= 65) && ((x - xlast) > (LDBL_EPSILON*x)); i++) {
		xlast = x;
		x = 1.0L + q*powl(p, r)*powl(x, r+1.0L);
		assert(x >= xlast);
		assert(p*x <= 1.0L);
	}

	return((double)(logl(1.0L-p*x) - logl((r+1.0L-r*x)*q) - (N+1.0L)*logl(x)));
}

static inline double calc_p_local(long max_run_len, long N, double ldomain){
	int j;
	double p, log_alpha;
	double lastP, pVal;
	double lvalue, hvalue;
	double hbound, lbound;
	double hdomain;

	log_alpha = log(0.99);
	hdomain = 1.0;

	lbound = ldomain;
	hbound = hdomain;

	lvalue = DBL_INFINITY;
	hvalue = -DBL_INFINITY;

	p = (lbound + hbound) / 2.0;
	pVal = prediction_estimate_function(p, max_run_len+1, N);

	for(j=0; j<ITERMAX; j++) {
		if(relEpsilonEqual(pVal, log_alpha, ABSEPSILON, RELEPSILON, 4)) break;

		if(log_alpha < pVal) {
			lbound = p;
			lvalue = pVal;
		} else {
			hbound = p;
			hvalue = pVal;
		}

		if(lbound >= hbound) {
			p = fmin(fmax(lbound, hbound),hdomain);
			break;
		}

		if(!(INCLOSEDINTERVAL(lbound, ldomain, hdomain) && INCLOSEDINTERVAL(hbound,  ldomain, hdomain))) {
			p = hdomain;
			break;
		}

		if(!INCLOSEDINTERVAL(log_alpha, lvalue, hvalue)) {
			p = hdomain;
			break;
		}

		lastP = p;
		p = (lbound + hbound) / 2.0;

		if(!INOPENINTERVAL(p,  lbound, hbound)) {
			p = hbound;
			break;
		}

		if(lastP == p) {
			p = hbound;
			break;
		}

		pVal = prediction_estimate_function(p, max_run_len+1, N);

		if(!INCLOSEDINTERVAL(pVal, lvalue, hvalue)) {
			p = hbound;
			break;
		}
	}

	return p;
}

static inline double predictionEstimate(long C, long N, long max_run_len, long k, const char *testname, const int verbose, const char *label) {
	double curMax;
	double p_local=-1.0;
	double entEst;
	double p_global;
	double p_globalPrime;

	curMax = 1.0 / ((double)k);
	p_global = (double)C/(double)N;

	if(p_global > 0) p_globalPrime = MIN(1.0, p_global + ZALPHA*sqrt((p_global*(1.0-p_global))/((double)N-1.0)));
	else p_globalPrime = 1.0 - pow(0.01, 1.0/(double)N);

	curMax = fmax(curMax, p_globalPrime);
	if((curMax < 1.0) && (prediction_estimate_function(curMax, max_run_len+1, N) > log(0.99))) {
		p_local = calc_p_local(max_run_len, N, curMax);
		curMax = fmax(curMax, p_local);
	}

	entEst = -log2(curMax);

	if(verbose == 2) {
		if(p_local > 0.0) printf("%s %s Prediction Estimate: N = %ld, Pglobal' = %.17g (C = %ld) Plocal = %.17g (r = %ld)\n", label, testname, N, p_globalPrime, C, p_local, max_run_len+1);
		else printf("%s %s Prediction Estimate: N = %ld, Pglobal' = %.17g (C = %ld) Plocal can't affect result (r = %ld)\n", label, testname, N, p_globalPrime, C, max_run_len+1);
	} else if(verbose == 3) {
		printf("%s %s Prediction Estimate: C = %ld\n", label, testname, C);
		printf("%s %s Prediction Estimate: r = %ld\n", label, testname, max_run_len + 1);
		printf("%s %s Prediction Estimate: N = %ld\n", label, testname, N);
		printf("%s %s Prediction Estimate: P_global = %.17g\n", label, testname, p_global);
		printf("%s %s Prediction Estimate: P_global' = %.17g\n", label, testname, p_globalPrime);

		if(p_local > 0.0) printf("%s %s Prediction Estimate: P_local = %.17g\n", label, testname, p_local);
		else printf("%s %s Prediction Estimate: P_local can't change the result.\n", label, testname);

		printf("%s %s Prediction Estimate: min entropy = %.17g\n", label, testname, entEst);
	}
	return entEst;
}

/* Pure-C equivalent of the reference implementation's
 * `map<array<uint8_t,N>, PostfixDictionary>` used by multi_mmc_test and
 * LZ78Y_test (non-binary alphabets): an incremental prefix -> per-symbol
 * postfix-count dictionary, hash-chained so lookups/inserts stay O(1)
 * amortized instead of the O(n) full-history rescans a naive port would need. */
#define PFDICT_KEYLEN 16
#define PFMAP_BUCKETS 131072

typedef struct {
	uint8_t *syms;
	long *counts;
	int n;
	int cap;
	long curBest;
	uint8_t curPrediction;
} postfix_dict_t;

static inline void pfd_init(postfix_dict_t *pd) {
	pd->syms = NULL;
	pd->counts = NULL;
	pd->n = 0;
	pd->cap = 0;
	pd->curBest = 0;
	pd->curPrediction = 0;
}

static inline void pfd_free(postfix_dict_t *pd) {
	free(pd->syms);
	free(pd->counts);
}

/* Mirrors PostfixDictionary::predict(): assumes curBest > 0 (only called on
 * entries that have had at least one increment). */
static inline uint8_t pfd_predict(const postfix_dict_t *pd, long *count) {
	*count = pd->curBest;
	return pd->curPrediction;
}

/* Mirrors PostfixDictionary::incrementPostfix(): returns true iff a brand
 * new (prefix,symbol) edge was created (used to track the MAX_ENTRIES /
 * MAX_DICTIONARY_SIZE caps). Ties prefer the larger symbol value. */
static inline bool pfd_increment(postfix_dict_t *pd, uint8_t sym, bool makeNew) {
	long c;
	for (int i = 0; i < pd->n; i++) {
		if (pd->syms[i] == sym) {
			c = ++pd->counts[i];
			if ((c > pd->curBest) || ((c == pd->curBest) && (sym > pd->curPrediction))) {
				pd->curBest = c;
				pd->curPrediction = sym;
			}
			return false;
		}
	}
	if (!makeNew) return false;

	if (pd->n == pd->cap) {
		pd->cap = pd->cap ? pd->cap * 2 : 4;
		pd->syms = (uint8_t*)realloc(pd->syms, pd->cap * sizeof(uint8_t));
		pd->counts = (long*)realloc(pd->counts, pd->cap * sizeof(long));
	}
	pd->syms[pd->n] = sym;
	pd->counts[pd->n] = 1;
	pd->n++;

	c = 1;
	if ((c > pd->curBest) || ((c == pd->curBest) && (sym > pd->curPrediction))) {
		pd->curBest = c;
		pd->curPrediction = sym;
	}
	return true;
}

typedef struct pfmap_node {
	uint8_t key[PFDICT_KEYLEN];
	postfix_dict_t pd;
	struct pfmap_node *next;
} pfmap_node_t;

typedef struct {
	pfmap_node_t **buckets;
	size_t nbuckets;
} pfmap_t;

static inline size_t pfmap_hash(const uint8_t *key, size_t nbuckets) {
	uint64_t h = 1469598103934665603ULL; /* FNV-1a offset basis */
	for (int i = 0; i < PFDICT_KEYLEN; i++) {
		h ^= key[i];
		h *= 1099511628211ULL; /* FNV-1a prime */
	}
	return (size_t)(h % nbuckets);
}

static inline pfmap_t* pfmap_create(size_t nbuckets) {
	pfmap_t *m = (pfmap_t*)malloc(sizeof(pfmap_t));
	m->buckets = (pfmap_node_t**)calloc(nbuckets, sizeof(pfmap_node_t*));
	m->nbuckets = nbuckets;
	return m;
}

static inline void pfmap_destroy(pfmap_t *m) {
	for (size_t i = 0; i < m->nbuckets; i++) {
		pfmap_node_t *n = m->buckets[i];
		while (n) {
			pfmap_node_t *nx = n->next;
			pfd_free(&n->pd);
			free(n);
			n = nx;
		}
	}
	free(m->buckets);
	free(m);
}

/* key must be exactly PFDICT_KEYLEN bytes, zero-padded past the real prefix length. */
static inline pfmap_node_t* pfmap_find(pfmap_t *m, const uint8_t *key) {
	size_t h = pfmap_hash(key, m->nbuckets);
	pfmap_node_t *n = m->buckets[h];
	while (n) {
		if (memcmp(n->key, key, PFDICT_KEYLEN) == 0) return n;
		n = n->next;
	}
	return NULL;
}

/* Caller must ensure `key` isn't already present. */
static inline pfmap_node_t* pfmap_insert(pfmap_t *m, const uint8_t *key) {
	size_t h = pfmap_hash(key, m->nbuckets);
	pfmap_node_t *n = (pfmap_node_t*)malloc(sizeof(pfmap_node_t));
	memcpy(n->key, key, PFDICT_KEYLEN);
	pfd_init(&n->pd);
	n->next = m->buckets[h];
	m->buckets[h] = n;
	return n;
}

#endif // UTILS_H
