#!/bin/sh
# Reproduce the headline numbers from the report and from the committed figures.
# Any mismatch exits non-zero.
set -e

BIN=${BIN:-bin}
fail=0

report() {
    if [ "$2" = "$3" ]; then
        printf '  PASS  %-34s %s\n' "$1" "$2"
    else
        printf '  FAIL  %-34s got %s, expected %s\n' "$1" "$2" "$3"
        fail=1
    fi
}

echo "Rossler system (a=0.1, b=0.1, c=1, x0=y0=z0=0)"

got=$("$BIN/integratore" 0.1 0.1 1 0 0 0 0.01 100 | tail -1 \
      | awk '{printf "%s %s %s", $2, $3, $4}')
report "x,y,z at t=100 (dt=0.01)" "$got" \
       "-0.7866364173 0.3313694622 0.0901477239"

got=$("$BIN/duratatransiente" | awk -F': ' '/transient duration of the system/{printf "%.2f", $2}')
report "transient duration of the system" "$got" "234.11"

got=$("$BIN/periodo" | awk -F': ' '/period averaged/{printf "%.5f", $2}')
report "period averaged over x,y,z" "$got" "5.84817"

echo
echo "Random walks (diffusion law <x^2> = t)"

got=$("$BIN/rw" 5000 500 20260829 \
      | awk '$1=="500"{s+=$4;n++} END{printf "%.2f", (s/n)/500}')
report "1D <x^2>/t at t=500, n=5000" \
       "$(awk -v r="$got" 'BEGIN{print (r>0.94 && r<1.06) ? "within 6%" : r}')" \
       "within 6%"

got=$("$BIN/rw2d" 400 100000 31337 \
      | awk 'NF>=6{s+=($2*$2+$3*$3);n++} END{printf "%.2f", (s/n)/100000}')
report "2D <r^2>/t at t=1e5, n=400" \
       "$(awk -v r="$got" 'BEGIN{print (r>0.85 && r<1.15) ? "within 15%" : r}')" \
       "within 15%"

echo
echo "Lattice gas (32x32, hard-core exclusion)"

out=$("$BIN/gas" 32 0.6 20000 4242)

# a move only relocates a particle, so the count can never change
got=$(echo "$out" | awk '/^particles:/{print ($2 == $6) ? "conserved" : "LOST"}')
report "particle count over 20000 moves" "$got" "conserved"

# a move is accepted when the target site is empty, so the acceptance rate
# has to track the fraction of empty sites, 1 - density
got=$(echo "$out" | awk '/^accepted moves:/{gsub(/[(%)]/,"",$NF); print $NF}')
report "acceptance at density 0.6" \
       "$(awk -v r="$got" 'BEGIN{print (r>34 && r<46) ? "tracks 1-density" : r"%"}')" \
       "tracks 1-density"

echo
if [ "$fail" -eq 0 ]; then
    echo "all checks passed"
else
    echo "SOME CHECKS FAILED"
fi
exit "$fail"
