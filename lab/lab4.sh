#!/bin/bash
#
# Lab 4 - conditioning (SP 800-90B Section 3.1.5) with libsp80090b, same shape
# as selftest/selftest: every case is run with test_conditioning, the output
# goes to results/lab4_conditioning/<case>.res and one line is printed:
# h_out and the full-entropy verdict (FIPS 140-3 IG D.K).
#
#   fixed cases       test_conditioning <n_in> <n_out> <nw> <h_in> [h']
#   vetted-thermal    H_I of thermal from Lab 2, input sized for 256 + 64 bits
#   nonvetted-<file>  h' = H_bitstring of test_non_iid -c on data/lab4_conditioning/
#
cd "$(dirname "$0")"
mkdir -p results/lab4_conditioning

run() {		# case, then the arguments of test_conditioning
	local c=$1; shift
	local res=results/lab4_conditioning/$c.res
	echo -n "$c: "
	../test_conditioning "$@" > ${res} 2>&1
	h=$(grep '^h_out =' ${res} | sed 's/.*= //')
	grep -q 'full entropy candidate' ${res} && v="FULL ENTROPY" || v="not full"
	echo "$* -> h_out = ${h:-error}  ${v}"
}

# vetted: n_in n_out nw h_in; non-vetted: + h'
run vetted-256-full   256 256 256 256
run vetted-320-margin 320 256 256 320
run vetted-512-6.4    512 256 256 409.6
run vetted-seed-384   3072 256 256 384
run vetted-128-short  256 256 256 128
run nonvetted-0.99    512 256 256 409.6 0.99

# thermal samples for 256 + 64 bits of entropy into SHA-256
h=$(grep '^Assessed min entropy:' results/lab2_non_iid/thermal.res 2>/dev/null | tail -1 | sed 's/.*: //')
if [ -n "$h" ]; then
	k=$(awk -v h="$h" 'BEGIN { n = 320 / h; printf "%d", (n == int(n)) ? n : int(n) + 1 }')
	run vetted-thermal $((k * 8)) 256 256 $(awk -v h="$h" -v k="$k" 'BEGIN { printf "%.6f", h * k }')
else
	echo "vetted-thermal: no H_I, run lab2.sh first"
fi

for file in data/lab4_conditioning/*.bin; do
	bfile=`basename $file`
	hp=$(../test_non_iid -c ${file} | grep '^H_bitstring =' | sed 's/.*= //')
	[ -n "$hp" ] || { echo "nonvetted-${bfile%.bin}: h' not measured"; continue; }
	run nonvetted-${bfile%.bin} 512 256 256 409.6 $hp
done
