---
reference: NT-01
type: Technical note
version: 5.1
version_date: 5.1 — September 2026
langue: en
titre: NIST SP 800-90B entropy assessment as a C library
titre_court: NIST SP 800-90B entropy assessment as a C library
sous_titre: libsp80090b: measuring the min-entropy of a noise source on the target machine, deciding whether it conforms and sizing a hybrid generator
objet: libsp80090b: method for assessing an entropy source according to SP 800-90B
auteurs: EPI Quantum Crypto team
diffusion: Technical note — public release
pied: Technical note — public release
referentiels: NIST SP 800-90A, SP 800-90B, SP 800-90C; FIPS 140-3 (IG D.K)
documents_lies: NT-02 User guide (API and labs); NT-03 Good-practice guide; French version: NT-01 (fr)
remplace: NT-01 v5.0
mots_cles: SP 800-90B; min-entropy; IID; non-IID; restart; conditioning; health tests; hybrid generator; DRBG; SP 800-90C
---

::: resume
For a physical random number generator (TRNG), assessing the entropy takes rigorous effort: modelling the noise source, capturing millions of raw samples under real operating conditions, then running long statistical tests. This effort is essential, because a deterministic random bit generator (DRBG) does not create entropy: it stretches the entropy of its seed. The security of the whole cryptographic chain therefore rests on a single quantity, the **min-entropy per sample** H of the noise source, which the **NIST SP 800-90B** recommendation [1] estimates with four tests.

**libsp80090b** is a C99 port of the NIST reference tool [2] as a library: its four functions assess an in-memory buffer, directly on the target machine. It gives the same values as the NIST tool, and a campaign of 52 cases on 22 datasets confirms that each test behaves as the theory predicts, with its limits: no statistical test can tell a DRBG output from real noise. The note explains the method, from the measurement to the sizing of the seed and of the health tests of a hybrid generator.
:::

# Introduction and context {#sec:intro}

A failing entropy source never reports its own failure. A biased, correlated or frozen source feeds the DRBG just like a good source, and the DRBG produces an output that passes every statistical test. The weakness only shows when an attacker manages to reconstruct the seed. The only protection is to **measure the min-entropy of the raw noise** before any processing, then to monitor the source continuously.

## Goals of the SP 800-90B assessment

1. **Guarantee cryptographic security.** Make sure that the required entropy, for example 256 bits for the seed, is really present.
2. **Detect hardware failures.** Ageing, temperature or voltage variations, environmental disturbances.
3. **Size the generator.** Derive from H the number of samples N = ⌈E / H⌉ to collect to seed the DRBG, and the cutoffs of the RCT and APT health tests.

## Vocabulary

Table: Terms used in this document. {#tab:vocab widths=28,72}

| Term | Definition |
|---|---|
| **Noise source** | Non-deterministic physical or logical component: clock jitter, ring oscillator, thermal noise |
| **Sample (symbol)** | Digitised value produced by the source, 1 to 8 bits, stored one sample per byte |
| **Raw noise** | Samples before any processing or conditioning. The only legitimate subject of an assessment |
| **Min-entropy H** | H = −log~2~(p~max~), where p~max~ is the probability of the most likely value: the difficulty of an adversary's best guess |
| **IID** | Independent and identically distributed samples |
| **H~original~, H~bitstring~** | Min-entropy of the samples as they are, and of the sequence of their bits |
| **H~I~** | Initial entropy: min(H~original~, n × H~bitstring~) for n-bit samples |
| **Conditioning** | Deterministic function (SHA-256, HMAC, CBC-MAC) that concentrates entropy without creating any |
| **Security strength s** | Effort, in bits, to predict the DRBG output: s = 256 in our generators |

# Why a library {#sec:pourquoi}

## The need: measuring on the target machine

Entropy is not a constant of a component: it varies with the processor frequency, the load, the temperature and virtualisation. On a virtual machine, the same jitter source gave 2.67 bits per sample in one campaign, then 1.16 bits in another [10]. An assessment made once, in the laboratory, on a captured file therefore does not guarantee the value on the production machine.

## Limits of the NIST reference tool

The NIST tool, SP800-90B_EntropyAssessment [2], is the reference: it produces the results of a validation submission. But it is a set of C++11 executables that work offline:

- it requires the data to be written to disk, then read back;
- it provides no API: the result has to be extracted from a text or JSON output;
- it depends on heavy libraries (libstdc++, jsoncpp, bzip2, gmp, mpfr, OpenSSL), often missing from embedded environments.

## What libsp80090b provides

libsp80090b performs the four tests of the standard through **four C99 functions** that work directly on an in-memory buffer:

```Non-IID assessment of a buffer of 1,000,000 samples
#include "sp80090b.h"

sp80090b_options_t opt = sp80090b_default_options();
sp80090b_result_t res;
opt.bits_per_symbol = 8;

/* non-IID track on 1,000,000 raw samples held in memory */
if (sp80090b_non_iid_evaluate_ex(buffer, 1000000, &opt, &res) == SP80090B_OK
    && res.warnings == 0)
    printf("Assessed min-entropy H_I = %.5f bits per sample\n", res.h_assessed);
```

Table: The four API functions. {#tab:api widths=34,14,52}

| Function | Test | Role |
|---|---|---|
| `sp80090b_iid_evaluate_ex()` | 1 (§5) | Checks the IID assumption: chi-square, longest repeated substring, 10,000 permutations |
| `sp80090b_non_iid_evaluate_ex()` | 2 (§6.3) | Computes the ten non-IID estimators |
| `sp80090b_restart_evaluate()` | 3 (§3.1.4) | Sanity check and entropy at restart, on a 1,000 × 1,000 matrix |
| `sp80090b_conditioning_evaluate()` | 4 (§3.1.5) | Output entropy of a conditioning function |

The `_ex` suffix (*extended*) marks the version 2 functions, which take an options structure. The four labs of the validation campaign call these functions through small programs (`test_iid`, `test_non_iid`, `test_restart`, `test_conditioning`); their use is described in NT-02.

**Conformity.** On the eleven NIST reference datasets, the non-IID track is identical to the NIST tool to within 7.2 × 10^−14^, and the IID track reproduces all of its deterministic statistics bit for bit (`make check`, NT-02).

The library does not replace three things: the **modelling** of the source, which is the designer's work; the **health tests**, which stay in the product; and the **ESV validation submission**, built with the NIST tool and assessed by NIST itself [5].

# The SP 800-90B qualification framework {#sec:cadre}

SP 800-90B splits an entropy source into three stages: the **noise source**, where the entropy comes from; the **health tests**, which check that it still works; and an optional **conditioning** stage. Figure {@fig:placement} places each test in the complete chain of a generator, from the source to the SP 800-90A DRBG [12], within an SP 800-90C construction [3].

::: fig placement Place of the tests in the SP 800-90B, 90A and 90C chain: offline assessment in cyan, continuous monitoring in red. {#fig:placement}

## Model first, then measure {#sec:modele}

NIST requires two complementary approaches [11]:

1. **The model.** A physical justification of the noise, which sets an entropy claim H~claim~ per sample.
2. **The statistical estimate.** A black-box check of the raw noise, performed by libsp80090b. Without knowing the inside of the source, it can be wrong: it checks the model, it does not replace it.

> **Rule.** The entropy used to size the generator is always the lower of the two: H = min(H~claim~, H~measured~).

## Offline assessment and continuous monitoring {#sec:modes}

Table: Offline assessment and continuous monitoring. {#tab:modes widths=20,40,40}

| Criterion | Offline assessment (libsp80090b) | Continuous monitoring (RCT and APT) |
|---|---|---|
| **Goal** | Measure the min-entropy H of the source | Detect a failure of the source |
| **Data** | 1,000,000 samples and 1,000 restarts | Every sample, as it is produced |
| **When** | Qualification, installation, kernel or hardware update | All the time, from start-up |
| **Cost** | Heavy: about 250 MB, from a few seconds to 2 hours of computation | Very light: a few bytes, a few operations per sample |
| **On failure** | Source refused, or claim revised downwards | Current seed discarded, output stopped, alarm |

The two families complement each other: offline assessment gives the value H, continuous monitoring checks that the source does not fall below that value, since its cutoffs are computed from H.

# The four tests of SP 800-90B {#sec:tests}

## Test 1: the IID assumption (section 5) {#sec:t1}

- **Goal.** Check that the samples are independent and identically distributed. If they are, a single estimator is enough: the most common value (MCV).
- **Data.** At least 1,000,000 consecutive raw samples.
- **Method.** Two chi-square tests, independence and goodness of fit (§5.2.1 to §5.2.4); the longest repeated substring test (§5.2.5); finally 10,000 random permutations of the file (§5.1), on which 19 statistics are recomputed: excursion, increasing and decreasing runs, runs around the median, collisions, periodicity, covariance and bzip2 compression.
- **Decision.** For each statistic i, the counters C~i,0~, C~i,1~ and C~i,2~ count the permutations whose statistic is greater than, equal to or smaller than that of the data. If C~i,0~ + C~i,1~ ≤ 5 or C~i,1~ + C~i,2~ ≤ 5, the IID assumption is **rejected** and the source moves to test 2.
- **Run time.** Under a minute for an IID source; 40 minutes to 2 hours for a non-IID source, which runs all 10,000 permutations.

>! **Warning.** A source only follows the IID track **if its designer has justified this assumption**. Passing the IID tests does not prove independence [11]: when in doubt, apply test 2.

## Test 2: the non-IID estimators (section 6.3) {#sec:t2}

- **Goal.** Estimate the min-entropy without any independence assumption: the case of almost every physical source.
- **Method.** Ten estimators, each looking for one form of regularity; four of them are predictors, which try to guess the next sample [7].

Table: The ten non-IID estimators. {#tab:estim widths=30,10,40,20}

| Estimator | § | Principle | Applied to |
|---|---|---|---|
| **Most common value (MCV)** | 6.3.1 | Frequency of the most common symbol | Samples and bits |
| **Collision** | 6.3.2 | Mean time before a value repeats | Bits only |
| **Markov** | 6.3.3 | First-order Markov chain | Bits only |
| **Compression** | 6.3.4 | Maurer statistic on 6-bit blocks | Bits only |
| **t-tuple** | 6.3.5 | Frequency of the most repeated t-tuples | Samples and bits |
| **Longest repeated substring (LRS)** | 6.3.6 | Collisions of long substrings, through a suffix array [8] | Samples and bits |
| **MultiMCW** | 6.3.7 | Predictor: majority value over four sliding windows | Samples and bits |
| **Lag** | 6.3.8 | Predictor: value observed d positions earlier (d ≤ 128) | Samples and bits |
| **MultiMMC** | 6.3.9 | Predictor: Markov models of orders 1 to 16 | Samples and bits |
| **LZ78Y** | 6.3.10 | Predictor: LZ78-type dictionary | Samples and bits |

- **Result.** H~original~ and H~bitstring~ are the minimum of the applicable estimators; the initial entropy is H~I~ = min(H~original~, n × H~bitstring~).
- **Run time.** 5 seconds to 1 minute for 10^6^ samples.

## Test 3: restart (section 3.1.4) {#sec:t3}

- **Goal.** Check that the entropy measured in steady state is really present from power-up: no frozen or recurring initial state.
- **Data.** A matrix M of r = 1,000 real restarts and c = 1,000 samples per restart. Row i holds restart i; column j gathers the j-th sample of every restart.
- **Sanity check** (§3.1.4.3). The count of the most common value in each row and each column must not exceed a cutoff X~cutoff~, obtained by simulation for an overall risk of 0.01 [6]. A source that always restarts in the same state gives a constant column and fails.
- **Validation** (§3.1.4.2). The estimates on the rows (H~r~) and on the columns (H~c~) must satisfy min(H~r~, H~c~) ≥ H~I~ / 2.
- **Result.** The validated entropy is H = min(H~r~, H~c~, H~I~).

## Test 4: conditioning (section 3.1.5) {#sec:t4}

- **Goal.** Compute the entropy h~out~ left after a conditioning function, for example SHA-256.
- **Vetted function** (HMAC, CMAC, CBC-MAC, SHA-2, SHA-3, Hash_df). h~out~ is computed with the Output_Entropy(n~in~, n~out~, nw, h~in~) formula of the standard, from the input entropy h~in~.
- **Non-vetted function.** h~out~ is further capped at 0.999 × n~out~ and at h′ × n~out~, where h′ is the entropy per bit measured on one million outputs: h~out~ = min(Output_Entropy, 0.999 n~out~, h′ n~out~).
- **Full entropy** (FIPS 140-3 IG D.K [4], SP 800-90C [3]). An output has full entropy if h~out~ equals n~out~ to within ε ≤ 2^−32^; this requires a vetted function and a margin of at least 64 bits: **h~in~ ≥ n~out~ + 64**.

# Deciding whether a source conforms {#sec:conformite}

A source is declared conforming if the following five criteria are met:

1. **Data** (§3.1.1). At least 1,000,000 consecutive raw samples, with more than one symbol: a stuck source is rejected outright.
2. **Initial entropy H~I~.** Through the IID track if it is justified and the assumption holds, otherwise through the ten non-IID estimators.
3. **Restart sanity.** X~max~ ≤ X~cutoff~ on every row and column.
4. **Restart entropy.** min(H~r~, H~c~) ≥ H~I~ / 2; the validated entropy is H = min(H~r~, H~c~, H~I~).
5. **Claim.** H is at least equal to the claim H~claim~ of the model; the lower of the two values is then credited.

# Integration into a hybrid generator {#sec:hybride}

A hybrid generator combines an entropy source, assessed according to SP 800-90B, with a DRBG that stretches its seed: this is the RBG2 construction of SP 800-90C [3]. For a security strength s = 256 bits, sizing takes three steps.

## Entropy to supply to the DRBG {#sec:graine}

Table: Entropy to supply to the DRBG for a 256-bit strength. {#tab:entropie widths=30,25,45}

| Operation | Required entropy E | Reason |
|---|---|---|
| **Instantiation** | 384 bits (3s/2) | s bits of entropy and s/2 bits of nonce |
| **Reseed** | 256 bits (s) | Restores the security strength after a compromise of the state |
| **Full-entropy output (RBG3)** | 320 bits (s + 64) per 256-bit block | 64-bit margin of FIPS 140-3 IG D.K |

## Number of raw samples

From the validated entropy H, output of test 3, the number of samples to collect is N = ⌈E / H⌉.

Table: Number of raw samples per operation. {#tab:echantillons widths=40,12,16,14,18}

| Source and entropy credited | H (bit/sample) | Instantiation 384 | Reseed 256 | Full entropy 320 |
|---|---|---|---|---|
| jitterentropy [9], design claim (osr = 3) | 0.3333 | 1,152 | 768 | 960 |
| Thermal noise of Lab 3, validated H (NT-03) | 4.0927 | 94 | 63 | 79 |

## Health test cutoffs

The cutoffs of the health tests are computed from the credited entropy H, for a false alarm rate α = 2^−20^:

- **RCT** (Repetition Count Test, §4.4.1): the test fails if the same value repeats C = 1 + ⌈20 / H⌉ times in a row;
- **APT** (Adaptive Proportion Test, §4.4.2): in a window of W = 512 samples (1,024 for a binary source), the test fails if a value appears at least C times, C being the smallest integer whose probability is ≤ 2^−20^.

Table: Health test cutoffs (α = 2^−20^). {#tab:seuils widths=46,16,19,19}

| Source and credited entropy H | RCT: C | APT: W | APT: C |
|---|---|---|---|
| jitterentropy, claim (1/3 bit) | 61 | 512 | 449 |
| Thermal noise of Lab 3 (4.0927 bits) | 6 | 512 | 59 |
| Ideal binary source (1 bit) | 21 | 1,024 | 589 |

No sample enters the seed without passing these tests; at the first failure, the current seed is discarded.

# Lessons from the labs {#sec:enseignements}

The NT-03 campaign applies the four tests to 22 datasets: healthy sources, design defects, environmental effects, failures and DRBG outputs. The "physical" sources are **simulated** from a model whose entropy is known, so that the measurement can be checked against the theory: 5.618 bits measured for 5.65 expected on thermal noise. Six lessons emerge.

1. **The tests do not see determinism.** An AES-256 stream with an all-zero key passes the IID tests with 7.87 bits per byte, and still gets 7.15 bits through the non-IID track, for a real entropy of zero. Rule: never assess a DRBG or hash output.
2. **IID does not mean high entropy.** A saturated converter passes the IID tests with only 1.92 bits of entropy out of 8.
3. **A single test is enough to reject the IID assumption.** A slowly read oscillator passes the chi-square and longest repeated substring tests, but a single permutation statistic out of 19 rejects it.
4. **The IID tests have a false rejection rate.** A truly IID source is rejected in 2 to 3 % of cases, because of the permutation threshold (C ≤ 5 out of 10,000 for each of the 19 statistics); the standard then requires the non-IID track.
5. **Only the restart test sees some defects.** A source whose first samples are identical at every start-up looks normal in steady state; only test 3 rejects it.
6. **The conditioning margin is essential.** SHA-256 receiving exactly 256 bits of entropy outputs only 251.69 bits; with 320 input bits, the output has full entropy.

# Conclusion

libsp80090b makes it possible to industrialise the qualification of entropy sources directly on their execution platform. It performs the four tests of SP 800-90B with the same results as the NIST tool, and provides the parameters of the hybrid generator: the number of seed samples N and the cutoffs of the RCT and APT health tests.

The measurement remains a check: the claimed entropy comes first from the model of the source, and a DRBG output fools every test. For an ESV validation submission, the NIST tool remains the reference.

# References

[1] M. Sönmez Turan, E. Barker, J. Kelsey, K. McKay, M. Baish, M. Boyle. *Recommendation for the Entropy Sources Used for Random Bit Generation*. NIST SP 800-90B, January 2018.
[2] National Institute of Standards and Technology. *SP800-90B_EntropyAssessment*. Source code, github.com/usnistgov/SP800-90B_EntropyAssessment.
[3] E. Barker, J. Kelsey, K. McKay, A. Roginsky, M. Sönmez Turan. *Recommendation for Random Bit Generator (RBG) Constructions*. NIST SP 800-90C, September 2025.
[4] National Institute of Standards and Technology. *Security Requirements for Cryptographic Modules*. FIPS PUB 140-3, 2019; Implementation Guidance, D.K.
[5] NIST CMVP. *Entropy Source Validation (ESV) Program*. ESV program documentation.
[6] P. Hagerty, T. Draper. *Entropy Bounds and Statistical Tests*. NIST Random Bit Generation Workshop, 2012.
[7] J. Kelsey, K. A. McKay, M. Sönmez Turan. *Predictive Models for Min-Entropy Estimation*. CHES 2015, LNCS 9293.
[8] Y. Mori. *libdivsufsort — suffix-sorting library*. Source code, version 2.0.
[9] S. Müller. *Jitter RNG (jitterentropy)*. Documentation and source code, version 3.7.1, 2026.
[10] EPI Quantum Crypto team. *entropy-lab — collect, test and assess the entropy of a PC*. Entropy/entropy-lab/README.md, 2026.
[11] T. Hall, J. Kelsey, M. Sönmez Turan. *Entropy Sources — Importance and testing*. NIST, Information Security and Privacy Advisory Board, 26 October 2022.
[12] E. Barker, J. Kelsey. *Recommendation for Random Number Generation Using Deterministic Random Bit Generators*. NIST SP 800-90A Rev. 1, June 2015.
