#!/usr/bin/env bash
# Run all seven configurations, then analyse and plot.
set -euo pipefail

BIN="${BIN:-./build/muontracker}"
EVENTS="${EVENTS:-20000}"

if [ ! -x "$BIN" ]; then
  echo "error: '$BIN' not found or not executable." >&2
  echo "build first:  cmake -S . -B build && cmake --build build -j" >&2
  exit 1
fi

mkdir -p results

for N in 4 8 16 32 64 128 256; do
  echo "=============================================="
  echo " N = $N   ($EVENTS events)"
  echo "=============================================="
  "$BIN" "$N" "$EVENTS"
done

echo
echo "running analysis..."
python3 analysis/analyze.py
python3 analysis/plot_results.py
echo "done -- see results/resolution.csv and results/resolution.png"
