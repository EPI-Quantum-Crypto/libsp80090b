# Security policy

libsp80090b measures entropy; a defect could make a weak source look acceptable.
Please report such problems privately.

## Reporting
- Use GitHub's **Report a vulnerability** button (Security tab → Advisories),
  or e-mail **epi-quantum-crypto@proton.me** with "[security]" in the subject.
- Include the version or commit, the platform and compiler, the command or API call,
  and, if possible, a dataset that reproduces the issue.

We aim to acknowledge reports within 7 days and to publish a fix and an advisory
once users can update.

## In scope
- Estimates that differ from the NIST reference tool beyond the selftest tolerance.
- Crashes, memory-safety bugs or undefined behaviour on valid or crafted inputs.
- Errors in the integration code of NT-02 (health tests, seed handling).

## Out of scope
- The SP 800-90B specification itself (contact NIST).
- Entropy claims of third-party noise sources.
