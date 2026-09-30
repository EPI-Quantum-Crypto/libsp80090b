# Changelog

The library follows semantic versioning. Each note keeps its own version number.

## [2.0.0] — 2026-09-30 — first public release

### Library
- C99 library port of NIST SP800-90B_EntropyAssessment 1.1.8: four API functions
  (IID, non-IID, restart, conditioning), return codes and warnings.
- `make check`: NIST selftest on the 11 reference datasets (tolerance 1e-10,
  largest gap 7.2e-14) and API self-test.
- SPDX headers, NIST notice, `NOTICE` listing the changes to the NIST code.

### Documentation
- NT-01 v5.1, NT-02 v3.1, NT-03 v4.1 (English and French), public release.
- Errata applied: APT window per SP 800-90B §4.4.2 (W = 1024 for a binary source,
  512 otherwise) and recomputed cut-offs (449, 59, 589); 0.999·n_out cap for
  non-vetted conditioning; full-entropy criterion clarified; H_I capped by the
  claim before the restart test; every warning invalidates a qualification;
  seed buffer wiped with a non-removable wipe; cross-references harmonised.
- The integration code of NT-02 §5 compiles with `-Wall -Wextra` against the API.
