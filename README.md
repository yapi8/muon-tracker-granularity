# Detector Granularity in Cosmic-Ray Muon Track Reconstruction

A Geant4 study of how the **number of active sampling layers** in a muon
tracker affects its **angular resolution** — and why more layers stop
helping past a point.

> **How Finely Must We Slice?**
> Callie Angeline Sutanto · Lorenzo Karyanto

---

## The result

Angular resolution improves steadily from N = 4 to N = 64, then **gets
worse again**:

| N | spacing (cm) | total scintillator (cm) | σ_θ (mrad) |
|---:|---:|---:|---:|
| 4 | 33.33 | 0.4 | 2.038 ± 0.012 |
| 8 | 14.29 | 0.8 | 1.502 ± 0.009 |
| 16 | 6.67 | 1.6 | 1.145 ± 0.007 |
| 32 | 3.23 | 3.2 | 0.932 ± 0.005 |
| **64** | **1.59** | **6.4** | **0.827 ± 0.005** |
| 128 | 0.79 | 12.8 | 0.940 ± 0.005 |
| 256 | 0.39 | 25.6 | 1.271 ± 0.007 |

Two effects pull in opposite directions as N grows:

- **Measurement term** — each layer adds an independent constraint on the
  track, so the fit uncertainty falls roughly as `1/√N`.
- **Scattering term** — each plate is 1 mm of scintillator, so the total
  material grows *linearly* with N and multiple Coulomb scattering grows
  with it.

Past N = 64 the material you add costs more than the measurement point it
buys. The limit on a tracker is not how many layers you can afford — it is
how many you can add before the detector's own material defeats it.

---

## Requirements

| | |
|---|---|
| Geant4 | 11.x, built with `GEANT4_USE_OPENGL_X11` (or Qt) for the viewer |
| CMake | ≥ 3.16 |
| Compiler | C++17 |
| Python | ≥ 3.9 with `numpy`, `matplotlib` (`pip install -r analysis/requirements.txt`) |

Geant4 must be **sourced** before building, and its **datasets** must be
reachable. If you see `G4ENSDFSTATEDATA environment variable must be set`,
that is the dataset path, not your build.

---

## Build

### Linux / macOS

```bash
source /path/to/geant4/bin/geant4.sh
git clone https://github.com/<you>/muon-tracker-granularity.git
cd muon-tracker-granularity
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

### Windows

```bat
call "C:\Program Files\Geant4-11.4\bin\geant4.bat"
cmake -S . -B build
cmake --build build --config Release
```

If Geant4 is installed somewhere else, find it first with
`where geant4.bat` — the version number is usually in the folder name.

On Windows the datasets often sit outside the install tree. Point at them
before running:

```bat
set G4ENSDFSTATEDATA=C:\Users\<you>\Documents\GEANT4\G4ENSDFSTATE3.0
set G4LEVELGAMMADATA=C:\Users\<you>\Documents\GEANT4\PhotonEvaporation6.1
set G4LEDATA=C:\Users\<you>\Documents\GEANT4\G4EMLOW8.7
set G4PARTICLEXSDATA=C:\Users\<you>\Documents\GEANT4\G4PARTICLEXS4.1
set G4SAIDXSDATA=C:\Users\<you>\Documents\GEANT4\G4SAIDDATA2.0
```

---

## Run

```bash
./muontracker <N_layers> <N_events>
```

One configuration:

```bash
./build/muontracker 64 20000      # -> results/hits_N64.csv
```

All seven, then analyse and plot:

```bash
./scripts/run_all.sh              # Windows: scripts\run_all.bat
python analysis/analyze.py        # -> results/resolution.csv
python analysis/plot_results.py   # -> results/resolution.png
```

### Looking at the detector

```bash
./build/muontracker 16 1   vis     # one muon through 16 layers
./build/muontracker 16 300 vis     # 300 events accumulated
```

`vis` opens the OpenGL viewer. If nothing appears, your Geant4 was built
without a visualisation driver — rebuild it with `GEANT4_USE_OPENGL_X11=ON`.

---

## How the resolution is measured

You cannot ask a single straight-line fit how wrong it is. So every track
is **cut in half**:

```
  layers 0 … N/2-1   ──fit──►  θ_top
  layers N/2 … N-1   ──fit──►  θ_bottom

         Δθ = θ_top − θ_bottom
```

`Δθ` is the difference of two *independent* estimates of the same angle,
so its spread is √2 wider than the spread of one estimate:

```
  σ_θ = σ(Δθ) / √2
  err(σ_θ) = σ_θ / √(2(n−1))
```

Geant4 reports the exact crossing point of each plate. A real detector
does not, so the analysis adds Gaussian **readout smearing** to each hit
before fitting (`--sigma-hit`, default 0.5 mm).

---

## Parameters

| Parameter | Value | Where |
|---|---|---|
| Stack height | 100 cm, **fixed for every N** | `DetectorConstruction.cc` |
| Layer thickness | 1 mm | `DetectorConstruction.cc` |
| Layer material | `G4_PLASTIC_SC_VINYLTOLUENE` | `DetectorConstruction.cc` |
| Gap between layers | `100 cm / (N−1)` | computed |
| Muon energy | 4 GeV, fixed | `PrimaryGeneratorAction.cc` |
| Zenith angle | 0–30° from vertical, uniform in solid angle | `PrimaryGeneratorAction.cc` |
| Physics list | `QGSP_BERT` + `G4EmStandardPhysics_option4` | `muontracker.cc` |
| Production cut | 1 mm | `muontracker.cc` |
| Trigger | hit required in **all** N layers | `EventAction.cc` |
| Readout smearing | 0.5 mm Gaussian | `analysis/analyze.py` |
| Events per config | 20 000 thrown | run argument |

**The stack height is held constant, not the spacing.** This is the single
most important choice in the setup. If the *gap* were fixed instead, the
detector would grow to 12.75 m at N = 256 and resolution would improve
simply because the lever arm got longer — a confound, not a result.

---

## Repository layout

```
muontracker.cc              entry point, physics list, run/vis mode
include/ , src/             detector, primary generator, hit recording
  DetectorConstruction      N plates, fixed 100 cm stack
  PrimaryGeneratorAction    4 GeV mu-, cos-sampled within 30°
  SteppingAction            records primary's entry into each plate
  EventAction               coincidence trigger (all N layers)
  RunAction                 CSV writer + acceptance summary
macros/                     vis.mac, vis_single.mac, vis_dense.mac
analysis/
  analyze.py                split-half fit -> resolution.csv
  plot_results.py           the log2 figure
  make_synthetic.py         fake data, to test the chain without Geant4
scripts/                    run_all.sh / run_all.bat
```

### Testing without Geant4

The analysis chain runs standalone:

```bash
python analysis/make_synthetic.py --outdir results_synth
python analysis/analyze.py --indir results_synth
```

`make_synthetic.py` uses the Highland approximation, **not** Geant4's
Urban model, so its numbers will not match the table above. It exists to
exercise the code, not to reproduce the physics.

---

## Limitations

- Granularity and material are coupled: layer thickness is fixed at 1 mm,
  so adding layers necessarily adds scatterer. Decoupling them would need
  a thickness that scales as 1/N.
- All muons are exactly 4 GeV. A real cosmic spectrum is broad, and the
  scattering term is momentum-dependent.
- Readout is idealised: Gaussian smearing, perfect efficiency, no noise,
  no dead channels, no clustering.
- N = 64 is therefore the optimum **for this geometry and energy**, not a
  universal number.

The same procedure is applied identically to all seven configurations, so
the comparison between them is internally consistent.

---

## Citation

```bibtex
@misc{sutanto_karyanto_muon_granularity,
  author = {Sutanto, Callie Angeline and Karyanto, Lorenzo},
  title  = {How Finely Must We Slice? Detector Granularity in
            Cosmic-Ray Muon Track Reconstruction},
  year   = {2026},
  howpublished = {\url{https://github.com/<you>/muon-tracker-granularity}}
}
```

## License

MIT — see [LICENSE](LICENSE).
