#!/usr/bin/env python3
"""
Split-half angular-resolution analysis.

Reads  results/hits_N<N>.csv  (written by the Geant4 app)
Writes results/resolution.csv

Method
------
A single straight-line fit through all N layers cannot tell you its own
error. So each muon track is cut in half:

    top half     layers 0 .. N/2-1      -> fit -> theta_top
    bottom half  layers N/2 .. N-1      -> fit -> theta_bottom
    dtheta = theta_top - theta_bottom

dtheta is the difference of two INDEPENDENT estimates of the same angle,
so its spread is sqrt(2) times the spread of one estimate:

    sigma_theta = sigma(dtheta) / sqrt(2)

and the uncertainty on that width, for n events, is

    err(sigma_theta) = sigma_theta / sqrt(2 * (n - 1))
"""

import argparse
import glob
import os
import re
import sys

import numpy as np

# ---------------------------------------------------------------------
# Detector readout resolution applied to the TRUE crossing positions.
# Geant4 gives the exact crossing point; a real detector does not. This
# is the Gaussian smearing that stands in for finite strip pitch and
# photodetector resolution.
#
# >>> Set this to the value quoted in your paper. <<<
DEFAULT_SIGMA_HIT_MM = 0.5
# ---------------------------------------------------------------------


def gaussian_sigma(values, n_iter=3, n_sigma=3.0):
    """Width of the central Gaussian core, with iterative outlier clipping.

    Plain RMS is pulled up by the rare hard-scatter tail. Clipping at
    +-3 sigma a few times converges on the core width, which is what the
    'bell curve' in the write-up refers to.
    """
    v = np.asarray(values, dtype=float)
    v = v[np.isfinite(v)]
    if v.size < 3:
        return np.nan, 0
    lo, hi = -np.inf, np.inf
    for _ in range(n_iter):
        sel = v[(v > lo) & (v < hi)]
        if sel.size < 3:
            break
        mu, sd = sel.mean(), sel.std(ddof=1)
        lo, hi = mu - n_sigma * sd, mu + n_sigma * sd
    sel = v[(v > lo) & (v < hi)]
    if sel.size < 3:
        return float(np.std(v, ddof=1)), int(v.size)
    return float(np.std(sel, ddof=1)), int(sel.size)


def slope_weights(z):
    """Weights w such that  slope = w . x  is the least-squares slope.

    Valid because every event shares the same z positions, so the
    normal-equation denominator is a constant.
    """
    z = np.asarray(z, dtype=float)
    dz = z - z.mean()
    denom = np.sum(dz * dz)
    if denom <= 0:
        raise ValueError("degenerate z positions")
    return dz / denom


def analyse_file(path, sigma_hit_mm, rng, verbose=True):
    data = np.loadtxt(path, delimiter=",", skiprows=1)
    if data.ndim != 2 or data.shape[0] == 0:
        raise ValueError(f"{path}: no rows")

    event = data[:, 0].astype(np.int64)
    layer = data[:, 1].astype(np.int32)
    x_mm = data[:, 2]
    z_mm = data[:, 4]

    n_layers = int(layer.max()) + 1
    if n_layers < 4:
        raise ValueError(f"{path}: need N >= 4 to split into halves")

    # Rows arrive grouped by event and sorted by layer (EventAction sorts
    # them), and only complete events were written -- so the table
    # reshapes cleanly.
    if x_mm.size % n_layers != 0:
        raise ValueError(f"{path}: row count {x_mm.size} not a multiple of N={n_layers}")

    n_events = x_mm.size // n_layers
    X = x_mm.reshape(n_events, n_layers)
    Z = z_mm.reshape(n_events, n_layers)

    # Sanity: every event must share the same z profile.
    if not np.allclose(Z, Z[0], atol=1e-6):
        raise ValueError(f"{path}: inconsistent layer z positions between events")
    z = Z[0]

    # Check the reshape really did line up event boundaries.
    ev = event.reshape(n_events, n_layers)
    if not np.all(ev == ev[:, :1]):
        raise ValueError(f"{path}: event IDs not contiguous -- cannot reshape")

    # ---- apply readout resolution ----------------------------------
    Xs = X + rng.normal(0.0, sigma_hit_mm, size=X.shape)

    # ---- split and fit ---------------------------------------------
    half = n_layers // 2
    w_top = slope_weights(z[:half])
    w_bot = slope_weights(z[half:])

    slope_top = Xs[:, :half] @ w_top
    slope_bot = Xs[:, half:] @ w_bot

    theta_top = np.arctan(slope_top)
    theta_bot = np.arctan(slope_bot)
    dtheta = theta_top - theta_bot            # radians

    sd_dtheta, n_used = gaussian_sigma(dtheta)
    sigma_theta = sd_dtheta / np.sqrt(2.0)
    err = sigma_theta / np.sqrt(2.0 * (n_used - 1)) if n_used > 1 else np.nan

    rms_sigma_theta = float(np.std(dtheta, ddof=1)) / np.sqrt(2.0)

    if verbose:
        print(f"  N = {n_layers:>4}  events = {n_events:>6}  "
              f"core = {n_used:>6}  "
              f"sigma_theta = {sigma_theta*1e3:7.3f} +- {err*1e3:.3f} mrad "
              f"(raw RMS {rms_sigma_theta*1e3:.3f})")

    return {
        "N": n_layers,
        "events": n_events,
        "core_events": n_used,
        "sigma_theta_mrad": sigma_theta * 1e3,
        "err_mrad": err * 1e3,
        "rms_sigma_theta_mrad": rms_sigma_theta * 1e3,
        "scint_cm": n_layers * 0.1,      # 1 mm per plate
        "spacing_cm": 100.0 / (n_layers - 1),
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--indir", default="results",
                    help="directory holding hits_N*.csv (default: results)")
    ap.add_argument("--out", default=None,
                    help="output CSV (default: <indir>/resolution.csv)")
    ap.add_argument("--sigma-hit", type=float, default=DEFAULT_SIGMA_HIT_MM,
                    help=f"readout resolution in mm (default {DEFAULT_SIGMA_HIT_MM})")
    ap.add_argument("--seed", type=int, default=20240,
                    help="RNG seed for the smearing")
    args = ap.parse_args()

    files = sorted(glob.glob(os.path.join(args.indir, "hits_N*.csv")),
                   key=lambda p: int(re.search(r"hits_N(\d+)\.csv", p).group(1)))
    if not files:
        print(f"no hits_N*.csv found in '{args.indir}/'", file=sys.stderr)
        print("run the simulation first, e.g.  ./muontracker 64 20000", file=sys.stderr)
        return 1

    rng = np.random.default_rng(args.seed)

    print(f"readout resolution sigma_hit = {args.sigma_hit} mm\n")
    rows = []
    for f in files:
        try:
            rows.append(analyse_file(f, args.sigma_hit, rng))
        except Exception as exc:
            print(f"  !! {f}: {exc}", file=sys.stderr)

    if not rows:
        return 1

    rows.sort(key=lambda r: r["N"])

    # change from the previous configuration, and its significance
    for i, r in enumerate(rows):
        if i == 0:
            r["delta_mrad"] = float("nan")
            r["significance"] = float("nan")
        else:
            prev = rows[i - 1]
            d = r["sigma_theta_mrad"] - prev["sigma_theta_mrad"]
            e = np.hypot(r["err_mrad"], prev["err_mrad"])
            r["delta_mrad"] = d
            r["significance"] = abs(d) / e if e > 0 else float("nan")

    out = args.out or os.path.join(args.indir, "resolution.csv")
    cols = ["N", "events", "core_events", "spacing_cm", "scint_cm",
            "sigma_theta_mrad", "err_mrad", "rms_sigma_theta_mrad",
            "delta_mrad", "significance"]
    with open(out, "w") as fh:
        fh.write(",".join(cols) + "\n")
        for r in rows:
            fh.write(",".join(
                "" if isinstance(r[c], float) and np.isnan(r[c]) else
                (f"{r[c]:.4f}" if isinstance(r[c], float) else str(r[c]))
                for c in cols) + "\n")

    best = min(rows, key=lambda r: r["sigma_theta_mrad"])
    print(f"\nwrote {out}")
    print(f"optimum: N = {best['N']}  "
          f"sigma_theta = {best['sigma_theta_mrad']:.3f} "
          f"+- {best['err_mrad']:.3f} mrad")
    return 0


if __name__ == "__main__":
    sys.exit(main())
