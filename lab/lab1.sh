#!/bin/bash
#
# Lab 1 - IID track (SP 800-90B Section 5) with libsp80090b, same shape as
# selftest/selftest: every dataset of data/lab1_iid/ is run with test_iid -vv,
# the output goes to results/lab1_iid/<case>.res and one line is printed:
# the three IID tests and the assessed min-entropy.
#
# A source that is not IID runs all 10,000 permutations: allow several
# minutes to about 45 minutes per 10^6-sample file.
#
cd "$(dirname "$0")"
mkdir -p results/lab1_iid

# Symbol width of each dataset of gen_data.sh (inferred otherwise)
width() {
	case $1 in
	thermal-lsb|ringosc-slow|ringosc-fast|biased-1bit|urandom-1bit) echo 1 ;;
	urandom-4bit) echo 4 ;;
	*) echo 8 ;;
	esac
}

for file in data/lab1_iid/*.bin; do
	bfile=`basename $file`
	res=results/lab1_iid/${bfile/bin/res}
	echo -n "${bfile}: "
	../test_iid -vv ${file} $(width ${bfile%.bin}) > ${res} 2>&1
	v() { grep "^$1" ${res} | grep -q ': passed' && echo pass || echo FAIL; }
	h=$(grep '^H_original:' ${res} | sed 's/H_original: //; s/ bits.*//')
	grep -q '^IID assumption holds' ${res} && c="IID" || c="not IID"
	[ -n "$h" ] || c="refused: $(grep '^Error' ${res})"
	echo "chi2 $(v Chi-square)  LRS $(v LRS)  perm $(v Permutation)  H_original = ${h:--}  ${c}"
done
