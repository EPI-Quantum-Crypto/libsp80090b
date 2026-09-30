# Contributing

- **Bugs and errata:** open an issue with the version, platform, compiler and a
  reproducer (dataset or API call).
- **Pull requests:** run `make check` before submitting; results must stay within the
  selftest tolerance (1e-10) of the NIST reference outputs. Keep the SPDX header of each
  file; changes to NIST-derived code are recorded in `NOTICE`.
- **Documentation:** edit only the Markdown sources `docs/pub/<lang>/NT-xx_<lang>.md`,
  in both languages, then run `make docs`; never edit the generated DOCX or PDF.

Contributions are accepted under the license of the file they change (see `LICENSE`).
