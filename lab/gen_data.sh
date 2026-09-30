#!/bin/sh
# gen_data.sh - build the lab datasets: simulated physical noise sources and
#               Linux sources, one case per good practice of SP 800-90B
#
#   lab/gen_data.sh --list                        the datasets, by group
#   lab/gen_data.sh all                           every dataset, into lab/data
#   lab/gen_data.sh thermal                       one dataset -> lab/data/thermal.bin
#   lab/gen_data.sh --thermal -s 2M -o t.bin      same, 2,000,000 samples, into t.bin
#   lab/gen_data.sh design -o /tmp/d              one group (see --list) into /tmp/d
#   lab/gen_data.sh lowpass,saturated -s 200k     several datasets
#   lab/gen_data.sh thermal --sd 5 --seed 7       change a model parameter, fixed seed
#   JENT_SRC=<jitterentropy-library> lab/gen_data.sh real   real raw jitter noise
#
# Every file holds one sample per byte, as the NIST tools and libsp80090b
# expect. The simulated sources are drawn with awk from a seed (--seed): the
# same seed gives the same file. The restart datasets are 1000 restarts x 1000
# samples (SP 800-90B Section 3.1.4, fixed size).
set -eu
export LC_ALL=C                       # tr and awk must work on bytes, not characters

LIB=$(cd "$(dirname "$0")/.." && pwd)
OUT=                                  # default: $LIB/lab/data, whatever the current directory
N=1000000
TYPES=
SEED=
AWK=$(command -v gawk || true)       # gawk: the published datasets and results were drawn with gawk
if [ -z "$AWK" ]; then
	AWK=$(command -v awk)
	echo "warning: gawk not found, using $AWK: the simulated datasets will differ from the published" >&2
	echo "         ones (lab/data.sha256) and the results from lab/results/. Install gawk to reproduce them." >&2
fi

# Model parameters (--name value)
SD=20          # thermal noise standard deviation, in ADC steps
GAIN_SD=200    # saturated: noise far wider than the ADC range
ALPHA=0.95     # lowpass: correlation between two consecutive samples
AMP=40         # interference: amplitude of the mains tone, in ADC steps
I_SD=4         # interference: noise under the tone, in ADC steps
PERIOD=20      # interference: samples per period (50 Hz sampled at 1 kHz)
STEP=0.08      # ringosc: oscillator periods per sample
JITTER=0.02    # ringosc-fast: jitter per sample, in periods
JITTER_SLOW=0.5  # ringosc-slow: jitter accumulated between two slow samples
SD_END=2       # aging: standard deviation at the end of the file
WARMUP=0.2     # warmup: fraction of the file before the source settles
FROZEN=20      # restart-warmup: samples identical after every power-up

GROUPS="physical design environment fault conditioned restart real lab1 lab2 lab3 lab4"
G_physical="thermal thermal-lsb ringosc-slow"
G_design="saturated lowpass ringosc-fast biased-1bit interference"
G_environment="aging warmup drift"
G_fault="stuck replay"
G_conditioned="urandom urandom-4bit urandom-1bit openssl drbg-zero-seed"
G_restart="restart-thermal restart-warmup restart-urandom restart-frozen"
G_real="jent-raw jent-restart"
ALL="$G_physical $G_design $G_environment $G_fault $G_conditioned $G_restart $G_real"
# Groups by lab: the datasets each lab reads
G_lab1="$G_physical $G_design $G_environment $G_fault $G_conditioned jent-raw"
G_lab2=$G_lab1
G_lab3="$G_restart jent-restart"
G_lab4=$G_conditioned

# Lab directories of a dataset (default output, without -o)
labdirs() {
	case $1 in
	restart-*|jent-restart) echo lab3_restart ;;
	urandom|urandom-4bit|urandom-1bit|openssl|drbg-zero-seed) echo lab1_iid lab2_non_iid lab4_conditioning ;;
	*) echo lab1_iid lab2_non_iid ;;
	esac
}

usage() {
	cat <<EOF
Usage: lab/gen_data.sh DATASET|GROUP|all[,...] [-s SIZE] [-o OUT] [--seed S] [model parameters]
       lab/gen_data.sh --DATASET|--GROUP|--all ...          (same, written as an option)
       lab/gen_data.sh --list

  DATASET, GROUP  what to build (see --list); all = every dataset
  -s, --size      samples per file, suffixes k and M allowed (default: 1M;
                  SP 800-90B asks for at least 1M; restart datasets stay 1000 x 1000)
  -o, --out       one dataset: output file; several datasets: output directory.
                  Default: lab/data/<lab>/, the directory of each lab that reads
                  the dataset (lab1_iid, lab2_non_iid, lab3_restart, lab4_conditioning)
      --seed      seed of the simulated sources (default: random, printed)
  -l, --list      list the groups, the datasets and what each one shows

Model parameters: --sd --gain-sd --alpha --amp --i-sd --period --step --jitter
                  --jitter-slow --sd-end --warmup --frozen
EOF
}

list() {
	cat <<'EOF'
GROUPS BY LAB: lab1 and lab2 = raw datasets (physical ... conditioned, jent-raw),
               lab3 = restart datasets, lab4 = conditioned outputs (h' with -n -i)

GROUP        DATASET          BITS  WHAT IT SIMULATES / GOOD PRACTICE IT SHOWS
physical     thermal          8     thermal noise on an 8-bit ADC (Gaussian, --sd): a good raw source, IID
             thermal-lsb      1     least significant bit of the same ADC: keep the bits the noise really moves
             ringosc-slow     1     ring oscillator sampled slowly (jitter accumulates, --jitter-slow): near IID bits
design       saturated        8     ADC gain too high (--gain-sd): clipped to 0 and 255, entropy lost
             lowpass          8     sampling faster than the noise bandwidth (--alpha): correlated, non-IID
             ringosc-fast     1     ring oscillator sampled too fast (--step, --jitter): long runs, non-IID
             biased-1bit      1     comparator offset: P(1) = 1/4, IID but poor
             interference     8     mains tone over weak noise (--amp, --i-sd, --period): periodic
environment  aging            8     noise that fades over the file (--sd to --sd-end): keep the worst case
             warmup           8     source not settled at start (--warmup): discard, test at start-up
             drift            8     law that changes halfway (4 bits, then 8 bits)
fault        stuck            8     dead source: one symbol only (refused by the tools)
             replay           8     one 4 KiB block repeated: a source that replays
conditioned  urandom          8     kernel DRBG output: close to 8 bits, NOT raw noise, never assess it
             urandom-4bit     4     /dev/urandom folded to 4 bits
             urandom-1bit     1     /dev/urandom folded to 1 bit
             openssl          8     OpenSSL DRBG output: same lesson
             drbg-zero-seed   8     AES-256-CTR with an all-zero key: passes every test, zero entropy
restart      restart-thermal  8     1000 power-ups x 1000 samples of thermal noise: passes the restart test
             restart-warmup   8     the first --frozen samples are the same after every power-up: fails
             restart-urandom  8     fresh bytes at every restart
             restart-frozen   8     every restart replays the same row: fails
real         jent-raw         8     jitterentropy raw noise (needs JENT_SRC)
             jent-restart     8     1000 fresh jitterentropy instances x 1000 (needs JENT_SRC)
EOF
}

size() {                              # 1M -> 1000000, 200k -> 200000
	case $1 in
	*[kK]) v=${1%?}; m=1000 ;;
	*[mM]) v=${1%?}; m=1000000 ;;
	*) v=$1; m=1 ;;
	esac
	case $v in ''|*[!0-9]*) echo "invalid size: $1 (e.g. 1M, 200k, 5000)" >&2; exit 2 ;; esac
	[ "$v" -gt 0 ] || { echo "size must be positive" >&2; exit 2; }
	echo $((v * m))
}

num() {                               # check a numeric parameter
	case $2 in ''|*[!0-9.]*|*.*.*) echo "invalid value for $1: $2" >&2; exit 2 ;; esac
	echo "$2"
}

while [ $# -gt 0 ]; do
	case $1 in
	-t|--type|-s|--size|-o|--out|--seed|--sd|--gain-sd|--alpha|--amp|--i-sd|--period|--step|--jitter|--jitter-slow|--sd-end|--warmup|--frozen)
		[ $# -ge 2 ] || { echo "$1 needs a value" >&2; exit 2; } ;;
	esac
	case $1 in
	-t|--type) TYPES=$2; shift 2 ;;
	--type=*) TYPES=${1#*=}; shift ;;
	-s|--size) N=$(size "$2"); shift 2 ;;
	--size=*) N=$(size "${1#*=}"); shift ;;
	-o|--out) OUT=$2; shift 2 ;;
	--seed) SEED=$(num "$1" "$2"); shift 2 ;;
	--sd) SD=$(num "$1" "$2"); shift 2 ;;
	--gain-sd) GAIN_SD=$(num "$1" "$2"); shift 2 ;;
	--alpha) ALPHA=$(num "$1" "$2"); shift 2 ;;
	--amp) AMP=$(num "$1" "$2"); shift 2 ;;
	--i-sd) I_SD=$(num "$1" "$2"); shift 2 ;;
	--period) PERIOD=$(num "$1" "$2"); shift 2 ;;
	--step) STEP=$(num "$1" "$2"); shift 2 ;;
	--jitter) JITTER=$(num "$1" "$2"); shift 2 ;;
	--jitter-slow) JITTER_SLOW=$(num "$1" "$2"); shift 2 ;;
	--sd-end) SD_END=$(num "$1" "$2"); shift 2 ;;
	--warmup) WARMUP=$(num "$1" "$2"); shift 2 ;;
	--frozen) FROZEN=$(num "$1" "$2"); shift 2 ;;
	-l|--list) list; exit 0 ;;
	-h|--help) usage; exit 0 ;;
	--*) case " all $GROUPS $ALL " in                       # --thermal, --design, --all
	     *" ${1#--} "*) TYPES="$TYPES,${1#--}"; shift ;;
	     *) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
	     esac ;;
	-*) echo "unknown option: $1" >&2; usage >&2; exit 2 ;;
	*) TYPES="$TYPES,$1"; shift ;;                          # thermal, design, all
	esac
done
if [ -z "$TYPES" ]; then
	usage; echo; list
	exit 2
fi

# Expand groups; without JENT_SRC, groups and "all" skip the jent-* datasets
TYPES_ASKED=$TYPES
sel=
for t in $(echo "$TYPES" | tr ',' ' '); do
	case $t in
	all) sel="$sel $ALL" ;;
	*) case " $GROUPS " in
	   *" $t "*) eval "sel=\"\$sel \$G_$t\"" ;;
	   *) case " $ALL " in *" $t "*) sel="$sel $t" ;; *) echo "unknown dataset or group: $t (see --list)" >&2; exit 2 ;; esac ;;
	   esac ;;
	esac
done
TYPES=$(echo $sel | tr ' ' '\n' | awk 'NF && !seen[$0]++' | tr '\n' ' ')   # each dataset once
explicit_jent=0                                          # jent-* named, or group "real"
for t in $(echo "$TYPES_ASKED" | tr ',' ' '); do case $t in jent-*|real|--jent-*|--real) explicit_jent=1 ;; esac; done
if [ -z "${JENT_SRC:-}" ] && [ $explicit_jent = 0 ]; then
	case " $TYPES " in *" jent-"*) echo "note: jent-raw and jent-restart skipped (set JENT_SRC=<jitterentropy-library>)" ;; esac
	TYPES=$(echo " $TYPES " | sed 's/ jent-[a-z]*//g')
fi
case " $TYPES " in *" jent-"*)
	[ -n "${JENT_SRC:-}" ] || { echo "jent-raw and jent-restart need JENT_SRC=<jitterentropy-library>" >&2; exit 2; }
	JENT_SRC=$(cd "$JENT_SRC" && pwd) ;;
esac
for t in $TYPES; do
	case $t in restart-*|jent-restart) ;; *) [ "$N" -lt 1000000 ] && { echo "note: $N samples, under the 1,000,000 of SP 800-90B Section 3.1.1"; break; } ;; esac
done
if [ -z "$SEED" ]; then
	SEED=$(od -An -N4 -tu4 /dev/urandom | tr -d ' ')
fi
echo "seed of the simulated sources: $SEED (--seed $SEED gives the same files)"

# One dataset and -o naming a file: write that file; otherwise -o is a directory
FILE=
set -- $TYPES
if [ $# -eq 1 ] && [ -n "$OUT" ] && [ ! -d "$OUT" ]; then
	mkdir -p "$(dirname "$OUT")"
	FILE=$(cd "$(dirname "$OUT")" && pwd)/$(basename "$OUT")
	OUT=$(mktemp -d "$(dirname "$FILE")/.gen_data.XXXXXX")
	trap 'rm -rf "$OUT"' EXIT
fi
BYLAB=
if [ -z "$OUT" ]; then                                  # default: one directory per lab
	BYLAB=$LIB/lab/data
	mkdir -p "$BYLAB"
	OUT=$(mktemp -d "$BYLAB/.gen_data.XXXXXX")
	trap 'rm -rf "$OUT"' EXIT
fi
mkdir -p "$OUT"
cd "$OUT"
OUT=$(pwd)
if [ -n "$BYLAB" ]; then echo "output directory: $BYLAB/<lab>/"
elif [ -z "$FILE" ]; then echo "output directory: $OUT"; fi

# 4-bit and 1-bit uniform samples: fold the byte range in halves with tr
to7='\200-\377'; from7='\000-\177'
fold4() { tr "$to7" "$from7" | tr '\100-\177' '\000-\077' | tr '\040-\077' '\000-\037' | tr '\020-\037' '\000-\017'; }
fold1() { fold4 | tr '\010-\017' '\000-\007' | tr '\004-\007' '\000-\003' | tr '\002-\003' '\000-\001'; }
repeat() { i=0; while [ $i -lt "$2" ]; do cat "$1"; i=$((i + 1)); done; }

# Simulated physical sources: sim <model> <samples> <seed>, bytes on stdout
sim() {
	"$AWK" -v model="$1" -v n="$2" -v seed="$3" -v sd="$SD" -v gsd="$GAIN_SD" -v a="$ALPHA" \
	       -v amp="$AMP" -v isd="$I_SD" -v period="$PERIOD" -v step="$STEP" -v jit="$JITTER" -v jslow="$JITTER_SLOW" \
	       -v sdend="$SD_END" -v warm="$WARMUP" -v frozen="$FROZEN" '
	function gauss(   u, v, r) {                # Box-Muller, two values per draw
		if (have) { have = 0; return keep }
		do u = rand(); while (u == 0)
		v = rand(); r = sqrt(-2 * log(u))
		keep = r * sin(TWO_PI * v); have = 1
		return r * cos(TWO_PI * v)
	}
	function adc(x) {                           # 8-bit ADC: round and clip to 0..255
		if (x < 0) return 0
		if (x > 255) return 255
		return int(x + 0.5) > 255 ? 255 : int(x + 0.5)
	}
	function level(p,   f) {                    # oscillator output: high in the first half-period
		f = p - int(p); if (f < 0) f += 1
		return f < 0.5 ? 1 : 0
	}
	BEGIN {
		TWO_PI = 6.283185307179586; srand(seed)
		if (model == "thermal")
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + sd * gauss())
		else if (model == "thermal-lsb")
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + sd * gauss()) % 2
		else if (model == "saturated")
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + gsd * gauss())
		else if (model == "lowpass") {          # first-order filter: x(t) = a x(t-1) + noise
			c = sqrt(1 - a * a); x = gauss()
			for (i = 0; i < n; i++) { x = a * x + c * gauss(); printf "%c", adc(127.5 + sd * x) }
		} else if (model == "interference")
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + isd * gauss() + amp * sin(TWO_PI * i / period))
		else if (model == "ringosc-fast" || model == "ringosc-slow") {
			j = (model == "ringosc-fast") ? jit : jslow; p = rand()
			for (i = 0; i < n; i++) { p += step + j * gauss(); printf "%c", level(p) }
		} else if (model == "aging")            # standard deviation from sd down to sdend
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + (sd + (sdend - sd) * i / n) * gauss())
		else if (model == "warmup") {           # oscillation not started: tiny noise, then thermal
			w = int(n * warm)
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + (i < w ? 0.3 : sd) * gauss())
		} else if (model == "restart-thermal")  # a fresh power-up every 1000 samples
			for (i = 0; i < n; i++) printf "%c", adc(127.5 + sd * gauss())
		else if (model == "restart-warmup") {   # the same first samples after every power-up
			for (c = 0; c < frozen; c++) start[c] = adc(127.5 + sd * gauss())
			for (r = 0; r < 1000; r++)
				for (c = 0; c < 1000; c++) printf "%c", (c < frozen) ? start[c] : adc(127.5 + sd * gauss())
		}
	}'
}

for t in $TYPES; do
	s=$(( (SEED + $(printf %s "$t" | cksum | cut -d' ' -f1)) % 2147483647 ))   # one seed per dataset name
	case $t in
	thermal|thermal-lsb|saturated|lowpass|interference|ringosc-fast|ringosc-slow|aging|warmup)
	                 sim "$t" "$N" "$s" > "$t.bin" ;;
	biased-1bit)     head -c "$N" /dev/urandom | tr '\000-\277\300-\377' '[\000*192][\001*64]' > biased-1bit.bin ;;
	drift)           { head -c $((N / 2)) /dev/urandom | fold4; head -c $((N - N / 2)) /dev/urandom; } > drift.bin ;;
	stuck)           head -c "$N" /dev/zero > stuck.bin ;;
	replay)          head -c 4096 /dev/urandom > block.tmp
	                 repeat block.tmp $((N / 4096 + 1)) | head -c "$N" > replay.bin
	                 rm -f block.tmp ;;
	urandom)         head -c "$N" /dev/urandom > urandom.bin ;;
	urandom-4bit)    head -c "$N" /dev/urandom | fold4 > urandom-4bit.bin ;;
	urandom-1bit)    head -c "$N" /dev/urandom | fold1 > urandom-1bit.bin ;;
	openssl)         openssl rand -out openssl.bin "$N" ;;
	drbg-zero-seed)  openssl enc -aes-256-ctr -nosalt -in /dev/zero \
	                     -K 0000000000000000000000000000000000000000000000000000000000000000 \
	                     -iv 00000000000000000000000000000000 2>/dev/null | head -c "$N" > drbg-zero-seed.bin ;;
	restart-thermal) sim restart-thermal 1000000 "$s" > restart-thermal.bin ;;
	restart-warmup)  sim restart-warmup 1000000 "$s" > restart-warmup.bin ;;
	restart-urandom) i=0; while [ $i -lt 1000 ]; do head -c 1000 /dev/urandom; i=$((i + 1)); done > restart-urandom.bin ;;
	restart-frozen)  head -c 1000 /dev/urandom > row.tmp
	                 repeat row.tmp 1000 > restart-frozen.bin
	                 rm -f row.tmp ;;
	jent-raw)        make -s -C "$LIB" JENT_SRC="$JENT_SRC" lab/jent_record
	                 "$LIB/lab/jent_record" -q jent-raw.bin "$N" ;;
	jent-restart)    make -s -C "$LIB" JENT_SRC="$JENT_SRC" lab/jent_record
	                 "$LIB/lab/jent_record" -q -r jent-restart.bin ;;
	esac
	f=$t.bin; where=
	if [ -n "$FILE" ]; then
		mv "$f" "$FILE"; f=$FILE
	elif [ -n "$BYLAB" ]; then                           # same bytes in every lab directory
		for d in $(labdirs "$t"); do mkdir -p "$BYLAB/$d"; cp "$f" "$BYLAB/$d/"; where="$where $d"; done
	fi
	printf '%-22s %9d bytes %s\n' "$f" "$(wc -c < "$f")" "$where"
done
