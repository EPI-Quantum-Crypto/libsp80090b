# libsp80090b — NIST SP 800-90B entropy assessment as a C library

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/EPI-Quantum-Crypto/libsp80090b?quickstart=1)

[![CI](https://github.com/EPI-Quantum-Crypto/libsp80090b/actions/workflows/ci.yml/badge.svg)](https://github.com/EPI-Quantum-Crypto/libsp80090b/actions/workflows/ci.yml)
[![NIST selftest](https://img.shields.io/badge/NIST%20selftest-max%20gap%207.2e--14-brightgreen)](#conformity)
[![Release](https://img.shields.io/github/v/release/EPI-Quantum-Crypto/libsp80090b)](https://github.com/EPI-Quantum-Crypto/libsp80090b/releases/latest)
[![Code: MIT + NIST notice](https://img.shields.io/badge/code-MIT%20%2B%20NIST%20notice-blue)](LICENSE)
[![Docs: CC BY 4.0](https://img.shields.io/badge/docs-CC%20BY%204.0-lightgrey)](LICENSES/CC-BY-4.0.txt)

*Version française : [README.fr.md](README.fr.md)*

C99 port of the NIST reference tool
[SP800-90B_EntropyAssessment](https://github.com/usnistgov/SP800-90B_EntropyAssessment) 1.1.8,
as a library that assesses an **in-memory buffer**: no file to write, no executable to
launch, no text output to parse.

It measures the **min-entropy** of a raw noise source, in bits per sample, with the four
tests of NIST SP 800-90B. This value sets the number of samples to collect for a DRBG seed,
the cutoffs of the RCT and APT health tests, and the entropy claim of an ESV submission.

## Documentation

Each document exists in French (`_fr`) and in English (`_en`).

| Ref. | Document | Audience | Files |
|---|---|---|---|
| **NT-01** | Technical note: method, the four tests, good practice, hybrid generator | Crypto engineers, managers | [`en`](docs/pub/en/NT-01_libsp80090b_v5.1_en.pdf) · [`fr`](docs/pub/fr/NT-01_libsp80090b_v5.1_fr.pdf) |
| **NT-02** | User guide: installation, C API, labs, integration code | Developers, bench operators | [`en`](docs/pub/en/NT-02_libsp80090b_v3.1_en.pdf) · [`fr`](docs/pub/fr/NT-02_libsp80090b_v3.1_fr.pdf) |
| **NT-03** | Good practice guide: entropy assessment in practice through the 18 lab cases, restart, conditioning, qualification protocol | Assessors, crypto engineers | [`en`](docs/pub/en/NT-03_libsp80090b_v4.1_en.pdf) · [`fr`](docs/pub/fr/NT-03_libsp80090b_v4.1_fr.pdf) |

Sources, house style and update rules: [`docs/README.md`](docs/README.md). Changes:
[`CHANGELOG.md`](CHANGELOG.md).

## Conformity

| Test (SP 800-90B) | Function | Against the NIST tool (`make check`, NT-02) |
|---|---|---|
| 1. IID assumption (§5, §6.3.1) | `sp80090b_iid_evaluate_ex()` | Every deterministic value bit-for-bit identical to `ea_iid`; same permutation verdicts |
| 2. Non-IID estimators (§6.3) | `sp80090b_non_iid_evaluate_ex()` | Identical to `ea_non_iid` (largest gap 7.2 × 10⁻¹⁴ against the NIST reference outputs), 2 to 3 times faster |
| 3. Restart (§3.1.4) | `sp80090b_restart_evaluate()` | Same H_r, H_c and verdicts as `ea_restart`; simulated cutoff within ±1 |
| 4. Conditioning (§3.1.5) | `sp80090b_conditioning_evaluate()` | Same values as `ea_conditioning` to within 1 ulp of `long double` |

The statistical estimate is a sanity check of the model of the source, not a replacement
for it (NT-01, "Model first, then measure").

## Quick start

```bash
sudo apt-get install -y build-essential libdivsufsort-dev libbz2-dev openssl gawk
make                                   # libraries, labs, test programs
./test_selftest                        # self-test against NIST values
make check                             # NIST selftest (11 reference datasets) + test_selftest
```

```c
#include "sp80090b.h"

/* buf: at least 1,000,000 RAW samples, one per byte */
sp80090b_options_t o = sp80090b_default_options();
sp80090b_result_t r;

o.bits_per_symbol = 8;
if (sp80090b_non_iid_evaluate_ex(buf, len, &o, &r) != SP80090B_OK || r.warnings)
    return -1;
printf("H_I = %.5f bits per sample\n", r.h_assessed);
```

```bash
gcc -std=c99 -O2 -fopenmp main.c -I./include libsp80090b.a \
    -ldivsufsort -ldivsufsort64 -lbz2 -lm -o my_app
```

The four tests can also be run with the labs: `lab/gen_data.sh` builds 24 datasets, from
good sources to typical defects (saturation, drift, replay, DRBG output...), and
`lab/lab1.sh` to `lab/lab4.sh` run tests 1 to 4 on them, one line per case. The programs
`lab/lab_iid`, `lab_non_iid`, `lab_restart` and `lab_conditioning` take the command line of
the NIST tools. Syntax and examples in NT-02, results and their interpretation in NT-03.

## Rules of use

1. Assess the **raw noise**, never `/dev/urandom`, `getrandom()` or a hashed output: they get close to 8 bits whatever the source.
2. At least **1,000,000 samples**, collected under operating conditions (`SP80090B_WARN_FEW_SAMPLES` otherwise).
3. The IID track only for a source **claimed and justified** IID by its designer; otherwise the non-IID track.
4. Keep the **minimum**: `h_assessed`, then `min(H_r, H_c, H_I)` after the restart test, and never more than the design claim.
5. Size the seed with this value: ⌈384 / H⌉ raw samples at instantiation and ⌈256 / H⌉ at reseed for a 256-bit strength.
6. For a FIPS 140-3 (ESV) submission, the data are assessed by NIST; the library is for development, qualification and measurement campaigns.

## Directory tree

```text
SP800-90B_lib/
├── README.md, README.fr.md, CHANGELOG.md, CHANGELOG.fr.md
├── Makefile                  make, make lib, make lab, make check, make docs, make clean
├── include/sp80090b.h        public API: the only header to include
├── src/                      sp80090b_api.c and test programs
├── lab/                      gen_data.sh, lab1.sh .. lab4.sh, data/, results/, lab_* programs
├── bin/                      NIST reference datasets
├── selftest/                 copy of the NIST selftest, compared with its reference outputs
└── docs/                     NT-01, NT-02, NT-03 (fr, en): sources, style, publications
```

## Reproduce the results

Website: **https://epi-quantum-crypto.github.io/libsp80090b/**

- **In your browser:** click *Open in GitHub Codespaces* above; the environment builds the library.
  The codespace runs on your own GitHub account and its free monthly allowance.
- **Everything, in order:** `lab/run_all.sh` (add `--with-lab1` for the IID track, several hours).
- **Step by step, with timings and expected values:** [`EXPERIMENTS.md`](EXPERIMENTS.md).
- **Check:** `git diff --stat lab/results` compares your results with the published ones.

## Need more depth? Contact the maintainers

The notes stay at the level needed to understand and apply the method. For **more specific or
deeper technical details** (derivations, raw lab results, datasets, integration on a given
platform, preparation of an ESV submission):

| Need | How |
|---|---|
| Technical question, deeper detail | [Technical detail request](https://github.com/EPI-Quantum-Crypto/libsp80090b/issues/new?template=technical-question.yml) (public) |
| Error in a note or in the code | [Erratum or bug report](https://github.com/EPI-Quantum-Crypto/libsp80090b/issues/new?template=erratum.yml) |
| Security issue in the library | Private advisory, see [`SECURITY.md`](SECURITY.md) |
| Private request (data, collaboration) | **epi-quantum-crypto@proton.me**, see [`CONTACT.md`](CONTACT.md) |

## License and acknowledgment

Derived from the NIST reference tool SP800-90B_EntropyAssessment 1.1.8; NIST is acknowledged
as the source of the original software ([`NOTICE`](NOTICE)). Not affiliated with or endorsed by
NIST, and not a substitute for an ESV assessment. Code: NIST software notice and MIT License;
documentation: CC BY 4.0 — see [`LICENSE`](LICENSE).

## How to cite

Use the **Cite this repository** button ([`CITATION.cff`](CITATION.cff)).
