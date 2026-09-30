/* SPDX-License-Identifier: LicenseRef-NIST-Software AND MIT
 * Part of libsp80090b. Derived from NIST SP800-90B_EntropyAssessment 1.1.8;
 * ported to C99 and modified in 2026 by EPI Quantum Crypto team (see NOTICE). */
#ifndef CHI_SQUARE_TESTS_H
#define CHI_SQUARE_TESTS_H

#include "utils.h"

/*
 * Cephes Math Library Release 2.8:  June, 2000
 * Copyright 1984, 1995, 2000 by Stephen L. Moshier
 * (BSD-licensed; see SP800-90B_code/cpp/iid/chi_square_tests.h for the full notice)
 */

static double MACHEP = 1.11022302462515654042E-16;     // 2**-53
static double MAXLOG = 7.09782712893383996732224E2; // log(MAXNUM)
static double MAXNUM = 1.7976931348623158E308;         // 2**1024*(1-MACHEP)
static double PI_cephes = 3.14159265358979323846;

static double cephes_big = 4.503599627370496e15;
static double cephes_biginv = 2.22044604925031308085e-16;

/* Written by cephes_lgam(): thread-local so that concurrent IID evaluations
 * from several threads do not race (the NIST tool is single-process). */
static __thread int sgngam = 0;

static double cephes_A[] = {
   8.11614167470508450300E-4,
   -5.95061904284301438324E-4,
   7.93650340457716943945E-4,
   -2.77777777730099687205E-3,
   8.33333333333331927722E-2
};
static double cephes_B[] = {
   -1.37825152569120859100E3,
   -3.88016315134637840924E4,
   -3.31612992738871184744E5,
   -1.16237097492762307383E6,
   -1.72173700820839662146E6,
   -8.53555664245765465627E5
};
static double cephes_C[] = {
   -3.51815701436523470549E2,
   -1.70642106651881159223E4,
   -2.20528590553854454839E5,
   -1.13933444367982507207E6,
   -2.53252307177582951285E6,
   -2.01889141433532773231E6
};

#define MAXLGM 2.556348e305

static double cephes_polevl(double x, double *coef, int N) {
	double ans;
	int i;
	double *p;

	p = coef;
	ans = *p++;
	i = N;

	do {
		ans = ans * x + *p++;
	} while (--i);

	return ans;
}

static double cephes_p1evl(double x, double *coef, int N) {
	double ans;
	double *p;
	int i;

	p = coef;
	ans = x + *p++;
	i = N - 1;

	do {
		ans = ans * x + *p++;
	} while (--i);

	return ans;
}

static double cephes_lgam(double x) {
	double p, q, u, w, z;
	int i;

	sgngam = 1;

	if (x < -34.0) {
		q = -x;
		w = cephes_lgam(q);
		p = floor(q);

		if (relEpsilonEqual(p, q, DBL_EPSILON, DBL_EPSILON, 4)) goto loverf;

		i = (int)p;
		if ((i & 1) == 0) sgngam = -1;
		else sgngam = 1;

		z = q - p;
		if (z > 0.5) {
			p += 1.0;
			z = p - q;
		}

		z = q * sin(PI_cephes * z);
		if (relEpsilonEqual(z, 0.0, DBL_EPSILON, DBL_EPSILON, 4)) goto loverf;

		z = log(PI_cephes) - log(z) - w;
		return z;
	}

	if (x < 13.0) {
		z = 1.0;
		p = 0.0;
		u = x;

		while (u >= 3.0) {
			p -= 1.0;
			u = x + p;
			z *= u;
		}

		while (u < 2.0) {
			if (relEpsilonEqual(u, 0.0, DBL_EPSILON, DBL_EPSILON, 4)) goto loverf;
			z /= u;
			p += 1.0;
			u = x + p;
		}

		if (z < 0.0) {
			sgngam = -1;
			z = -z;
		} else {
			sgngam = 1;
		}

		if (relEpsilonEqual(u, 2.0, DBL_EPSILON, DBL_EPSILON, 4)) return (log(z));

		p -= 2.0;
		x = x + p;
		p = x * cephes_polevl(x, cephes_B, 5) / cephes_p1evl(x, cephes_C, 6);

		return log(z) + p;
	}

	if (x > MAXLGM) {
loverf:
		/* NIST prints "lgam: OVERFLOW" on stderr; a library stays silent. */
		return sgngam * MAXNUM;
	}

	q = (x - 0.5) * log(x) - x + log(sqrt(2 * PI_cephes));

	if (x > 1.0e8) return q;

	p = 1.0 / (x * x);

	if (x >= 1000.0) {
		q += ((7.9365079365079365079365e-4 * p - 2.7777777777777777777778e-3) * p + 0.0833333333333333333333) / x;
	} else {
		q += cephes_polevl(p, cephes_A, 4) / x;
	}

	return q;
}

static double cephes_igamc(double a, double x);

static double cephes_igam(double a, double x) {
	double ans, ax, c, r;

	if ((x <= 0) || (a <= 0)) return 0.0;
	if ((x > 1.0) && (x > a)) return 1.e0 - cephes_igamc(a, x);

	ax = a * log(x) - x - cephes_lgam(a);
	if (ax < -MAXLOG) {
		/* NIST prints "igam: UNDERFLOW" on stderr; a library stays silent. */
		return 0.0;
	}

	ax = exp(ax);

	r = a;
	c = 1.0;
	ans = 1.0;

	do {
		r += 1.0;
		c *= x / r;
		ans += c;
	} while (c / ans > MACHEP);

	return ans * ax / a;
}

static double cephes_igamc(double a, double x) {
	double ans, ax, c, yc, r, t, y, z;
	double pk, pkm1, pkm2, qk, qkm1, qkm2;

	if ((x <= 0) || (a <= 0)) return (1.0);
	if ((x < 1.0) || (x < a)) return (1.e0 - cephes_igam(a, x));

	ax = a * log(x) - x - cephes_lgam(a);
	if (ax < -MAXLOG) {
		/* NIST prints "igamc: UNDERFLOW" on stderr; a library stays silent. */
		return 0.0;
	}

	ax = exp(ax);

	y = 1.0 - a;
	z = x + y + 1.0;
	c = 0.0;
	pkm2 = 1.0;
	qkm2 = x;
	pkm1 = x + 1.0;
	qkm1 = z * x;
	ans = pkm1 / qkm1;

	do {
		c += 1.0;
		y += 1.0;
		z += 2.0;
		yc = y * c;
		pk = pkm1 * z - pkm2 * yc;
		qk = qkm1 * z - qkm2 * yc;

		if (!relEpsilonEqual(qk, 0.0, DBL_EPSILON, DBL_EPSILON, 4)) {
			r = pk / qk;
			t = fabs((ans - r) / r);
			ans = r;
		} else {
			t = 1.0;
		}

		pkm2 = pkm1;
		pkm1 = pk;
		qkm2 = qkm1;
		qkm1 = qk;

		if (fabs(pk) > cephes_big) {
			pkm2 *= cephes_biginv;
			pkm1 *= cephes_biginv;
			qkm2 *= cephes_biginv;
			qkm1 *= cephes_biginv;
		}
	} while (t > MACHEP);

	return ans * ax;
}

static inline double chi_square_pvalue(double x, double k) {
	return cephes_igamc(k / 2.0, x / 2.0);
}

/*
 * ---------------------------------------------
 * HELPERS FOR CHI_SQUARE_INDEPENDENCE / GOODNESS OF FIT
 * ---------------------------------------------
 */
typedef struct {
	uint16_t tuple;
	double expectation;
	int bin;
} tupleTranslateEntry_t;

static void calc_proportions(const uint8_t data[], double *p, const int alph_size, const int sample_size) {
	for (int i = 0; i < alph_size; i++) p[i] = 0.0;
	for (int i = 0; i < sample_size; i++) p[data[i]] += 1.0;
	for (int i = 0; i < alph_size; i++) p[i] /= (double)sample_size;
}

static void independence_calc_expectations(const double *p, int alph_size, tupleTranslateEntry_t *e, const int sample_size) {
	uint16_t index;
	for (int i = 0; i < alph_size; i++) {
		for (int j = 0; j < alph_size; j++) {
			index = (uint16_t)((i * alph_size) + j);
			e[index].tuple = index;
			e[index].expectation = p[i] * p[j] * floor(sample_size * 0.5);
			e[index].bin = -1;
		}
	}
}

static int expectationOrder(const void *av, const void *bv) {
	const tupleTranslateEntry_t *a = (const tupleTranslateEntry_t*)av;
	const tupleTranslateEntry_t *b = (const tupleTranslateEntry_t*)bv;
	if (a->expectation != b->expectation) return (a->expectation < b->expectation) ? -1 : 1;
	if (a->tuple != b->tuple) return (a->tuple < b->tuple) ? -1 : 1;
	return 0;
}

static int tupleOrder(const void *av, const void *bv) {
	const tupleTranslateEntry_t *a = (const tupleTranslateEntry_t*)av;
	const tupleTranslateEntry_t *b = (const tupleTranslateEntry_t*)bv;
	if (a->tuple != b->tuple) return (a->tuple < b->tuple) ? -1 : 1;
	return 0;
}

/* Allocates *bin_exp (caller frees) and fills *n_bins. */
static void allocate_bins(tupleTranslateEntry_t *e, int n, double **bin_exp_out, int *n_bins_out) {
	int current_bin = 0;
	double current_expectation = 0.0;
	int cap = 16, n_bins = 0;
	double *bin_exp = (double*)malloc(cap * sizeof(double));

	for (int i = 0; i < n; i++) {
		if (current_expectation >= 5.0) {
			if (n_bins == cap) { cap *= 2; bin_exp = (double*)realloc(bin_exp, cap * sizeof(double)); }
			bin_exp[n_bins++] = current_expectation;
			current_bin++;
			current_expectation = 0.0;
		}

		e[i].bin = current_bin;
		current_expectation += e[i].expectation;
	}

	if ((current_bin != 0) && (current_expectation < 5.0)) {
		// Combine the last two bins
		for (int i = n - 1; e[i].bin == current_bin; i--) {
			e[i].bin = current_bin - 1;
		}
		bin_exp[n_bins - 1] += current_expectation;
	} else {
		if (n_bins == cap) { cap *= 2; bin_exp = (double*)realloc(bin_exp, cap * sizeof(double)); }
		bin_exp[n_bins++] = current_expectation;
	}

	*bin_exp_out = bin_exp;
	*n_bins_out = n_bins;
}

static void independence_calc_observed(const uint8_t data[], const tupleTranslateEntry_t *e, int *o, const int sample_size, const int alphabet_size) {
	for (int j = 0; j < sample_size - 1; j += 2) {
		uint16_t index = (uint16_t)((data[j] * alphabet_size) + data[j + 1]);
		o[e[index].bin]++;
	}
}

static double calc_T(const double *bin_expectations, const int *o, int n_bins) {
	double T = 0.0;
	for (int i = 0; i < n_bins; i++) {
		double diff = o[i] - bin_expectations[i];
		T += (diff * diff) / bin_expectations[i];
	}
	return T;
}

static void goodness_of_fit_calc_observed(const uint8_t data[], const tupleTranslateEntry_t *e, int *o, const int sample_size) {
	for (int j = 0; j < sample_size; j++) {
		o[e[data[j]].bin]++;
	}
}

/*
 * ---------------------------------------------
 * TESTS
 * ---------------------------------------------
 */
static void binary_chi_square_independence(const uint8_t data[], double *score, int *df, const int sample_size) {
	double p0 = 0.0, p1 = 0.0;

	for (int i = 0; i < sample_size; i++) p1 += data[i];
	p1 /= sample_size;
	p0 = 1.0 - p1;

	double min_p = MIN(p0, p1);
	int m = 11;
	int threshhold = 5;
	while (m > 1) {
		/* integer division, as in the NIST tool */
		if (pow(min_p, m) * (double)(sample_size / m) >= threshhold) break;
		else m--;
	}

	int tuple_count = 1 << m;

	if (m < 2) {
		*score = 0.0;
		*df = 0;
		return;
	}

	double T = 0;
	int *occ = (int*)calloc(tuple_count, sizeof(int));
	int block_count = sample_size / m;

	for (int i = 0; i < block_count; i++) {
		int symbol = 0;
		for (int j = 0; j < m; j++) {
			symbol = (symbol << 1) | data[i * m + j];
		}
		occ[symbol]++;
	}

	for (int i = 0; i < tuple_count; i++) {
		int w = __builtin_popcount((unsigned int)i);
		double e = pow(p1, w) * pow(p0, m - w) * block_count;
		double diff = occ[i] - e;
		T += (diff * diff) / e;
	}

	free(occ);

	*score = T;
	*df = (1 << m) - 2;
}

static void chi_square_independence(const uint8_t data[], double *score, int *df, const int sample_size, const int alphabet_size) {
	double *p = (double*)malloc(alphabet_size * sizeof(double));
	calc_proportions(data, p, alphabet_size, sample_size);

	int n_tuples = alphabet_size * alphabet_size;
	tupleTranslateEntry_t *e = (tupleTranslateEntry_t*)malloc(n_tuples * sizeof(tupleTranslateEntry_t));
	independence_calc_expectations(p, alphabet_size, e, sample_size);
	free(p);

	qsort(e, n_tuples, sizeof(tupleTranslateEntry_t), expectationOrder);

	double *bin_expectations;
	int n_bins;
	allocate_bins(e, n_tuples, &bin_expectations, &n_bins);

	qsort(e, n_tuples, sizeof(tupleTranslateEntry_t), tupleOrder);

	int *o = (int*)calloc(n_bins, sizeof(int));
	independence_calc_observed(data, e, o, sample_size, alphabet_size);

	*score = calc_T(bin_expectations, o, n_bins);
	*df = n_bins - alphabet_size;

	free(e);
	free(bin_expectations);
	free(o);
}

static void binary_goodness_of_fit(const uint8_t data[], double *score, int *df, const int sample_size) {
	int sublength = sample_size / 10;
	int ones = 0;

	for (int i = 0; i < sample_size; i++) ones += data[i];

	double p = (double)ones / (double)sample_size;
	double T = 0;

	double e0 = (1.0 - p) * sublength;
	double e1 = p * sublength;

	for (int i = 0; i < 10; i++) {
		int o1 = 0;
		for (int j = 0; j < sublength; j++) o1 += data[i * sublength + j];
		int o0 = sublength - o1;

		double d0 = o0 - e0, d1 = o1 - e1;
		T += (d0 * d0 / e0) + (d1 * d1 / e1);
	}

	*score = T;
	*df = 9;
}

static void goodness_of_fit(const uint8_t data[], double *score, int *df, const int sample_size, const int alphabet_size) {
	double *p = (double*)malloc(alphabet_size * sizeof(double));
	calc_proportions(data, p, alphabet_size, sample_size);

	tupleTranslateEntry_t *e = (tupleTranslateEntry_t*)malloc(alphabet_size * sizeof(tupleTranslateEntry_t));
	for (int j = 0; j < alphabet_size; j++) {
		e[j].tuple = (uint16_t)j;
		e[j].expectation = p[j] * floor((double)sample_size / 10.0);
		e[j].bin = -1;
	}
	free(p);

	qsort(e, alphabet_size, sizeof(tupleTranslateEntry_t), expectationOrder);

	double *bin_expectations;
	int n_bins;
	allocate_bins(e, alphabet_size, &bin_expectations, &n_bins);

	qsort(e, alphabet_size, sizeof(tupleTranslateEntry_t), tupleOrder);

	int block_size = sample_size / 10;
	double T = 0.0;
	int *o = (int*)malloc(n_bins * sizeof(int));

	for (int j = 0; j < 10; j++) {
		for (int i = 0; i < n_bins; i++) o[i] = 0;
		goodness_of_fit_calc_observed(data + j * block_size, e, o, block_size);
		T += calc_T(bin_expectations, o, n_bins);
	}

	*score = T;
	*df = 9 * (n_bins - 1);

	free(e);
	free(bin_expectations);
	free(o);
}

/* SP 800-90B 5.2.1 to 5.2.4. *min_pvalue (optional) receives the smaller
 * of the two p-values; a test fails when its p-value is below 0.001. */
static inline bool chi_square_tests(const uint8_t* data, long len, const int alph_size, const int verbose, double *min_pvalue) {
	double score = 0.0;
	double pvalue;
	int df = 0;
	bool result = true;
	int sample_size = (int)len;

	// Chi square independence test
	if (alph_size == 2) {
		binary_chi_square_independence(data, &score, &df, sample_size);
	} else {
		chi_square_independence(data, &score, &df, sample_size, alph_size);
	}

	pvalue = chi_square_pvalue(score, df);

	if (verbose == 2) {
		printf("Chi square independence\n");
		printf("\tscore = %f\n", score);
		printf("\tdegrees of freedom = %d\n", df);
		printf("\tp-value = %f\n\n", pvalue);
	} else if (verbose >= 3) {
		printf("Chi square independence: T = %.17g\n", score);
		printf("Chi square independence: df = %d\n", df);
		printf("Chi square independence: P-value = %.17g\n", pvalue);
	}

	if (pvalue < 0.001) result = false;
	if (min_pvalue) *min_pvalue = pvalue;

	score = 0.0;
	df = 0;

	// Chi square goodness of fit test
	if (alph_size == 2) {
		binary_goodness_of_fit(data, &score, &df, sample_size);
	} else {
		goodness_of_fit(data, &score, &df, sample_size, alph_size);
	}

	pvalue = chi_square_pvalue(score, df);

	if (verbose == 2) {
		printf("Chi square goodness of fit\n");
		printf("\tscore = %f\n", score);
		printf("\tdegrees of freedom = %d\n", df);
		printf("\tp-value = %f\n\n", pvalue);
	} else if (verbose >= 3) {
		printf("Chi square goodness of fit: T = %.17g\n", score);
		printf("Chi square goodness of fit: df = %d\n", df);
		printf("Chi square goodness of fit: P-value = %.17g\n", pvalue);
	}

	if (pvalue < 0.001) result = false;
	if (min_pvalue && pvalue < *min_pvalue) *min_pvalue = pvalue;

	return result;
}

#endif // CHI_SQUARE_TESTS_H
