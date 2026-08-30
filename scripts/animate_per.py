"""Animate the percolation relaxation in per.c.

per.c gives every site its own label, flips a fraction of them negative to mark
a broken link, and then sweeps the lattice over and over, each cell copying a
neighbour's label as long as the sign matches. It stops when a whole sweep
changes nothing. What is left is the connected clusters: every site in a cluster
carries the same label.

This script runs it once and animates that convergence.

  left    the lattice. Working sites are bright, one colour per cluster; broken
          sites are dark, likewise grouped. At the start every cell is its own
          label, so it looks like noise; as the sweeps go by, patches of uniform
          colour appear - those are the clusters finding each other.
  right   how many distinct labels are left, and how big the largest working
          cluster is as a share of the lattice.

    python scripts/animate_per.py                     # writes figures/percolation.gif
    python scripts/animate_per.py --L 24 --p 0.45

The interesting parameter is --p, the fraction of broken links. Below the site
percolation threshold for a 2D square lattice, about 0.593 working sites, the
working cluster spans the lattice and the network holds together. Above it, it
shatters into islands.

Needs bin/per - run `make` first.
"""
import argparse
import os
import subprocess
import sys

import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib import animation

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PER = os.path.join(ROOT, 'bin', 'per')

WORKING = ['#F2C14E', '#E07A5F', '#4F9DA6', '#8E6C99', '#7FA650', '#D4694A',
           '#3D6E8C', '#C9A227', '#5B8C7B', '#A8577E', '#E2A03F', '#6B9E78']
BROKEN = ['#1B1F27', '#252B36', '#2F3745', '#394352', '#434F61']


def run_per(L, p, seed):
    """Run per.c once and return every lattice it printed, in order."""
    if not (os.path.exists(PER) or os.path.exists(PER + '.exe')):
        sys.exit('bin/per not found - run `make` first')

    out = subprocess.run([PER, str(L), str(p), str(seed)],
                         capture_output=True, text=True, check=True).stdout

    grids, row_buf = [], []
    for line in out.splitlines():
        parts = line.split()
        # a lattice row is exactly L integers and nothing else
        if len(parts) == L and all(v.lstrip('-').isdigit() for v in parts):
            row_buf.append([int(v) for v in parts])
            if len(row_buf) == L:
                grids.append(np.array(row_buf, dtype=int))
                row_buf = []
        else:
            row_buf = []
    return grids


def colour_of(label):
    """Deterministic colour: sign picks the family, magnitude picks the shade."""
    if label > 0:
        return WORKING[label % len(WORKING)]
    return BROKEN[(-label) % len(BROKEN)]


def to_rgb(grid):
    L = grid.shape[0]
    img = np.zeros((L, L, 3))
    for lab in np.unique(grid):
        img[grid == lab] = _hex(colour_of(int(lab)))
    return img


def _hex(h):
    h = h.lstrip('#')
    return np.array([int(h[k:k + 2], 16) / 255 for k in (0, 2, 4)])


def stats(grid):
    """Distinct labels left, and the largest working cluster as a share."""
    labels = np.unique(grid)
    working = grid[grid > 0]
    if working.size == 0:
        return len(labels), 0.0
    _, counts = np.unique(working, return_counts=True)
    return len(labels), counts.max() / grid.size


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--L', type=int, default=24, help='lattice size')
    ap.add_argument('--p', type=float, default=0.45, help='fraction of broken links')
    ap.add_argument('--seed', type=int, default=2024)
    ap.add_argument('--frames', type=int, default=48, help='frames to keep')
    ap.add_argument('--out', default=os.path.join(ROOT, 'figures', 'percolation.gif'))
    ap.add_argument('--fps', type=int, default=8)
    args = ap.parse_args()

    print('running per %d %.2f %d ...' % (args.L, args.p, args.seed), flush=True)
    grids = run_per(args.L, args.p, args.seed)
    if not grids:
        sys.exit('no lattices came back from per')

    # per.c reprints the lattice after every single cell it touches, so a run is
    # thousands of frames long; keep an even spread and always the last one
    if len(grids) > args.frames:
        idx = np.linspace(0, len(grids) - 1, args.frames).round().astype(int)
        grids = [grids[i] for i in idx]
    print('%d frames' % len(grids))

    nlab = [stats(g)[0] for g in grids]
    biggest = [100 * stats(g)[1] for g in grids]

    plt.rcParams.update({'font.size': 9, 'font.family': 'DejaVu Sans'})
    fig = plt.figure(figsize=(9.0, 4.6), dpi=110)
    gs = fig.add_gridspec(2, 2, width_ratios=[1, 1.15], height_ratios=[1, 1],
                          left=0.055, right=0.975, top=0.83, bottom=0.12,
                          hspace=0.42, wspace=0.18)

    axl = fig.add_subplot(gs[:, 0])
    im = axl.imshow(to_rgb(grids[0]), interpolation='nearest')
    axl.set_xticks([]); axl.set_yticks([])
    for sp in axl.spines.values():
        sp.set_edgecolor('#C9D1DA')
    ttl = axl.set_title('', fontsize=9.5, pad=6)

    ax1 = fig.add_subplot(gs[0, 1])
    ln1, = ax1.plot([], [], lw=1.9, color='#3D6E8C')
    ax1.set_xlim(0, len(grids) - 1)
    ax1.set_ylim(0, max(nlab) * 1.06)
    ax1.set_ylabel('distinct labels')
    ax1.grid(alpha=0.25, lw=0.6)

    ax2 = fig.add_subplot(gs[1, 1])
    ln2, = ax2.plot([], [], lw=1.9, color='#F2C14E')
    ax2.set_xlim(0, len(grids) - 1)
    ax2.set_ylim(0, max(max(biggest) * 1.15, 5))
    ax2.set_ylabel('largest working\ncluster (% of lattice)')
    ax2.set_xlabel('relaxation progress')
    ax2.grid(alpha=0.25, lw=0.6)

    for ax in (ax1, ax2):
        for sp in ('top', 'right'):
            ax.spines[sp].set_visible(False)

    fig.suptitle('Percolation: clusters emerging from the relaxation',
                 fontsize=12.5, y=0.965)
    fig.text(0.5, 0.895,
             'L = %d  ·  %.0f%% of links broken  ·  bright = working, dark = broken'
             % (args.L, 100 * args.p),
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        im.set_data(to_rgb(grids[f]))
        ttl.set_text('%d distinct labels  ·  largest working cluster %.0f%%'
                     % (nlab[f], biggest[f]))
        ln1.set_data(range(f + 1), nlab[:f + 1])
        ln2.set_data(range(f + 1), biggest[:f + 1])
        return im, ln1, ln2, ttl

    anim = animation.FuncAnimation(fig, update, frames=len(grids),
                                   interval=1000 // args.fps, blit=False)
    os.makedirs(os.path.dirname(args.out), exist_ok=True)
    anim.save(args.out, writer=animation.PillowWriter(fps=args.fps))
    print('wrote %s  (%.2f MB)' % (args.out, os.path.getsize(args.out) / 1e6))
    print('ended with %d clusters, largest working cluster %.0f%% of the lattice'
          % (nlab[-1], biggest[-1]))


if __name__ == '__main__':
    main()
