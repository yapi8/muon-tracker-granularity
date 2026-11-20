#!/usr/bin/env python3
"""
Generate synthetic hits_N*.csv WITHOUT Geant4.

This exists so the analysis chain can be tested (and this repository's
results sanity-checked) on a machine that has no Geant4 installed. It is
NOT a substitute for the simulation: it uses the Highland approximation
for multiple Coulomb scattering instead of Geant4's Urban model.

    python analysis/make_synthetic.py --outdir results_synth
"""
import argparse, os
import numpy as np

X0_SCINT_CM   = 42.5    # radiation length, polyvinyltoluene
PLATE_MM      = 1.0
STACK_CM      = 100.0
P_GEV         = 4.0


def highland_theta0(x_over_X0, p_gev=P_GEV):
    if x_over_X0 <= 0:
        return 0.0
    return (13.6e-3 / p_gev) * np.sqrt(x_over_X0) * (1 + 0.038 * np.log(x_over_X0))


def generate(n_layers, n_events, rng, max_theta_deg=30.0):
    spacing_mm = STACK_CM * 10.0 / (n_layers - 1)
    z = 0.5 * STACK_CM * 10.0 - np.arange(n_layers) * spacing_mm

    # Per-plate scattering. Highland gives the SPACE angle; the angle
    # projected into one plane is smaller by sqrt(2).
    t0 = highland_theta0((PLATE_MM / 10.0) / X0_SCINT_CM) / np.sqrt(2.0)

    cos_max = np.cos(np.deg2rad(max_theta_deg))
    cos_t = cos_max + (1 - cos_max) * rng.random(n_events)
    phi = 2 * np.pi * rng.random(n_events)
    slope0 = np.tan(np.arccos(cos_t)) * np.cos(phi)

    x = rng.uniform(-50, 50, n_events)
    slope = slope0.copy()
    X = np.empty((n_events, n_layers))

    for i in range(n_layers):
        if i > 0:
            # z decreases going down, so (z[i] - z[i-1]) is negative.
            x = x + slope * (z[i] - z[i - 1])
        X[:, i] = x
        slope = slope + rng.normal(0.0, t0, n_events)   # kick inside plate

    return z, X


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--outdir", default="results_synth")
    ap.add_argument("--events", type=int, default=15300)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--layers", type=int, nargs="+",
                    default=[4, 8, 16, 32, 64, 128, 256])
    a = ap.parse_args()

    os.makedirs(a.outdir, exist_ok=True)
    rng = np.random.default_rng(a.seed)

    for n in a.layers:
        z, X = generate(n, a.events, rng)
        path = os.path.join(a.outdir, f"hits_N{n}.csv")
        with open(path, "w") as fh:
            fh.write("event,layer,x_mm,y_mm,z_mm\n")
            for e in range(X.shape[0]):
                for i in range(n):
                    fh.write(f"{e},{i},{X[e,i]:.6f},0.000000,{z[i]:.6f}\n")
        print(f"wrote {path}")


if __name__ == "__main__":
    main()
