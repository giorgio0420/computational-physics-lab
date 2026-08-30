"""Animate the lattice gas, and show why density controls how far it moves.

Runs bin/gas and turns the run into an animated GIF:

  left    the lattice at each sweep. Particles are dark. Empty sites are
          coloured by which connected pocket of empty space they belong to,
          wrapping across the periodic boundary.
  right   mean squared displacement against sweep, drawn as it goes, and how
          the empty space is broken up.

The colouring is the point, and it is where this program meets per.c next door.
A particle can only step into an empty neighbour, so how far the gas rearranges
is set by the shape of the empty space, not by the particles. At low density the
empty sites form one connected network spanning the lattice - they percolate -
and particles travel through it. Past the site percolation threshold for the
empty sites, about 0.593 on a 2D square lattice and so a gas density near 0.407,
that network breaks into isolated pockets and the displacement collapses.

    python scripts/animate_gas.py                      # one run -> figures/gas.gif
    python scripts/animate_gas.py --density 0.35
    python scripts/animate_gas.py --compare            # 3 densities side by side

Needs bin/gas - run `make` first.
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
GAS = os.path.join(ROOT, 'bin', 'gas')
FIGS = os.path.join(ROOT, 'figures')

PARTICLE = '#1B1F27'
VOID_COLOURS = ['#F2C14E', '#E07A5F', '#4F9DA6', '#8E6C99', '#7FA650',
                '#D4694A', '#3D6E8C', '#C9A227', '#5B8C7B', '#A8577E']
POCKET_GREY = '#9AA6B4'          # everything past the named colours


def run_gas(L, density, sweeps, seed, every):
    """Run the C program and hand back (snapshots, sweep_index, msd)."""
    if not (os.path.exists(GAS) or os.path.exists(GAS + '.exe')):
        sys.exit('bin/gas not found - run `make` first')

    steps = sweeps * max(1, int(round(density * L * L)))
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

    Returns an integer array - 0 where a particle sits, otherwise a cluster id
    starting at 1, largest cluster first - and a size per id.
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
    img = np.zeros((grid.shape[0], grid.shape[1], 3))
    img[:] = _hex(PARTICLE)
    for cid in np.unique(lab):
        if cid == 0:
            continue
        colour = VOID_COLOURS[cid - 1] if cid <= len(VOID_COLOURS) else POCKET_GREY
        img[lab == cid] = _hex(colour)
    return img, len(sizes), (max(sizes.values()) if sizes else 0)


def _hex(h):
    h = h.lstrip('#')
    return np.array([int(h[k:k + 2], 16) / 255 for k in (0, 2, 4)])


def pocket_label(big, cells):
    frac = 100.0 * big / cells
    return ('%.0f%% of the lattice' % frac) if frac >= 1 else ('%d sites' % big)


def style_axes(ax):
    ax.grid(alpha=0.25, lw=0.6)
    for sp in ('top', 'right'):
        ax.spines[sp].set_visible(False)


def build_single(args):
    """One run: the lattice beside its displacement curve."""
    print('running density %.2f ...' % args.density, flush=True)
    frames, sweeps, msd = run_gas(args.L, args.density, args.sweeps,
                                  args.seed, args.every)
    if not frames:
        sys.exit('no snapshots came back - is --every larger than --sweeps?')
    cells = args.L * args.L

    fig = plt.figure(figsize=(9.0, 4.6), dpi=110)
    gs = fig.add_gridspec(2, 2, width_ratios=[1, 1.15], left=0.055, right=0.975,
                          top=0.83, bottom=0.12, hspace=0.42, wspace=0.18)

    axl = fig.add_subplot(gs[:, 0])
    rgb, npock, big = to_rgb(frames[0])
    im = axl.imshow(rgb, interpolation='nearest')
    axl.set_xticks([]); axl.set_yticks([])
    for sp in axl.spines.values():
        sp.set_edgecolor('#C9D1DA')
    ttl = axl.set_title('', fontsize=9.5, pad=6)

    ax1 = fig.add_subplot(gs[0, 1])
    ln1, = ax1.plot([], [], lw=1.9, color='#F2C14E')
    ax1.set_xlim(0, sweeps.max() if sweeps.size else 1)
    ax1.set_ylim(0, (msd.max() * 1.06) if msd.size and msd.max() > 0 else 1)
    ax1.set_ylabel('mean squared\ndisplacement')
    style_axes(ax1)

    pockets = [to_rgb(f)[1] for f in frames]
    ax2 = fig.add_subplot(gs[1, 1])
    ln2, = ax2.plot([], [], lw=1.9, color='#3D6E8C')
    ax2.set_xlim(0, sweeps.max() if sweeps.size else 1)
    ax2.set_ylim(0, max(pockets) * 1.15 + 1)
    ax2.set_ylabel('pockets of\nempty space')
    ax2.set_xlabel('sweep')
    style_axes(ax2)

    fig.suptitle('Lattice gas: empty space is what lets the gas move',
                 fontsize=12.5, y=0.965)
    fig.text(0.5, 0.895,
             'L = %d  ·  density %.2f  ·  empty sites coloured by connected pocket'
             % (args.L, args.density),
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        rgb, npock, big = to_rgb(frames[f])
        im.set_data(rgb)
        ttl.set_text('%d pockets  ·  largest %s'
                     % (npock, pocket_label(big, cells)))
        upto = (f + 1) * args.every
        m = sweeps <= upto
        ln1.set_data(sweeps[m], msd[m])
        # one pocket count per frame, and a frame is `every` sweeps apart
        ln2.set_data([(k + 1) * args.every for k in range(f + 1)],
                     pockets[:f + 1])
        return im, ln1, ln2, ttl

    return fig, update, len(frames)


def build_compare(args):
    """Three densities side by side, sharing one displacement plot."""
    runs = []
    for d in args.compare:
        print('running density %.2f ...' % d, flush=True)
        runs.append(run_gas(args.L, d, args.sweeps, args.seed, args.every))
    n_frames = min(len(r[0]) for r in runs)
    if n_frames == 0:
        sys.exit('no snapshots came back - is --every larger than --sweeps?')
    cells = args.L * args.L

    fig = plt.figure(figsize=(9.6, 5.4), dpi=110)
    gs = fig.add_gridspec(2, 3, height_ratios=[1.45, 1], hspace=0.40, wspace=0.12,
                          left=0.07, right=0.985, top=0.845, bottom=0.11)

    images, titles = [], []
    for k, d in enumerate(args.compare):
        ax = fig.add_subplot(gs[0, k])
        images.append(ax.imshow(to_rgb(runs[k][0][0])[0], interpolation='nearest'))
        titles.append(ax.set_title('', fontsize=9.5, pad=5, linespacing=1.5))
        ax.set_xticks([]); ax.set_yticks([])
        for sp in ax.spines.values():
            sp.set_edgecolor('#C9D1DA')

    axm = fig.add_subplot(gs[1, :])
    lines = [axm.plot([], [], lw=1.9, label='density %.2f' % d,
                      color=VOID_COLOURS[k])[0]
             for k, d in enumerate(args.compare)]
    top = max(r[2].max() for r in runs) * 1.06
    axm.set_xlim(0, max(r[1].max() for r in runs))
    axm.set_ylim(0, top if top > 0 else 1)
    axm.set_xlabel('sweep')
    axm.set_ylabel('mean squared displacement')
    axm.legend(loc='upper left', frameon=False, ncol=3)
    style_axes(axm)

    fig.suptitle('Lattice gas: empty space is what lets the gas move',
                 fontsize=12.5, y=0.978)
    fig.text(0.5, 0.933,
             'empty sites coloured by connected pocket  ·  particles dark',
             ha='center', fontsize=8.5, color='#5A6472')

    def update(f):
        for k in range(len(args.compare)):
            frames, sweeps, msd = runs[k]
            rgb, npock, big = to_rgb(frames[f])
            images[k].set_data(rgb)
            titles[k].set_text('density %.2f\n%d pockets  ·  largest %s'
                               % (args.compare[k], npock,
                                  pocket_label(big, cells)))
            upto = (f + 1) * args.every
            m = sweeps <= upto
            lines[k].set_data(sweeps[m], msd[m])
        return images + lines + titles

    return fig, update, n_frames


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--L', type=int, default=40)
    ap.add_argument('--density', type=float, default=0.55)
    ap.add_argument('--sweeps', type=int, default=160)
    ap.add_argument('--every', type=int, default=4, help='sweeps between frames')
    ap.add_argument('--seed', type=int, default=4242)
    ap.add_argument('--compare', type=float, nargs='*', metavar='D',
                    help='three densities side by side; bare flag uses 0.2 0.6 0.9')
    ap.add_argument('--out')
    ap.add_argument('--fps', type=int, default=8)
    args = ap.parse_args()

    if args.compare is not None:
        if not args.compare:
            args.compare = [0.2, 0.6, 0.9]
        if len(args.compare) != 3:
            sys.exit('--compare wants exactly three densities')
        fig, update, n = build_compare(args)
        default_out = os.path.join(FIGS, 'gas_densities.gif')
    else:
        fig, update, n = build_single(args)
        default_out = os.path.join(FIGS, 'gas.gif')

    out = args.out or default_out
    plt.rcParams.update({'font.size': 9, 'font.family': 'DejaVu Sans'})
    anim = animation.FuncAnimation(fig, update, frames=n,
                                   interval=1000 // args.fps, blit=False)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    anim.save(out, writer=animation.PillowWriter(fps=args.fps))
    print('wrote %s  (%d frames, %.2f MB)'
          % (out, n, os.path.getsize(out) / 1e6))


if __name__ == '__main__':
    main()
