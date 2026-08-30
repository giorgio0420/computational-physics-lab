# Computational physics laboratory

![Rossler transient, phase portraits, bifurcation diagram, 3D attractor, random walks, and the 2D probability surface](gif.gif)

Numerical physics in plain C, from a university lab course. Two independent exercise sets:

1. **The Rössler system** — Runge–Kutta integration, Poincaré sections, period and
   transient measurement, and a period-doubling route to chaos. Written up in full in
   [`report/REPORT.md`](report/REPORT.md).
2. **Stochastic lattice models** — 1-D and 2-D random walks, a biased walk with traps, and
   percolation cluster labelling.

No libraries beyond libc and libm. Plots were produced with gnuplot from the programs'
stdout.

```bash
make          # build everything into bin/
make check    # reproduce the report's headline numbers
```

`make check` verifies five results against the values in the report and exits non-zero on
any mismatch:

```
Rossler system (a=0.1, b=0.1, c=1, x0=y0=z0=0)
  PASS  x,y,z at t=100 (dt=0.01)           -0.7866364173 0.3313694622 0.0901477239
  PASS  transient duration of the system   234.11
  PASS  period averaged over x,y,z         5.84817

Random walks (diffusion law <x^2> = t)
  PASS  1D <x^2>/t at t=500, n=5000        within 6%
  PASS  2D <r^2>/t at t=1e5, n=400         within 15%
```

---

## 1. The Rössler system

$$
x' = -y - z, \qquad y' = x + a\,y, \qquad z' = b + (x - c)\,z
$$

With `a = b = 0.1` this has a single control parameter `c`, and it goes from a clean
periodic orbit to chaos as `c` grows. The exercise walks that transition.

| Program | What it does |
|---|---|
| `integratore.c` | RK2 integrator. All parameters from the command line. |
| `periodo.c` | Period of each variable, from the spacing of successive maxima. |
| `duratatransiente.c` | Transient duration: when neighbouring maxima stop differing by more than 0.0005. |
| `ex1.c` | Bifurcation diagram. RK4, with linear interpolation to locate the `y(t) = 0` crossings. |
| `ex2.c` | Arc length of the trajectory over the transient, as a function of `c`. RK4. |

### Results

| Quantity | Value |
|---|---|
| `x, y, z` at `t = 100` (`dt = 0.01`) | `-0.7866364173`, `0.3313694622`, `0.0901477239` |
| Period of the stationary orbit | `5.84817` (identical for all three variables) |
| Transient duration | `t_x = 233.12`, `t_y = 217.12`, `t_z = 234.11` → system `234.11` |
| Stationary for | `c ∈ [2 : 5]`; first self-intersection at `c = 6` |
| Bifurcation cascade | `c ∈ [5 : 9.5]` |

The Poincaré sections collapse onto line segments, which is what says the orbit is a
stable limit cycle rather than a chaotic attractor. All three periods come out equal, so
their ratios are rational and the system is genuinely periodic.

The convergence study is the part worth reading: at `dt = 0.1` RK2 gives
`x(100) = 2.869`, at `dt = 10⁻⁵` it gives `-0.6118`. The `dt = 0.01` value used throughout
the rest of the report is still ~28% off the converged one. The squared distance from the
orbit centre behaves much better — `0.5136` at `dt = 10⁻⁵` versus `0.8219` at `dt = 0.01` —
because it is a property of the orbit rather than of the phase along it.

![Bifurcation diagram](report/figures/p10_1.png)

*Bifurcation diagram: `x > 0` sampled whenever `y(t) = 0`, against `c`. The period-doubling
cascade is visible from `c ≈ 5.5`.*

### Running

```bash
./bin/integratore 0.1 0.1 1 0 0 0 0.01 100 > rossler.dat
gnuplot -e "plot 'rossler.dat' u 1:2 w l, '' u 1:3 w l, '' u 1:4 w l; pause -1"

./bin/periodo             # ~20M steps at dt=1e-4, takes a minute
./bin/duratatransiente
./bin/ex1  > bifurcation.dat     # columns: c, x
./bin/ex2  > length.dat          # columns: c, L_r, t_transient
```

`integratore` takes all eight parameters; the other four have the report's parameters
compiled in, since each was written for one specific figure.

---

## 2. Stochastic lattice models

| Program | Arguments | What it does |
|---|---|---|
| `rw.c` | `n_walks t_max [seed]` | 1-D random walk. Prints `t, x, σ_x, x², σ_x², √t` per step. |
| `rw2d.c` | `n_walks t_max [seed]` | 2-D walk on a square lattice. Prints `t, x, y, P(x), P(y), P(x,y)` at `t = 10⁵`. |
| `ale.c` | `n_walks t_max seed` | Biased 1-D walk with traps at `x = ±25, ±51, ±52, ±100`. |
| `per.c` | `L p [seed]` | Percolation on an `L × L` torus, `p` = fraction of broken links. Relaxation labelling until no cluster label changes. |

Each program writes one blank-line-separated block per trajectory, which is the format
gnuplot's `index` wants.

### What the figures show

| Figure | |
|---|---|
| ![](figures/rw1d_10traj_t1e2.png) ![](figures/rw1d_10traj_t1e3.png) | 10 trajectories to `t = 10²` and `t = 10³`, with the `±√t` envelope. |
| ![](figures/rw1d_10traj_t1e4.png) | The same to `t = 10⁴`. Most trajectories stay inside `±√t`; excursions well past it are expected and visible. |
| ![](figures/rw1d_x2_vs_t.png) | `x²(t)` for two single trajectories against the line `x² = t`. A single walk fluctuates wildly around it — the diffusion law is a statement about the ensemble mean, not about any one walk. |
| ![](figures/rw1d_Pxt_t1e3_1e4_1e5.png) | `P(x_t)` at `t = 10³, 10⁴, 10⁵` with Gaussian fits. The distribution flattens and widens as `√t`. |
| ![](figures/rw1d_Pxt_t1e4_zoom.png) | The `t = 10⁴` curve alone. |
| ![](figures/rw2d_trajectory.png) | A single 2-D walk. The fractal, clustered structure is characteristic — the walk revisits neighbourhoods many times before escaping. |
| ![](figures/rw2d_Pxy_t1e5_surface.png) | `P(x, y)` at `t = 10⁵` against the expected 2-D Gaussian surface. |
| ![](figures/rw1d_traps.png) | The biased walk with traps. Two trajectories descend, stall at `x = -25`, then settle around `-50`/`-51`. |

The trap mechanism: a bias `p` is drawn once at the start, and each step goes up with
probability `1 - p`. On the trap sites the rule switches to "step up with probability 0.9",
which pushes the walk back toward the origin. When the drift is downward, the walk gets
caught for long stretches at the traps it meets. Reproduce that figure with

```bash
./bin/ale 1 1000 180        # seed 180 gives p = 0.951, a strong downward drift
```

Seeds giving small `p` produce an upward drift that passes straight through the traps
instead, which is the same code doing the same thing.

---

## Fixes applied before publishing

The programs are as originally written, apart from defects that stopped them producing
correct output on a current toolchain. Each is listed here rather than quietly folded in.

**Portability**

- `unsigned long int seme` → `unsigned long long`. `unsigned long` is 32-bit on Windows
  (LLP64), so `16807 * seme` and `22695477 * seme` overflowed and the linear congruential
  generator degenerated. On the 64-bit Linux `unsigned long` this code was written against,
  it worked.

**Seeding and arguments**

- `seme` was never initialised in `rw.c`, `rw2d.c`, `per.c`. It read whatever was on the
  stack. With a zero-filled stack the generator sticks at 0 and the walk marches in one
  direction forever. It now defaults to `12345` and takes an optional seed argument.
- `rw.c`, `rw2d.c`, `ale.c` and `integratore.c` called `atof(argv[n])` *before* checking
  `argc`, so running them with no arguments dereferenced a null pointer. The check now
  comes first, and prints a usage line.
- The `argv[argc] != NULL` half of the old argument check was always false and did nothing.

**Random number generation**

- Every draw did `seme = (a*seme) % m; srand(seme); r = rand()/RAND_MAX;`. Re-seeding the C
  library generator on every draw makes consecutive draws strongly correlated, because
  `rand()`'s first output is close to linear in its seed on several C libraries. Measured
  effect: `⟨x²⟩` came out ~30% below `t`. The `srand`/`rand` round-trip is gone and the LCG
  value is used directly, which is what the LCG was there for. `⟨x²⟩/t` is now within 2%
  over `t ∈ [50, 500]` at 5000 walks.

**Out-of-bounds access**

- `per.c` segfaulted on every run. The left-neighbour branch had been copy-pasted from the
  upper-neighbour branch and not adapted: it tested `l[i][L-1]` and assigned `l[i-1][j]`,
  reading `l[-1][j]` whenever `i == 0`. The other three branches make the intent
  unambiguous, so it now tests and assigns `l[i][j-1]`.

**Stray debug output**

- `ale.c` printed each raw random number to stdout, interleaving it with the data columns.
- `per.c` printed a per-cell trace inside the relaxation sweep.

Italian user-facing strings were translated to English. Comments in the source are
unchanged.

---

## Not working — `src/wip/`

Two programs are published as they were, because repairing them would mean rewriting them
rather than fixing them. They are excluded from the default build.

**`wip/corr.c`** — cross-correlations between `x`, `y`, `z` at varying offset `k`
(report §1.3). Its own period detection returns `0.01`, which is just `dt`, so the
normalisation `somxy/mperiodi` is meaningless. The `printf` sits inside the accumulation
loop, so it emits a running partial sum for every term instead of one row per `k`. And
`x1[k+i2]` indexes up to `10588` into a `double[10000]`. The correlation figures in the
report were produced before these regressions; `periodo.c` is the reliable period
measurement.

**`wip/gas.c`** — lattice gas on an `L × L` torus. Several independent defects: the
main loop is `do { ... } while (t > Tmax)` with `t` starting at 0, so it executes exactly
one step; `rand() % L + 1` gives indices in `[1, L]` against an array indexed `[0, L-1]`;
the site-picking loop searches for an *empty* cell and then tries to move a particle from
it; and the four direction branches overlap, so `r < 0.25` sets `direzione` to 2 rather
than 1.

---

## Layout

```
src/rossler/       RK2 and RK4 programs for the Rössler system
src/stochastic/    random walks and percolation
src/wip/           known-broken, excluded from the build (see above)
scripts/check.sh   reproduces the report's headline numbers
figures/           gnuplot output for the stochastic models
report/REPORT.md   the Rössler report, in English
report/relazione_IT.pdf   the original Italian report
report/figures/    figures extracted from the PDF, named by page
```

## Requirements

A C99 compiler and `make`. gnuplot for the plots. Nothing else.

## License

MIT — see [LICENSE](LICENSE).
