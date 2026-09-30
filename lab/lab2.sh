#!/bin/bash
#
# Lab 2 - non-IID track (SP 800-90B Section 6.3) with libsp80090b, same shape
# as selftest/selftest: every dataset of data/lab2_non_iid/ is run with
# test_non_iid -vv, the output goes to results/lab2_non_iid/<case>.res and one
# line is printed: H_original, H_bitstring and the assessed H_I.
# H_I feeds Lab 3 (restart) and Lab 4 (vetted-thermal).
#
# About 3 s per 1-bit file and 20 to 30 s per 8-bit file of 10^6 samples.
#
cd "$(dirname "$0")"
mkdir -p results/lab2_non_iid

# Symbol width of each dataset of gen_data.sh (inferred otherwise)
width() {
	case $1 in
	thermal-lsb|ringosc-slow|ringosc-fast|biased-1bit|urandom-1bit) echo 1 ;;
	urandom-4bit) echo 4 ;;
	*) echo 8 ;;
	esac
}

for file in data/lab2_non_iid/*.bin; do
	bfile=`basename $file`
	res=results/lab2_non_iid/${bfile/bin/res}
	echo -n "${bfile}: "
	../test_non_iid -vv ${file} $(width ${bfile%.bin}) > ${res} 2>&1
	g() { grep "^$1" ${res} | tail -1 | sed 's/.*[=:] //'; }
	h=$(g 'Assessed min entropy:')
	if [ -n "$h" ]; then
		echo "H_original = $(g 'H_original =')  H_bitstring = $(g 'H_bitstring =')  H_I = $h"
	else
		echo "refused: $(grep -m1 '^Error' ${res})"
	fi
done
