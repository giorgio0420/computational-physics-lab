"""Turn a run of the lattice programs into an animated GIF.

    python scripts/animate.py gas               # one run  -> figures/gas.gif
    python scripts/animate.py gas --compare     # three densities side by side
    python scripts/animate.py per               # one run  -> figures/percolation.gif

Nothing else in this repository needs Python: `make` and `make check` build and
verify the C programs on their own. These animations are the only reason
requirements.txt exists.

Both models are about the same thing seen two ways - which sites on a lattice
are connected to which - so both animations colour by connected cluster.

  per   starts with every site carrying its own label and sweeps until
        neighbours of the same kind agree, so clusters assemble out of noise.
  gas   colours the EMPTY sites by connected pocket. A particle can only step
        into an empty neighbour, so the shape of the empty space is what decides
        how far the gas can rearrange. Below a gas density of about 0.407 the
        empty sites percolate and particles travel freely; above it the empty
        space shatters into pockets and the displacement collapses.

Needs the programs built - run `make` first.
"""
import argparse
import os
import subprocess
import sys
from collections import deque

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import animation

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
BIN = os.path.join(ROOT, 'bin')
FIGS = os.path.join(ROOT, 'figures')

PARTICLE = '#1B1F27'
BRIGHT = ['#F2C14E', '#E07A5F', '#4F9DA6', '#8E6C99', '#7FA650',
          '#D4694A', '#3D6E8C', '#C9A227', '#5B8C7B', '#A8577E',
          '#E2A03F', '#6B9E78']
DARK = ['#1B1F27', '#252B36', '#2F3745', '#394352', '#434F61']
GREY = '#9AA6B4'          # everything past the named colours


# ----------------------------------------------------------------- shared
def run(program, *args):
    """Run one of the compiled programs and return its stdout."""
    exe = os.path.join(BIN, program)
    if not (os.path.exists(exe) or os.path.exists(exe + '.exe')):
        sys.exit('bin/%s not found - run `make` first' % program)
    return subprocess.run([exe] + [str(a) for a in args],
                          capture_output=True, text=True, check=True).stdout


def rgb(h):
    h = h.lstrip('#')
    return np.array([int(h[k:k + 2], 16) / 255 for k in (0, 2, 4)])


def paint(grid, colour_for):
    """Map every distinct value in the grid to a colour."""
    img = np.zeros((grid.shape[0], grid.shape[1], 3))
    for value in np.unique(grid):
        img[grid == value] = rgb(colour_for(int(value)))
    return img


def lattice_axes(ax):
    ax.set_xticks([])
    ax.set_yticks([])
    for sp in ax.spines.values():
        sp.set_edgecolor('#C9D1DA')


def plot_axes(ax):
    ax.grid(alpha=0.25, lw=0.6)
    for sp in ('top', 'right'):
        ax.spines[sp].set_visible(False)


def save(fig, update, n_frames, out, fps):
    anim = animation.FuncAnimation(fig, update, frames=n_frames,
                                   interval=1000 // fps, blit=False)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    anim.save(out, writer=animation.PillowWriter(fps=fps))
    print('wrote %s  (%d frames, %.2f MB)'
          % (out, n_frames, os.path.getsize(out) / 1e6))


def clusters(grid, belongs):
    """Connected components on a torus, over the cells `belongs` accepts.

    Returns a label array - 0 where the cell is not part of any component -
    and the size of each label, renumbered largest first.
    """
    L = grid.shape[0]
    lab = np.zeros_like(grid)
    sizes = {}
    nxt = 1
    for si in range(L):
        for sj in range(L):
            if not belongs(grid[si, sj]) or lab[si, sj]:
                continue
            q = deque([(si, sj)])
            lab[si, sj] = nxt
            count = 0
            while q:
                a, b = q.popleft()
                count += 1
                for da, db in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                    na, nb = (a + da) % L, (b + db) % L
                    if belongs(grid[na, nb]) and not lab[na, nb]:
                        lab[na, nb] = nxt
                        q.append((na, nb))
            sizes[nxt] = count
            nxt += 1

    if not sizes:
        return lab, sizes
    order = sorted(sizes, key=lambda k: -sizes[k])
    remap = {old: new for new, old in enumerate(order, start=1)}
    out = np.zeros_like(lab)
    for old, new in remap.items():
        out[lab == old] = new
    return out, {remap[k]: v for k, v in sizes.items()}


# -------------------------------------------------------------- lattice gas
def read_gas(L, density, sweeps, seed, every):
    """(snapshots, sweep index, msd) from one gas run."""
    steps = sweeps * max(1, int(round(density * L * L)))
    out = run('gas', L, density, steps, seed, every)

    frames, sweep_idx, msd = [], [], []
    lines = out.splitlines()
    i = 0
    in_table = False              # the msd table sits between these markers
    while i < len(lines):
        line = lines[i]
        if line.startswith('# sweep'):
            in_table = True
        elif line.startswith('final lattice'):
            in_table = False
        if line.startswith('# snapshot'):
            grid = []
            i += 1
            while not lines[i].startswith('# end'):
                grid.append([int(v) for v in lines[i].split()])
                i += 1
            frames.append(np.array(grid, dtype=int))
        else:
            parts = line.split()
            if in_table and len(parts) == 3 and parts[0].isdigit():
                sweep_idx.append(int(parts[0]))
                msd.append(float(parts[2]))
        i += 1
    return frames, np.array(sweep_idx), np.array(msd)


def gas_frame(grid):
    """Particles dark, empty pockets coloured by size rank."""
    lab, sizes = clusters(grid, lambda v: v == 0)
    img = paint(lab, lambda cid: PARTICLE if cid == 0 else
                (BRIGHT[cid - 1] if cid <= len(BRIGHT) else GREY))
    return img, len(sizes), (max(sizes.values()) if sizes else 0)


def pocket_label(big, cells):
    frac = 100.0 * big / cells
    return ('%.0f%% of the lattice' % frac) if frac >= 1 else ('%d sites' % big)


def build_gas(args):
    print('running density %.2f ...' % args.density, flush=True)
    frames, sweeps, msd = read_gas(args.L, args.density, args.sweeps,
                                   args.seed, args.every)
    if not frames:
        sys.exit('no snapshots came back - is --every larger than --sweeps?')
    cells = args.L * args.L
    pockets = [gas_frame(f)[1] for f in frames]

    fig = plt.figure(figsize=(9.0, 4.6), dpi=110)
    gs = fig.add_gridspec(2, 2, width_ratios=[1, 1.15], left=0.055, right=0.975,
                          top=0.83, bottom=0.12, hspace=0.42, wspace=0.18)

    axl = fig.add_subplot(gs[:, 0])
    im = axl.imshow(gas_frame(frames[0])[0], interpolation='nearest')
    lattice_axes(axl)
    ttl = axl.set_title('', fontsize=9.5, pad=6)

    ax1 = fig.add_subplot(gs[0, 1])
    ln1, = ax1.plot([], [], lw=1.9, color=BRIGHT[0])
    ax1.set_xlim(0, sweeps.max() if sweeps.size else 1)
    ax1.set_ylim(0, (msd.max() * 1.06) if msd.size and msd.max() > 0 else 1)
    ax1.set_ylabel('mean squared\ndisplacement')
    plot_axes(ax1)

    ax2 = fig.add_subplot(gs[1, 1])
    ln2, = ax2.plot([], [], lw=1.9, color=BRIGHT[6])
    ax2.set_xlim(0, sweeps.max() if sweeps.size else 1)
    ax2.set_ylim(0, max(pockets) * 1.15 + 1)
    ax2.set_ylabel('pockets of\nempty space')
    ax2.set_xlabel('sweep')
    plot_axes(ax2)

    fig.suptitle('Lattice gas: empty space is what lets the gas move',
                 fontsize=12.5, y=0.965)
    fig.text(0.5, 0.895,
             'L = %d  ·  density %.2f  ·  empty sites coloured by connected pocket'
             % (args.L, args.density),
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        img, npock, big = gas_frame(frames[f])
        im.set_data(img)
        ttl.set_text('%d pockets  ·  largest %s'
                     % (npock, pocket_label(big, cells)))
        m = sweeps <= (f + 1) * args.every
        ln1.set_data(sweeps[m], msd[m])
        # one pocket count per frame, and a frame is `every` sweeps apart
        ln2.set_data([(k + 1) * args.every for k in range(f + 1)], pockets[:f + 1])
        return im, ln1, ln2, ttl

    return fig, update, len(frames), os.path.join(FIGS, 'gas.gif')


def build_gas_compare(args):
    runs = []
    for d in args.compare:
        print('running density %.2f ...' % d, flush=True)
        runs.append(read_gas(args.L, d, args.sweeps, args.seed, args.every))
    n_frames = min(len(r[0]) for r in runs)
    if n_frames == 0:
        sys.exit('no snapshots came back - is --every larger than --sweeps?')
    cells = args.L * args.L

    fig = plt.figure(figsize=(9.6, 5.4), dpi=110)
    gs = fig.add_gridspec(2, 3, height_ratios=[1.45, 1], hspace=0.40, wspace=0.12,
                          left=0.07, right=0.985, top=0.845, bottom=0.11)

    images, titles = [], []
    for k in range(3):
        ax = fig.add_subplot(gs[0, k])
        images.append(ax.imshow(gas_frame(runs[k][0][0])[0], interpolation='nearest'))
        titles.append(ax.set_title('', fontsize=9.5, pad=5, linespacing=1.5))
        lattice_axes(ax)

    axm = fig.add_subplot(gs[1, :])
    lines = [axm.plot([], [], lw=1.9, label='density %.2f' % d, color=BRIGHT[k])[0]
             for k, d in enumerate(args.compare)]
    top = max(r[2].max() for r in runs) * 1.06
    axm.set_xlim(0, max(r[1].max() for r in runs))
    axm.set_ylim(0, top if top > 0 else 1)
    axm.set_xlabel('sweep')
    axm.set_ylabel('mean squared displacement')
    axm.legend(loc='upper left', frameon=False, ncol=3)
    plot_axes(axm)

    fig.suptitle('Lattice gas: empty space is what lets the gas move',
                 fontsize=12.5, y=0.978)
    fig.text(0.5, 0.933,
             'empty sites coloured by connected pocket  ·  particles dark',
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        for k in range(3):
            frames, sweeps, msd = runs[k]
            img, npock, big = gas_frame(frames[f])
            images[k].set_data(img)
            titles[k].set_text('density %.2f\n%d pockets  ·  largest %s'
                               % (args.compare[k], npock, pocket_label(big, cells)))
            m = sweeps <= (f + 1) * args.every
            lines[k].set_data(sweeps[m], msd[m])
        return images + lines + titles

    return fig, update, n_frames, os.path.join(FIGS, 'gas_densities.gif')


# --------------------------------------------------------------- percolation
def read_per(L, p, seed):
    """Every lattice per.c printed, in order."""
    grids, buf = [], []
    for line in run('per', L, p, seed).splitlines():
        parts = line.split()
        if len(parts) == L and all(v.lstrip('-').isdigit() for v in parts):
            buf.append([int(v) for v in parts])
            if len(buf) == L:
                grids.append(np.array(buf, dtype=int))
                buf = []
        else:
            buf = []
    return grids


def per_frame(grid):
    """Working links bright, broken links dark, one colour per cluster."""
    return paint(grid, lambda v: BRIGHT[v % len(BRIGHT)] if v > 0
                 else DARK[(-v) % len(DARK)])


def per_stats(grid):
    """Distinct labels left, and the largest working cluster as a share."""
    working = grid[grid > 0]
    if working.size == 0:
        return len(np.unique(grid)), 0.0
    _, counts = np.unique(working, return_counts=True)
    return len(np.unique(grid)), counts.max() / grid.size


def build_per(args):
    print('running per %d %.2f %d ...' % (args.L, args.p, args.seed), flush=True)
    grids = read_per(args.L, args.p, args.seed)
    if not grids:
        sys.exit('no lattices came back from per')

    # per.c reprints the lattice after every cell it touches, so a run is
    # thousands of frames long; keep an even spread, always including the last
    if len(grids) > args.frames:
        idx = np.linspace(0, len(grids) - 1, args.frames).round().astype(int)
        grids = [grids[i] for i in idx]
    print('%d frames' % len(grids))

    nlab = [per_stats(g)[0] for g in grids]
    biggest = [100 * per_stats(g)[1] for g in grids]

    fig = plt.figure(figsize=(9.0, 4.6), dpi=110)
    gs = fig.add_gridspec(2, 2, width_ratios=[1, 1.15], left=0.055, right=0.975,
                          top=0.83, bottom=0.12, hspace=0.42, wspace=0.18)

    axl = fig.add_subplot(gs[:, 0])
    im = axl.imshow(per_frame(grids[0]), interpolation='nearest')
    lattice_axes(axl)
    ttl = axl.set_title('', fontsize=9.5, pad=6)

    ax1 = fig.add_subplot(gs[0, 1])
    ln1, = ax1.plot([], [], lw=1.9, color=BRIGHT[6])
    ax1.set_xlim(0, len(grids) - 1)
    ax1.set_ylim(0, max(nlab) * 1.06)
    ax1.set_ylabel('distinct labels')
    plot_axes(ax1)

    ax2 = fig.add_subplot(gs[1, 1])
    ln2, = ax2.plot([], [], lw=1.9, color=BRIGHT[0])
    ax2.set_xlim(0, len(grids) - 1)
    ax2.set_ylim(0, max(max(biggest) * 1.15, 5))
    ax2.set_ylabel('largest working\ncluster (% of lattice)')
    ax2.set_xlabel('relaxation progress')
    plot_axes(ax2)

    fig.suptitle('Percolation: clusters emerging from the relaxation',
                 fontsize=12.5, y=0.965)
    fig.text(0.5, 0.895,
             'L = %d  ·  %.0f%% of links broken  ·  bright = working, dark = broken'
             % (args.L, 100 * args.p),
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        im.set_data(per_frame(grids[f]))
        ttl.set_text('%d distinct labels  ·  largest working cluster %.0f%%'
                     % (nlab[f], biggest[f]))
        ln1.set_data(range(f + 1), nlab[:f + 1])
        ln2.set_data(range(f + 1), biggest[:f + 1])
        return im, ln1, ln2, ttl

    return fig, update, len(grids), os.path.join(FIGS, 'percolation.gif')


# ---------------------------------------------------------------------- cli
def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', help='override the output path')
    ap.add_argument('--fps', type=int, default=8)
    sub = ap.add_subparsers(dest='model', required=True)

    g = sub.add_parser('gas', help='lattice gas')
    g.add_argument('--L', type=int, default=40)
    g.add_argument('--density', type=float, default=0.55)
    g.add_argument('--sweeps', type=int, default=160)
    g.add_argument('--every', type=int, default=4, help='sweeps between frames')
    g.add_argument('--seed', type=int, default=4242)
    g.add_argument('--compare', type=float, nargs='*', metavar='D',
                   help='three densities side by side; bare flag uses 0.2 0.6 0.9')

    p = sub.add_parser('per', help='percolation')
    p.add_argument('--L', type=int, default=24)
    p.add_argument('--p', type=float, default=0.45, help='fraction of broken links')
    p.add_argument('--seed', type=int, default=2024)
    p.add_argument('--frames', type=int, default=48)

    args = ap.parse_args()
    plt.rcParams.update({'font.size': 9, 'font.family': 'DejaVu Sans'})

    if args.model == 'per':
        fig, update, n, out = build_per(args)
    elif args.compare is not None:
        if not args.compare:
            args.compare = [0.2, 0.6, 0.9]
        if len(args.compare) != 3:
            sys.exit('--compare wants exactly three densities')
        fig, update, n, out = build_gas_compare(args)
    else:
        fig, update, n, out = build_gas(args)

    save(fig, update, n, args.out or out, args.fps)


if __name__ == '__main__':
    main()
