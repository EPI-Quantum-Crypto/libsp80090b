#!/bin/bash
#
# Lab 3 - restart test (SP 800-90B Section 3.1.4) with libsp80090b, same shape
# as selftest/selftest: every dataset of data/lab3_restart/ (1000 restarts x
# 1000 samples) is run with test_restart -v, the output goes to
# results/lab3_restart/<case>.res and one line is printed.
#
# The test needs H_I of the same source in steady state: it is read from the
# Lab 2 result of the matching raw dataset (restart-thermal, restart-warmup ->
# thermal; restart-urandom, restart-frozen -> urandom; jent-restart ->
# jent-raw), so run lab2.sh first.
#
# About 40 s to 1 min 30 s per dataset.
#
cd "$(dirname "$0")"
mkdir -p results/lab3_restart

source_of() {
	case $1 in
	restart-thermal|restart-warmup) echo thermal ;;
	restart-urandom|restart-frozen) echo urandom ;;
	jent-restart) echo jent-raw ;;
	esac
}

for file in data/lab3_restart/*.bin; do
	bfile=`basename $file`
	res=results/lab3_restart/${bfile/bin/res}
	echo -n "${bfile}: "
	src=$(source_of ${bfile%.bin})
	h=$(grep '^Assessed min entropy:' results/lab2_non_iid/${src:-none}.res 2>/dev/null | tail -1 | sed 's/.*: //')
	if [ -z "$h" ]; then
		echo "no H_I: run lab2.sh on ${src:-its raw dataset} first"
		continue
	fi
	../test_restart ${file} $h 8 -v > ${res} 2>&1
	echo "H_I = $h  $(grep -E '^(Sanity|Validation|Entropy after)' ${res} | sed 's/ (3.1.4.[23])//' | tr '\n' ' ')"
done
