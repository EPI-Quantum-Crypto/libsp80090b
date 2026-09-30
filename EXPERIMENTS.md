# Reproducing the experiments

Every number of the technical notes NT-01 to NT-03 comes from the commands below.
Timings are for 10^6-sample files on a 4-core laptop (Intel Core i7-8650U); a
2-core codespace is about twice as slow.

## 0. Where to run

| Option | How | Notes |
|---|---|---|
| Browser | *Open in GitHub Codespaces* on the README | Nothing to install; uses your GitHub account's free monthly allowance. Stop the codespace when done. |
| Linux (Ubuntu/Debian) | commands below | Recommended for lab 1 (several hours). |
| Windows / macOS | WSL 2 (Ubuntu), or Docker with the `.devcontainer` | Native builds are not supported. |

## 1. Build and verify (about 5 minutes)

```bash
sudo apt-get install -y build-essential libdivsufsort-dev libbz2-dev openssl gawk
git clone https://github.com/EPI-Quantum-Crypto/libsp80090b.git
cd libsp80090b
make                 # libsp80090b.a, libsp80090b.so and the test programs
make check           # NIST selftest on the 11 NIST datasets + API self-test
```

Expected: 11 lines `Maximum delta: ...`, all below 1e-10 (the largest is about 7.2e-14,
`truerand_8bit.bin`), then `SUCCESS: 0 failure(s)`.

## 2. Generate the datasets (about 10 seconds)

```bash
lab/gen_data.sh --list               # the 22 datasets and what each one shows
lab/gen_data.sh all --seed 90        # all of them, into lab/data/<lab>/
cd lab && sha256sum -c data.sha256 && cd ..   # identical to the published datasets?
```

**gawk is required** to get the published datasets: other awk implementations draw
different random numbers, and the results then differ in the second decimal.
`urandom`, `urandom-4bit`, `urandom-1bit` and `openssl` are random by design and are not
in `data.sha256`: their results change slightly at each run.

## 3. Lab 2 — non-IID track (5 to 15 minutes): run it first

```bash
lab/lab2.sh
```

One line per dataset with H_original, H_bitstring and H_I. Published values (NT-03, Table 2):

| Dataset | H_I | Dataset | H_I |
|---|---|---|---|
| thermal | 4.0927 | aging | 3.3811 |
| thermal-lsb | 0.9216 | warmup | 1.0189 |
| ringosc-slow | 0.8608 | drift | 2.4404 |
| saturated | 1.7272 | stuck | refused (−3) |
| lowpass | 3.2484 | replay | 1.6 × 10⁻⁶ |
| ringosc-fast | 0.1096 | drbg-zero-seed | 7.1450 |
| biased-1bit | 0.2498 | urandom | ≈ 7.32 (random) |
| interference | 3.4304 | openssl | ≈ 6.6 to 7.3 (random) |

## 4. Lab 3 — restart test (3 to 6 minutes, needs lab 2)

```bash
lab/lab3.sh
```

Expected: `restart-thermal` and `restart-urandom` pass (X_max 39 / X_cutoff 99, and
15 / 24); `restart-warmup` and `restart-frozen` fail the sanity check with X_max = 1000.

## 5. Lab 4 — conditioning (under a minute, needs lab 2)

```bash
lab/lab4.sh
```

Expected: `vetted-256-full` gives h_out = 251.6898 (not full); `vetted-320-margin`,
`vetted-512-6.4`, `vetted-seed-384` and `vetted-thermal` (79 thermal samples) give full
entropy; every non-vetted case stays below 256.

## 6. Lab 1 — IID track (several hours: run it last, overnight)

```bash
lab/lab1.sh
# or one file:  cd lab && ../test_iid -vv data/lab1_iid/thermal.bin 8
```

IID sources finish in under a minute; a source whose compression statistic fails runs all
10,000 permutations and takes about an hour (NT-03 §8). The permutations are random:
a verdict close to the threshold (ringosc-slow, openssl) can change from one run to the next.

## 7. Everything in one command

```bash
lab/run_all.sh              # steps 1 to 5
lab/run_all.sh --with-lab1  # and lab 1
```

## 8. Compare with the published results

```bash
git diff --stat lab/results      # which result files changed
git diff lab/results/lab2_non_iid/thermal.res
```

With gawk and seed 90, the deterministic datasets give byte-identical `.res` files except
for the file path on the first line.

## 9. Your own noise source

```bash
cd lab
../test_non_iid -vv my_source.bin 8        # H_I; the file: >= 1,000,000 raw samples, one per byte
../test_restart my_restart.bin <H_I> 8 -v  # 1,000 power-ups x 1,000 samples
```

Then follow the qualification protocol of NT-03 §6 and size the generator with NT-01 §6.
Real jitter noise: `JENT_SRC=<path to jitterentropy-library> lab/gen_data.sh real`.
