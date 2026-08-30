"""Animate the lattice gas, and show why density controls how far it moves.

Runs bin/gas at three densities and builds a single animated GIF:

  top row     the lattice at each sweep. Particles are dark. Empty sites are
              coloured by which connected pocket of empty space they belong to,
              wrapping across the periodic boundary.
  bottom      mean squared displacement against sweep, drawn as it goes.

The colouring is the point. A particle can only step into an empty neighbour,
so how far the gas can rearrange is set by the shape of the empty space, not by
the particles. At low density the empty sites form one connected network that
spans the lattice - percolation - and particles travel freely through it. Raise
the density past the site-percolation threshold for the empty sites (about
0.593 on a 2D square lattice, so a gas density near 0.407) and that network
breaks into isolated pockets. Particles can still shuffle, because they move
too, but the mean squared displacement collapses.

    python scripts/animate_gas.py                 # writes figures/gas.gif
    python scripts/animate_gas.py --L 48 --sweeps 200

Needs bin/gas to exist - run `make` first.
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
from matplotlib.colors import ListedColormap, BoundaryNorm

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
GAS = os.path.join(ROOT, 'bin', 'gas')

# empty sites are tinted by cluster, particles are a flat dark slab
PARTICLE = '#1B1F27'
VOID_COLOURS = ['#F2C14E', '#E07A5F', '#4F9DA6', '#8E6C99', '#7FA650',
                '#D4694A', '#3D6E8C', '#C9A227', '#5B8C7B', '#A8577E']
SPAN_COLOUR = '#F2C14E'          # a pocket that wraps the whole lattice
POCKET_GREY = '#9AA6B4'          # small pockets, when we stop naming them


def run_gas(L, density, sweeps, seed, every):
    """Run the C program and hand back (snapshots, sweep_index, msd)."""
    if not (os.path.exists(GAS) or os.path.exists(GAS + '.exe')):
        sys.exit('bin/gas not found - run `make` first')

    n_particles = max(1, int(round(density * L * L)))
    steps = sweeps * n_particles
    out = subprocess.run(
        [GAS, str(L), str(density), str(steps), str(seed), str(every)],
        capture_output=True, text=True, check=True).stdout

    frames, sweep_idx, msd = [], [], []
    lines = out.splitlines()
    i = 0
    in_table = False          # the msd table sits between these two markers
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


def void_clusters(grid):
    """Label connected pockets of empty sites, wrapping at the edges.

    Returns an integer array: 0 where a particle sits, otherwise a cluster id
    starting at 1, with ids ordered largest cluster first.
    """
    L = grid.shape[0]
    lab = np.zeros_like(grid)
    sizes = {}
    nxt = 1
    for si in range(L):
        for sj in range(L):
            if grid[si, sj] or lab[si, sj]:
                continue
            q = deque([(si, sj)])
            lab[si, sj] = nxt
            count = 0
            while q:
                a, b = q.popleft()
                count += 1
                for da, db in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                    na, nb = (a + da) % L, (b + db) % L      # torus
                    if not grid[na, nb] and not lab[na, nb]:
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


def to_rgb(grid):
    """Paint one frame: particles dark, empty pockets coloured by size rank."""
    lab, sizes = void_clusters(grid)
    L = grid.shape[0]
    img = np.zeros((L, L, 3))
    img[:] = _hex(PARTICLE)
    for cid in np.unique(lab):
        if cid == 0:
            continue
        colour = VOID_COLOURS[(cid - 1) % len(VOID_COLOURS)] \
            if cid <= len(VOID_COLOURS) else POCKET_GREY
        img[lab == cid] = _hex(colour)
    biggest = max(sizes.values()) if sizes else 0
    return img, len(sizes), biggest


def _hex(h):
    h = h.lstrip('#')
    return np.array([int(h[k:k + 2], 16) / 255 for k in (0, 2, 4)])


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--L', type=int, default=40)
    ap.add_argument('--sweeps', type=int, default=160)
    ap.add_argument('--every', type=int, default=4, help='sweeps between frames')
    ap.add_argument('--seed', type=int, default=4242)
    ap.add_argument('--densities', type=float, nargs=3, default=[0.2, 0.6, 0.9])
    ap.add_argument('--out', default=os.path.join(ROOT, 'figures', 'gas.gif'))
    ap.add_argument('--fps', type=int, default=8)
    args = ap.parse_args()

    runs = []
    for d in args.densities:
        print('running density %.2f ...' % d, flush=True)
        runs.append(run_gas(args.L, d, args.sweeps, args.seed, args.every))

    n_frames = min(len(r[0]) for r in runs)
    if n_frames == 0:
        sys.exit('no snapshots came back - is --every larger than --sweeps?')

    plt.rcParams.update({'font.size': 9, 'font.family': 'DejaVu Sans'})
    fig = plt.figure(figsize=(9.6, 5.4), dpi=110)
    gs = fig.add_gridspec(2, 3, height_ratios=[1.45, 1], hspace=0.40, wspace=0.12,
                          left=0.07, right=0.985, top=0.845, bottom=0.11)

    axes, images, titles = [], [], []
    for k, d in enumerate(args.densities):
        ax = fig.add_subplot(gs[0, k])
        rgb, npock, big = to_rgb(runs[k][0][0])
        images.append(ax.imshow(rgb, interpolation='nearest'))
        titles.append(ax.set_title('density %.2f' % d, fontsize=9.5, pad=5,
                                   linespacing=1.5))
        ax.set_xticks([]); ax.set_yticks([])
        for sp in ax.spines.values():
            sp.set_edgecolor('#C9D1DA')
        axes.append(ax)

    axm = fig.add_subplot(gs[1, :])
    lines = []
    for k, d in enumerate(args.densities):
        ln, = axm.plot([], [], lw=1.9, label='density %.2f' % d,
                       color=VOID_COLOURS[k])
        lines.append(ln)
    top = max(r[2].max() for r in runs) * 1.06
    axm.set_xlim(0, max(r[1].max() for r in runs))
    axm.set_ylim(0, top if top > 0 else 1)
    axm.set_xlabel('sweep')
    axm.set_ylabel('mean squared displacement')
    axm.grid(alpha=0.25, lw=0.6)
    axm.legend(loc='upper left', frameon=False, ncol=3)
    for sp in ('top', 'right'):
        axm.spines[sp].set_visible(False)

    fig.suptitle('Lattice gas: empty space is what lets the gas move',
                 fontsize=12.5, y=0.978)
    fig.text(0.5, 0.933,
             'empty sites coloured by connected pocket  ·  particles dark',
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        for k in range(3):
            frames, sweeps, msd = runs[k]
            rgb, npock, big = to_rgb(frames[f])
            images[k].set_data(rgb)
            cells = args.L * args.L
            frac = 100.0 * big / cells
            if frac >= 1:
                biggest = '%.0f%% of the lattice' % frac
            else:
                biggest = '%d sites' % big
            titles[k].set_text('density %.2f\n%d pockets  ·  largest %s'
                               % (args.densities[k], npock, biggest))
            upto = (f + 1) * args.every
            m = sweeps <= upto
            lines[k].set_data(sweeps[m], msd[m])
        return images + lines + titles

    anim = animation.FuncAnimation(fig, update, frames=n_frames,
                                   interval=1000 // args.fps, blit=False)
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    anim.save(args.out, writer=animation.PillowWriter(fps=args.fps))
    print('wrote %s  (%d frames, %.2f MB)'
          % (args.out, n_frames, os.path.getsize(args.out) / 1e6))


if __name__ == '__main__':
    main()
