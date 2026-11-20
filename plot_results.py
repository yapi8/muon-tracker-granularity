#!/usr/bin/env python3
"""
Plot angular resolution vs N from results/resolution.csv.

The x axis is logarithmic base 2 and says so. The configurations double
each step (4, 8, 16 ... 256), so the scan samples uniformly in log2(N);
a linear axis would crush five of the seven points into the left edge
and flatten the minimum.
"""
import argparse, csv, sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter, NullLocator, NullFormatter

INK, MUTED, GRID = "#1A1F2B", "#6B7280", "#E3E7EC"
TRACK, WARM = "#2E6F9E", "#C25E5E"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--infile", default="results/resolution.csv")
    ap.add_argument("--out", default="results/resolution.png")
    a = ap.parse_args()

    N, S, E = [], [], []
    try:
        with open(a.infile) as fh:
            for row in csv.DictReader(fh):
                N.append(int(row["N"]))
                S.append(float(row["sigma_theta_mrad"]))
                E.append(float(row["err_mrad"]) if row["err_mrad"] else 0.0)
    except FileNotFoundError:
        print(f"{a.infile} not found -- run analysis/analyze.py first", file=sys.stderr)
        return 1

    N, S, E = np.array(N), np.array(S), np.array(E)
    order = np.argsort(N)
    N, S, E = N[order], S[order], E[order]

    fig, ax = plt.subplots(figsize=(7.4, 4.75), dpi=200)
    fig.patch.set_facecolor("white"); ax.set_facecolor("white")
    for sp in ("top", "right"): ax.spines[sp].set_visible(False)
    for sp in ("left", "bottom"):
        ax.spines[sp].set_color("#C6CBD2"); ax.spines[sp].set_linewidth(1.0)
    ax.grid(True, color=GRID, lw=0.8); ax.set_axisbelow(True)
    ax.tick_params(colors=MUTED, labelsize=10, length=3.5)

    ax.set_xscale("log", base=2)
    ax.set_xticks(N)
    ax.xaxis.set_major_formatter(ScalarFormatter())
    ax.xaxis.set_minor_locator(NullLocator())
    ax.xaxis.set_minor_formatter(NullFormatter())
    ax.set_xlim(N.min() * 0.82, N.max() * 1.22)

    ax.errorbar(N, S, yerr=E, fmt="-", color=TRACK, lw=1.9,
                ecolor=TRACK, elinewidth=1.2, capsize=3.0, capthick=1.2, zorder=3)
    ax.plot(N, S, "o", ms=6.5, mfc=TRACK, mec="white", mew=1.4, zorder=4)

    k = int(np.argmin(S))
    ax.plot(N[k], S[k], "o", ms=13, mfc="none", mec=WARM, mew=2.0, zorder=5)
    ax.annotate(f"optimum:  N = {N[k]}\n"
                rf"$\sigma_\theta$ = {S[k]:.3f} $\pm$ {E[k]:.3f} mrad",
                xy=(N[k], S[k]),
                xytext=(N[k], S[k] + 0.42 * (S.max() - S.min())),
                fontsize=10, color=WARM, ha="center", va="center",
                linespacing=1.6, fontweight="bold",
                arrowprops=dict(arrowstyle="->", color=WARM, lw=1.3,
                                shrinkA=2, shrinkB=10))

    ax.set_xlabel(r"Number of active sampling layers, $N$   (log$_2$ scale)",
                  fontsize=11.5, color=INK, labelpad=9)
    ax.set_ylabel(r"Angular resolution  $\sigma_\theta$  (mrad)",
                  fontsize=11.5, color=INK, labelpad=9)
    ax.set_title("Angular Resolution vs. Detector Granularity",
                 fontsize=13, color=INK, pad=14, fontweight="bold")

    fig.tight_layout(pad=1.2)
    fig.savefig(a.out, facecolor="white", bbox_inches="tight")
    print(f"wrote {a.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
