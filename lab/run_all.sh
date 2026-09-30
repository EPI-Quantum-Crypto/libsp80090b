#!/usr/bin/env bash
# run_all.sh - reproduce every result of the technical notes, in the right order.
#
#   lab/run_all.sh               build, NIST selftest, datasets, labs 2, 3 and 4 (about 15 to 30 min)
#   lab/run_all.sh --with-lab1   same, then the IID track (lab 1: several hours)
#
# The results overwrite lab/results/; compare them with the published ones
# with:  git diff --stat lab/results   (git diff lab/results for the details)
set -euo pipefail
cd "$(dirname "$0")/.."
WITH_LAB1=0
[[ "${1:-}" == "--with-lab1" ]] && WITH_LAB1=1
step() { printf '\n=== %s\n' "$*"; }

command -v gawk >/dev/null || echo "warning: gawk is missing, the datasets will differ from the published ones"

step "1/6 build";                          make
step "2/6 NIST selftest and API self-test"; make check
step "3/6 datasets (seed 90)";             sh lab/gen_data.sh all --seed 90
if (cd lab && sha256sum -c --quiet data.sha256); then
	echo "datasets identical to the published ones (urandom and openssl are random by design)"
else
	echo "warning: some datasets differ from the published ones (see above)"
fi
step "4/6 lab 2: non-IID track";           bash lab/lab2.sh
step "5/6 lab 3: restart test";            bash lab/lab3.sh
step "6/6 lab 4: conditioning";            bash lab/lab4.sh
if [[ $WITH_LAB1 == 1 ]]; then
	step "lab 1: IID track (long)";        bash lab/lab1.sh
fi
step "done"
echo "Compare with the published results:  git diff --stat lab/results"
