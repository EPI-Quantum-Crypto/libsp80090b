---
reference: NT-03
type: Good practice guide
version: 4.1
version_date: 4.1 — September 2026
langue: en
titre: libsp80090b: entropy assessment in practice
titre_court: Entropy assessment in practice
sous_titre: Good practice of SP 800-90B through the 18 lab cases, the restart test and conditioning
objet: Mastering the min-entropy assessment of a noise source with libsp80090b
auteurs: EPI Quantum Crypto team
diffusion: Good practice guide — public release
pied: Good practice guide — public release
referentiels: NIST SP 800-90B; SP 800-90C; FIPS 140-3 IG D.K
documents_lies: NT-01 Technical note (method); NT-02 User guide (API and labs); French version: NT-03 (fr)
remplace: NT-03 v4.0
mots_cles: libsp80090b; min-entropy; good practice; IID track; non-IID track; restart; conditioning; health tests; gen_data.sh; labs
---

::: resume
This master-level guide teaches the min-entropy assessment of a noise source under **NIST SP 800-90B** [1], with **libsp80090b**, through the **18 cases** built by `lab/gen_data.sh`: healthy sources, design flaws, environmental effects, faults and outputs of deterministic generators. Each physical source is simulated from a model whose entropy is known, so every measurement can be checked against the truth, which no real source allows. Labs 3 and 4 complete the 18 cases with the restart test and conditioning.

The results come down to seven rules. Statistical tests do not see determinism: AES with an all-zero key scores 7.87 bits out of 8. An IID source can be poor: clipping in an ADC destroys 66 % of the entropy without creating any dependency. An estimate can exceed the truth: 3.43 bits measured for 3.33 real bits under a mains tone, hence the model's claim as a ceiling. An average hides the worst case: 3.38 bits credited for a source that ends at 2.38. Finally, only the restart test sees a source that always restarts from the same values. The guide ends with an eight-step qualification protocol, a complete worked example and the explanation of the computation times: test 1 can take an hour per file, because of 10,000 bzip2 compressions that only a failure forces to the end.
:::

# Purpose and how to use this guide {#sec:objet}

This guide is for assessors, cryptographers and engineers who qualify an entropy source. It assumes the SP 800-90B method (NT-01) and the use of the API and labs (NT-02). It does not compare the library with the NIST tool: that conformity is checked separately by `make check` (NT-02). It answers another question: **what should be concluded, and what should be done, in front of each result?**

Table: The four labs and the question each one asks. {#tab:labs widths=10,20,22,48}

| Lab | Test (SP 800-90B) | Program | Question asked |
|---|---|---|---|
| 1 | IID track (§5) | `lab/lab1.sh` | Are the samples independent and identically distributed? If so, H~original~ comes from the most common value estimator (MCV) alone. |
| 2 | Non-IID track (§6) | `lab/lab2.sh` | How much entropy is left against the ten estimators, predictors included? H~I~ = min(H~original~, n × H~bitstring~). |
| 3 | Restart (§3.1.4) | `lab/lab3.sh` | Does the source restart from an unpredictable state at every power-up? |
| 4 | Conditioning (§3.1.5) | `lab/lab4.sh` | How much entropy leaves a conditioning function, and when is the output full entropy? |

The data come from `lab/gen_data.sh all --seed 90`: 10^6^ samples per file, 1,000 × 1,000 for the restart test. The results are in `lab/results/labN_*/<case>.res`, with one summary line per case in `labN.log`.

> **Reading the numbers.** Another seed changes the estimates around the third decimal, and may flip an IID verdict close to the threshold (cases 3 and 17). The conclusions of this guide do not depend on these decimals.

# Seven golden rules {#sec:regles}

Each rule is demonstrated by at least one of the cases of section {@sec:cas}.

1. **Assess the raw noise, never a conditioned output.** A DRBG or a hash passes every test, even with no entropy at all (cases 14 to 18). Capture upstream of any deterministic function.
2. **The physical model first, the measurement second.** The tests are a black-box check. An estimate can exceed the real entropy (case 8): the model's claim H~claim~ is the ceiling.
3. **The IID track must be earned.** It needs a physical argument **and** all three tests passed. A single rejected test forces the non-IID track (case 3), and a capture is not repeated until it passes (case 17).
4. **IID does not mean a good source.** A clipped or biased source is IID and poor (cases 4 and 7). Look at the histogram before the verdicts.
5. **One campaign per condition, keep the worst case.** A file that mixes regimes gives an average, not the worst case (cases 9 to 11). Capture each temperature, voltage and ageing state separately.
6. **Test the start-up.** A source that is healthy in steady state may restart from the same values at every power-up; only the restart test sees it (lab 3).
7. **Apply the minimum rule, then size.** H = min(H~claim~, H~I~, H~r~, H~c~); the seed, the health test cutoffs and the conditioning margin follow from H (section {@sec:protocole}).

# Map of the 18 cases {#sec:carte}

Table {@tab:carte} summarises the 18 cases of labs 1 and 2, in the order of `gen_data.sh --list`. The "Expected H" column is the min-entropy per sample given by the model; lab 1 gives the IID verdict and, in brackets, H~original~, which is only usable if the verdict is IID.

Table: The 18 cases: model, expected entropy and results of labs 1 and 2. {#tab:carte widths=5,15,6,19,11,15,9,20}

| No. | Dataset | Bits | Model (`gen_data.sh`) | Expected H | Lab 1: verdict (H~original~) | Lab 2: H~I~ | Lesson |
|---|---|---|---|---|---|---|---|
| 1 | thermal | 8 | Gaussian noise σ = 20 on an 8-bit ADC | 5.65 | IID (5.618) | 4.093 | Reference; cost of the non-IID track |
| 2 | thermal-lsb | 1 | Least significant bit of the same ADC | 1.00 | IID (0.992) | 0.922 | Keep the bits the noise really moves |
| 3 | ringosc-slow | 1 | Oscillator, 0.5 period of jitter between two reads | ≈ 1 | not IID (0.996) | 0.861 | One rejected test is enough |
| 4 | saturated | 8 | Gain too high (σ = 200), clipped to 0 and 255 | 1.93 | IID (1.921) | 1.727 | IID but poor |
| 5 | lowpass | 8 | First-order filter, α = 0.95 | 3.97 ^a^ | not IID (5.626) | 3.248 | Oversampling |
| 6 | ringosc-fast | 1 | Oscillator read too fast (0.08 period per read) | low | not IID (0.996) | 0.110 | MCV is blind to runs |
| 7 | biased-1bit | 1 | Comparator offset: P(1) = 1/4 | 0.415 | IID (0.414) | 0.250 | A counted bias is not a flaw |
| 8 | interference | 8 | 40-step sine, period 20, over noise σ = 4 | 3.33 ^b^ | not IID (5.726) | 3.430 | The estimate exceeds the truth |
| 9 | aging | 8 | σ falls from 20 to 2 along the file | 5.65 → 2.38 | not IID (4.275) | 3.381 | An average is not the worst case |
| 10 | warmup | 8 | σ = 0.3 over the first 20 %, then σ = 20 | 1.00 then 5.65 | not IID (3.094) | 1.019 | Start-up ruins the whole file |
| 11 | drift | 8 | Uniform 4 bits, then uniform 8 bits | 4 then 8 | not IID (4.884) | 2.440 | Assess each regime |
| 12 | stuck | 8 | One symbol only (`/dev/zero`) | 0 | refused (−3) | refused (−3) | Handle the error codes |
| 13 | replay | 8 | One 4 KiB block repeated | ≈ 0 ^c^ | no verdict (−4) | 1.6 × 10^−6^ | Health tests do not see a replay |
| 14 | urandom | 8 | Kernel DRBG output | out of scope ^d^ | IID (7.882) | 7.323 | Ceiling of the estimators |
| 15 | urandom-4bit | 4 | Same output folded to 4 bits | out of scope | IID (3.972) | 3.539 | Ceiling of the estimators |
| 16 | urandom-1bit | 1 | Same output folded to 1 bit | out of scope | IID (0.994) | 0.857 | Ceiling for a binary source |
| 17 | openssl | 8 | OpenSSL DRBG output | out of scope | not IID (7.896) ^e^ | 6.638 | Spread of the estimates |
| 18 | drbg-zero-seed | 8 | AES-256-CTR, all-zero key and IV | 0 | IID (7.866) | 7.145 | No test sees determinism |

^a^ Entropy conditioned on the past: the filter innovation has a standard deviation of 20 × √(1 − 0.95²) = 6.24. ^b^ The sine is deterministic: an attacker knows its phase. ^c^ The whole file holds at most 4,096 × 8 = 32,768 bits of entropy, those of the first block. ^d^ A DRBG output only has the entropy of its seed; SP 800-90B does not apply. ^e^ False rejection: a fresh `openssl rand` file passes all three tests.

# Analysis of the cases {#sec:cas}

## Reading a lab 2 output

On an n-bit source, `test_non_iid -vv` applies the estimators to the symbols ("Literal") and to the bit string ("Bitstring"). H~original~ is the minimum of the former, H~bitstring~ that of the latter, and H~I~ = min(H~original~, n × H~bitstring~). For case 1: the weakest literal estimator is the t-tuple (5.492), the weakest on bits is compression (0.5116), so H~I~ = min(5.492; 8 × 0.5116) = **4.093**. Identifying the estimator that sets H~I~ is the first step of the diagnosis: it tells which structure was found.

Table: Weakest estimators, and what they reveal. {#tab:estim widths=22,39,39}

| Weakest estimator | Cases | What it found |
|---|---|---|
| Compression (bits) | thermal, aging, drift, interference, urandom, urandom-4bit, drbg-zero-seed | Nothing specific: a cautious bound, which costs a healthy source about 1.4 bits out of 8 |
| MultiMCW (symbols) | openssl (6.638) | A local regularity, due to chance |
| Lag predictor | lowpass (3.976 on the symbols) | The correlation with the previous sample |
| MultiMMC (bits) | lowpass (0.406 × 8 = 3.248) | The transitions of the bit string |
| Longest repeated substring (LRS) | replay, warmup | Long repeated sequences |
| Compression (symbols) | biased-1bit, ringosc-fast, urandom-1bit | The runs and the bias of a binary source |
| Collision | ringosc-slow, saturated (bits) | Close repetitions |
| t-tuple | thermal-lsb | Frequent patterns |

## Healthy sources: the reference (cases 1 to 3)

**Case 1, thermal.** The model gives 5.65 bits; lab 1 concludes IID with 5.618 bits, the 99 % bound of the MCV costing only 0.03 bit. The non-IID track credits 4.093 bits: **1.5 bits less, i.e. 27 %**, the price of having no IID argument. This is the value of a sound physical model: if the model justifies IID, 5.618 bits are credited; otherwise only 4.093.

**Case 2, thermal-lsb.** The least significant bit of a noise of σ = 20 steps is perfectly balanced: IID at 0.992, non-IID at 0.922, on a par with a perfect binary source (case 16: 0.857). Per captured bit, this is much denser than the full byte (4.093 / 8 = 0.51 bit per bit). Per sample, the byte remains richer: keeping only the bits the noise moves is a throughput choice, not an obligation.

**Case 3, ringosc-slow.** The phase accumulates 0.5 period of jitter between two reads: the source is nearly IID. It passes the chi-square and longest repeated substring tests, but the statistic counting runs around the median rejects the hypothesis (C~0~ = 9,998 out of 10,000). The rejection is sound: a bit equals the previous one with probability 0.5021 instead of 0.5, i.e. 4.2 standard deviations over 10^6^ bits, a likely trace of the regular drift of 0.08 period per read. **One rejected test is enough**; the non-IID track credits 0.861 bit.

## Design flaws (cases 4 to 8)

**Case 4, saturated.** A gain that is too high sends 26 % of the samples to 0 and 26 % to 255. The source remains IID (1.921 bits, 1.928 in theory), but has lost **66 %** of the entropy of case 1. Clipping destroys entropy without creating dependency: no independence test can report it. Only the histogram and the comparison with the model show it. Fix: reduce the analogue gain so that the noise fits within the ADC range.

**Case 5, lowpass.** Sampling faster than the noise bandwidth correlates two neighbouring samples (α = 0.95). All three IID tests fail. The Lag predictor finds **exactly** the conditional entropy of the model (3.976 measured for 3.97), and H~I~ = 3.248 stays below. The MCV alone would have given 5.626 bits: crediting H~original~ without an IID verdict would have overestimated the source by 2.4 bits. Fix: lower the sampling rate, or keep one sample out of k, until the autocorrelation is negligible.

**Case 6, ringosc-fast.** The oscillator moves 0.08 period per read: the level only changes about every six samples, and a bit equals the previous one 84 times out of 100. The bits remain balanced: the MCV gives 0.996, but compression finds 0.110. **An unjustified IID track would overestimate by a factor of 9.** Fix: divide the sampling clock so that the jitter accumulated between two reads covers at least one period.

**Case 7, biased-1bit.** P(1) = 1/4: theory gives −log~2~(3/4) = 0.415 bit, lab 1 concludes IID at 0.414. A bias is not a flaw if it is counted: the seed is simply longer. The non-IID track is very cautious here (0.250, compression estimator): with 384 bits to collect, the IID track needs 929 samples and the non-IID track 1,538. A debiaser (von Neumann) placed before the capture point is part of the noise source and must be modelled; placed after it, it is not a vetted conditioning function.

**Case 8, interference.** A 40-step mains tone dominates a noise of σ = 4. For an attacker who knows the phase, the entropy is that of the noise alone: **3.33 bits**. Yet H~I~ = 3.430: **the estimate exceeds the truth by 0.1 bit**, because none of the ten estimators models a sine. This is why the minimum rule exists: H~claim~, from the model, caps the estimate. Fix: decouple the supply and shield; the periodic component could moreover be injected by an attacker.

## Environment and non-stationarity (cases 9 to 11)

**Case 9, aging.** The noise fades from σ = 20 to σ = 2 along the file. H~I~ = 3.381 is an **average**: at the end of the file, the source is only worth 2.38 bits. An APT set for 3.381 bits (W = 512, C = 85) only fires on average below about 2.6 bits: in between, the source delivers less than it is credited with, without any alarm. Good practice: qualify on aged parts and at the extremes of temperature and voltage, one file per condition, and keep the minimum.

**Case 10, warmup.** The first 20 % of the samples take only four values, with 1.00 bit. The longest repeated substring finds them and sets H~I~ = 1.019 **for the whole file**: the start-up phase has cost the steady state 3 bits. Good practice: determine the settling time, discard those samples at start-up, capture the campaign in steady state, and check the start-up with the restart test.

**Case 11, drift.** The distribution changes halfway: 4 uniform bits, then 8. The worst regime is worth 4 bits; H~I~ = 2.440, because the bit estimators see the four most significant bits at zero in half of the file. The result is cautious but describes no real regime. Good practice: find the breaks (mean and histogram per slice) and assess each regime separately.

## Hard faults (cases 12 and 13)

**Case 12, stuck.** One symbol only: both tracks refuse the file with the code `SP80090B_EALPHABET` (−3). An error code is not a zero result to be interpreted later: the caller must stop the qualification. In operation, the RCT detects this fault within a few samples.

**Case 13, replay.** A 4 KiB block is read again and again, like a DMA buffer that is no longer refreshed. The longest repeated substring sets H~I~ = 1.6 × 10^−6^ bit: the fault is detected. The IID track fails the chi-square test, then the longest repeated substring test cannot compute its probability, too small for a `long double`: code `SP80090B_EINTERNAL` (−4), no verdict (limitation L1). A major lesson: **health tests do not see a replay**. The replayed block looks random: no consecutive repetition for the RCT, no abnormal proportion for the APT. The countermeasure lies in the design: frame counter, timestamp or buffer freshness check.

## Conditioned outputs: the trap (cases 14 to 18)

**Case 18, drbg-zero-seed.** AES-256 in counter mode with an all-zero key and IV: the sequence is entirely predictable, its entropy is zero. Yet it passes all three IID tests with 7.866 bits, and the non-IID track grants it 7.145 bits.

>! **No statistical test sees determinism.** The estimators measure predictability through simple models; a cryptographic algorithm fools them all. The assessor must require proof that the samples come from the physical component, upstream of any deterministic function: schematic, capture point and acquisition code.

**Cases 14 to 16, urandom.** These DRBG outputs must not be assessed as sources; they serve as a **reference of perfect randomness** and show the ceiling of the estimators over 10^6^ samples: 7.88 bits on the IID track and about 7.3 on the non-IID track for 8 bits, 3.54 for 4 bits, 0.86 for 1 bit. No real source can be credited beyond that.

**Case 17, openssl.** Two lessons. First, the IID track rejects this file on the periodicity statistic at lag 8 alone (C~2~ = 9,997): this is a **false rejection**, which happens to about 2 to 3 % of IID sources, and a fresh file passes. The standard still imposes the non-IID track on the rejected file: repeating a capture until it passes would amount to choosing one's result. Second, the non-IID track gives 6.638 bits, against 7.323 for urandom: the MultiMCW predictor found a local regularity by chance. **Two perfect sources differ here by 0.7 bit**: an estimate is a random variable, and a margin on H is prudent.

# Restart and conditioning {#sec:complements}

## Lab 3: the restart test

The restart test (§3.1.4) arranges 1,000 power-ups of 1,000 samples in a matrix. It first checks that no value dominates a row or a column (X~max~ ≤ X~cutoff~), then estimates the entropy of the rows (H~r~) and of the columns (H~c~); min(H~r~, H~c~) ≥ H~I~ / 2 is required.

Table: Lab 3: restart test (1,000 × 1,000 samples). {#tab:lab3 widths=17,13,21,23,11,15}

| Dataset | H~I~ (lab 2) | X~max~ / X~cutoff~ | H~r~ / H~c~ | Validated H | Result |
|---|---|---|---|---|---|
| restart-thermal | 4.093 | 39 / 99: passed | 5.306 / 5.306: passed | **4.093** | **Qualified** |
| restart-urandom | 7.323 | 15 / 24: passed | 7.719 / 7.354: passed | **7.323** | **Qualified** |
| restart-warmup | 4.093 | 1,000 / 99: **failed** | — | 0 | **Rejected** |
| restart-frozen | 7.323 | 1,000 / 23: **failed** | — | 0 | **Rejected** |

- **The fault invisible in continuous operation.** restart-warmup has exactly the distribution of case 1 in steady state, but its first 20 samples are the same at every start-up: a whole column is constant (X~max~ = 1,000). No continuous capture reveals it.
- **X~cutoff~ depends on H~I~.** The higher the claimed entropy, the lower the cutoff (99 for 4.09 bits, 23 or 24 for 7.32 bits); it is simulated, hence the difference of one for the same H~I~.
- **H~I~ remains the bound.** H~r~ and H~c~ exceed H~I~ on the healthy sources: the validated value remains H~I~. The test only confirms, or cancels, the entropy of lab 2.
- **Good practice:** real power-ups (supply cut), not software restarts; the source must have reached the state in which it will deliver its first seed.

## Lab 4: conditioning and full entropy

Table: Lab 4: conditioning to a 256-bit output (n~w~ = 256). {#tab:lab4 widths=24,9,10,12,17,28}

| Case | n~in~ | h~in~ | h′ | h~out~ (bits) | Full entropy |
|---|---|---|---|---|---|
| vetted-320-margin | 320 | 320 | — | 256 − 6 × 10^−9^ | **Yes**: 64-bit margin |
| vetted-512-6.4 | 512 | 409.6 | — | 256 | **Yes** |
| vetted-seed-384 | 3,072 | 384 | — | 256 | **Yes** |
| vetted-thermal | 632 | 323.3 | — | 256 | **Yes**: 79 samples of case 1 |
| vetted-256-full | 256 | 256 | — | 251.69 | **No**: no margin |
| vetted-128-short | 256 | 128 | — | 128 | **No**: input too short |
| nonvetted-0.99 | 512 | 409.6 | 0.99 | 253.44 | **No**: ceiling h′ × n~out~ |
| nonvetted-urandom | 512 | 409.6 | 0.915 | 234.34 | **No** |
| nonvetted, 4 others | 512 | 409.6 | 0.857 to 0.908 | 219.38 to 232.38 | **No** |

- **The 64-bit margin.** With a vetted function (SHA-256), 256 bits of input entropy give 251.69 bits of output: the collisions of the function cost 4.3 bits. Full entropy requires h~in~ ≥ n~out~ + 64 = 320 bits [3] [4].
- **The ceiling of a non-vetted function.** h~out~ never exceeds h′ × n~out~, h′ being the entropy per bit measured on the output of the function. Even perfect randomness gives h′ between 0.86 and 0.92 over 10^6^ samples (cases 14 to 18): a non-vetted function never gives full entropy.
- **From lab 2 to lab 4.** vetted-thermal turns the measurement into a number of samples: ⌈320 / 4.093⌉ = 79 samples, i.e. 323.3 bits, for a full-entropy SHA-256 output.

# Qualification protocol {#sec:protocole}

Figure {@fig:protocole} arranges the golden rules into a protocol; the functions named are those of the API (prefix `sp80090b_`).

::: fig protocole Qualification protocol of a noise source with libsp80090b. {#fig:protocole}

1. **Model.** Describe the physical phenomenon, derive H~claim~ from it and state whether IID can be justified.
2. **Capture.** At least 10^6^ consecutive raw samples, at the documented capture point, for each extreme condition (temperature, voltage, ageing); discard the start-up phase.
3. **Choose the track.** IID track only with the model's argument and all three tests passed on the first file; otherwise the non-IID track.
4. **Estimate H~I~.** Check the return code, note the weakest estimator, compare with H~claim~; a large gap in either direction calls the model into question.
5. **Restart.** 1,000 real power-ups × 1,000 samples.
6. **Apply the minimum rule.** H = min(H~claim~, H~I~, H~r~, H~c~), on the worst condition.
7. **Size.** Seed, RCT and APT cutoffs, conditioning input.
8. **Monitor.** RCT and APT on every raw sample, in the product; at the first failure, discard the current seed.

Table: Complete example: sizing for the thermal noise (case 1), H = 4.093 bits. {#tab:exemple widths=34,30,36}

| Quantity | Computation | Value |
|---|---|---|
| H~I~ (lab 2) | min(5.492; 8 × 0.5116) | 4.093 bits |
| Restart (lab 3) | H~r~ = H~c~ = 5.306 ≥ H~I~ / 2 | validated |
| Retained H | min(H~I~, H~r~, H~c~), no IID claim | **4.093 bits** |
| RCT, cutoff C | 1 + ⌈20 / H⌉ | **6** |
| APT, window and cutoff | W = 512 (non-binary source), α = 2^−20^ | **C = 59** |
| Seed of a 256-bit DRBG | ⌈384 / H⌉ | **94 samples** |
| Full-entropy SHA-256 output | ⌈(256 + 64) / H⌉ | **79 samples** |

The APT window is 1,024 samples for a binary source and 512 otherwise (§4.4.2). With an IID claim justified by the model, H would be 5.618 bits, subject to a restart test run with that value.

# Common mistakes {#sec:erreurs}

Table: Common mistakes, the cases that demonstrate them and good practice. {#tab:erreurs widths=33,22,45}

| Mistake | Shown by | Good practice |
|---|---|---|
| Assessing a DRBG, hash or cipher output | Cases 14 to 18 | Capture the raw noise, upstream of any deterministic function |
| Crediting H~original~ without an IID verdict | Cases 5, 6, 8 | H~original~ is only valid if all three tests pass; otherwise H~I~ |
| Repeating the capture until IID passes | Cases 3, 17 | The first file decides; a rejection imposes the non-IID track |
| Judging a source's quality by its IID verdict | Cases 4, 7 | Compare H with the model, look at the histogram |
| Trusting the estimate over the model | Case 8 | Cap with H~claim~ |
| Mixing several conditions in one file | Cases 9, 11 | One file per condition, keep the minimum |
| Including the start-up phase in the campaign | Case 10, restart-warmup | Discard at start-up; restart test |
| Treating an error code as a result | Cases 12, 13 | Stop and diagnose |
| Relying on RCT and APT to detect everything | Cases 9, 13 | Health tests detect hard faults, not a replay or a slow degradation |
| Conditioning without a margin | vetted-256-full | h~in~ ≥ n~out~ + 64 bits |
| Reading three decimals as a certainty | Cases 14, 17 | Keep a margin; several captures if possible |

# Computation time {#sec:temps}

Figure {@fig:performance} gives the run times measured per file of 10^6^ samples on an Intel Core i7-8650U (4 cores, 8 OpenMP threads). Three labs take seconds; test 1 can take an hour per file.

::: fig performance Run times measured per file of 10^6^ samples (Intel Core i7-8650U, 8 threads). {#fig:performance}

## Why test 1 is slow

Test 1 (§5.1) compares 19 statistics of the original data with the same statistics computed on **10,000 random permutations** of the data. If the samples are IID, the order does not matter and the original value ranks among the others; if it is among the 6 largest or the 6 smallest, the IID assumption is rejected. Three factors drive the run time:

1. **Volume.** Each permutation shuffles the whole file (10^6^ samples) and recomputes the statistics still in play: up to 10^10^ samples processed per statistic.
2. **bzip2 compression.** The 19th statistic (§5.1.11) is the length of the bzip2 compression of the values written in decimal, about 4 MB of text for an 8-bit file. One compression takes 0.3 to 0.45 s on one core of the test machine: 10,000 compressions alone amount to nearly an hour of computation, the order of magnitude of the 41 to 57 min measured. The other 18 statistics are single-pass counts, much cheaper.
3. **The asymmetry between pass and fail.** The verdict of a statistic is settled as soon as the counters C~0~ + C~1~ and C~1~ + C~2~ both exceed 5: the library then stops computing it. On an IID source, this happens after a few tens to a few hundred permutations. A **failure**, however, is only decided after all 10,000: all permutations must be seen to assert that the original value stays among the 6 extremes.

Table {@tab:perm} shows this on the lab 1 files: the run time depends less on the verdict than on the compression statistic.

Table: Lab 1: permutations computed and run time, per file. {#tab:perm widths=17,27,17,20,19}

| Dataset | Verdict | Permutations (max.) | Permutations of the compression | Run time |
|---|---|---|---|---|
| thermal | IID | 217 | 82 | 36 s |
| thermal-lsb | IID | 3,750 | 16 | 57 s |
| openssl | not IID (periodicity(8)) | 10,000 | 76 | under 3 min |
| ringosc-slow | not IID (numRunsMedian) | 10,000 | 29 | under 3 min |
| interference | not IID (18 statistics) | 10,000 | 10,000 | 41 min |
| lowpass | not IID (18 statistics) | 10,000 | 10,000 | 51 min |
| warmup | not IID (10 statistics) | 10,000 | 10,000 | 53 min |
| ringosc-fast | not IID (18 statistics) | 10,000 | 10,000 | 57 min |

openssl and ringosc-slow run all 10,000 permutations, but their compression stops before the hundredth: under 3 min. As soon as the compression fails, the file costs nearly an hour.

## Why the other tests are fast

- **Test 2 (5 to 53 s).** The ten estimators read the file once, or sort its suffixes once (longest repeated substring, t-tuple). An 8-bit file is assessed twice, on the symbols and then on the 8 × 10^6^ bits, hence 31 to 53 s against 5 to 7 s for 1 bit. Exception: replay (8 min), because the repetition estimators go through every length up to the longest repetition, here almost the whole file.
- **Test 3 (46 to 74 s).** Two non-IID assessments (rows and columns of the matrix) and the simulation of X~cutoff~.
- **Test 4.** A formula, instantaneous, for a vetted function; 7 to 38 s to measure h′ on the output of a non-vetted function (one test 2 assessment).

## Practical consequences

- **Start with test 2.** In a minute, it gives H~I~, valid in every case.
- **Run test 1 only if the model claims IID.** Without such a claim, the non-IID track applies and test 1 is useless; this is the default case of SP 800-90B.
- **Read the run time as a clue.** A test 1 that lasts more than a few minutes has at least one unsettled statistic, most often the compression: the verdict will almost always be "not IID". The end must still be awaited to conclude.
- **Budget the campaign.** The 18 files need 5 to 6 h in lab 1 (the six files whose compression fails account for most of it), against a quarter of an hour in lab 2: run lab 1 overnight, one lab at a time (limitation C2).
- **Do not shorten the file to save time.** The run time is proportional to the number of samples, but SP 800-90B requires at least 10^6^ samples (§3.1.1); a shorter file distorts the confidence bounds.

# Tool limitations to know {#sec:limites}

Table: Limitations of libsp80090b 2.0.0 that matter to the assessor. {#tab:limites widths=8,46,46}

| Id | Limitation | What to do |
|---|---|---|
| L1 | On a source that replays a long block, the longest repeated substring test of the IID track returns −4 instead of a verdict | Treat the source as not IID; the non-IID track gives the right result (case 13) |
| C2 | Each call starts its own OpenMP team: simultaneous assessments oversubscribe the processor | Run one lab at a time, or set `OMP_NUM_THREADS` |
| M2 | A few allocations of the MultiMMC and LZ78Y estimators and of the chi-square tests are not checked | Provide the memory needed; an abrupt stop means memory ran out |


# Conclusion {#sec:conclusions}

The 18 cases show what the SP 800-90B tests can do: detect dependencies (cases 3, 5, 6, 8), changes of distribution (cases 9 to 11), repetitions (case 13) and dead sources (case 12), and cautiously quantify the remaining entropy. They also show what they cannot do: see determinism (case 18), judge the quality of an IID source (case 4), guarantee a value no higher than the truth (case 8) or describe the worst case of a file that mixes regimes (case 9).

libsp80090b provides the measurements; security comes from the approach around them: a physical model, a capture at the right place and at the worst operating point, a restart test, the minimum rule and health tests set on the retained value. This approach prepares an ESV submission [5]; it does not replace the NIST assessment.

# References

[1] M. Sönmez Turan, E. Barker, J. Kelsey, K. McKay, M. Baish, M. Boyle. *Recommendation for the Entropy Sources Used for Random Bit Generation*. NIST SP 800-90B, January 2018.
[3] E. Barker, J. Kelsey, K. McKay, A. Roginsky, M. Sönmez Turan. *Recommendation for Random Bit Generator (RBG) Constructions*. NIST SP 800-90C, September 2025.
[4] National Institute of Standards and Technology. *Security Requirements for Cryptographic Modules*. FIPS PUB 140-3, 2019; Implementation Guidance, D.K.
[5] NIST CMVP. *Entropy Source Validation (ESV) Program*. ESV program documentation.
[7] J. Kelsey, K. A. McKay, M. Sönmez Turan. *Predictive Models for Min-Entropy Estimation*. CHES 2015, LNCS 9293.

Reference numbers are those of note NT-01.
