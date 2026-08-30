/* Lattice gas on an L x L torus with hard-core exclusion.
 *
 * Every site is either empty or holds one particle. At each step a random
 * particle attempts a move to one of its four neighbours; the move succeeds
 * only if the target site is empty. That single rule - no two particles on
 * one site - is what makes the model interesting: at low density particles
 * diffuse freely, at high density they block each other and the whole system
 * slows down.
 *
 * usage: gas [L] [density] [n_steps] [seed] [snapshot_every]
 *        defaults: L=4  density=0.7  n_steps=100  seed=12345  snapshot_every=0
 *
 * snapshot_every > 0 dumps the whole lattice that many sweeps apart, in a form
 * scripts/animate_gas.py can parse. 0 turns it off.
 *
 * Prints the initial lattice, then one line per sweep with the mean squared
 * displacement, then the final lattice. A sweep is N attempted moves, N being
 * the number of particles, so on average every particle has tried once.
 *
 * Displacement is measured on unwrapped coordinates: when a particle crosses
 * the periodic boundary its unwrapped position keeps counting, otherwise the
 * wrap would look like a huge jump backwards and the MSD would be nonsense.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Park-Miller (MINSTD). 64-bit state: on Windows `unsigned long` is 32 bits
 * and 16807*seme would overflow. */
static unsigned long long seme;

static double uniform(void)
{
    seme = (16807ULL * seme) % 2147483647ULL;
    return (double)seme / 2147483647.0;
}

int main(int argc, char *argv[argc])
{
    int L = 4, Tmax = 100, snap_every = 0;
    double p = 0.7;
    seme = 12345ULL;

    if (argc > 6) {
        fprintf(stderr,
                "usage: %s [L] [density] [n_steps] [seed] [snapshot_every]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 1) L    = atoi(argv[1]);
    if (argc > 2) p    = atof(argv[2]);
    if (argc > 3) Tmax = atoi(argv[3]);
    if (argc > 4) seme = strtoull(argv[4], NULL, 10);
    if (argc > 5) snap_every = atoi(argv[5]);

    if (L < 2 || p < 0.0 || p > 1.0 || Tmax < 0) {
        fprintf(stderr, "need L >= 2, 0 <= density <= 1, n_steps >= 0\n");
        exit(EXIT_FAILURE);
    }

    /* lattice: 0 = empty, otherwise the label of the particle sitting there */
    int **l = malloc(L * sizeof(int *));
    if (l == NULL) exit(EXIT_FAILURE);
    for (int i = 0; i < L; i++) {
        l[i] = malloc(L * sizeof(int));
        if (l[i] == NULL) exit(EXIT_FAILURE);
    }

    int nmax = L * L;
    /* index 0 is unused so a label can be used directly as an index */
    int *px = malloc((nmax + 1) * sizeof(int));   /* site, wrapped   */
    int *py = malloc((nmax + 1) * sizeof(int));
    int *ux = malloc((nmax + 1) * sizeof(int));   /* path, unwrapped */
    int *uy = malloc((nmax + 1) * sizeof(int));
    int *x0 = malloc((nmax + 1) * sizeof(int));   /* starting point  */
    int *y0 = malloc((nmax + 1) * sizeof(int));
    if (!px || !py || !ux || !uy || !x0 || !y0) exit(EXIT_FAILURE);

    /* fill the lattice: each site is occupied with probability p */
    int n = 0;
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            if (uniform() <= p) {
                n += 1;
                l[i][j] = n;
                px[n] = ux[n] = x0[n] = i;
                py[n] = uy[n] = y0[n] = j;
            } else {
                l[i][j] = 0;
            }
        }
    }

    /* width the widest label needs, so columns never run together on a big
     * lattice - with a fixed %4d, four-digit labels merge into one token */
    int w = snprintf(NULL, 0, "%d", n);

    printf("# lattice %dx%d, density %.3f, %d particles, %d steps, seed %llu\n",
           L, L, p, n, Tmax, seme);
    printf("# 0 = empty, otherwise the particle label\n\n");
    printf("initial lattice\n");
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) printf("%*d ", w, l[i][j]);
        printf("\n");
    }

    if (n == 0 || n == nmax) {
        /* an empty lattice has nothing to move, a full one has nowhere to go */
        printf("\n# density leaves no possible move; nothing to simulate\n");
        goto done;
    }

    printf("\n# sweep   accepted   msd\n");

    int accepted = 0;
    int sweep = n;                       /* n attempted moves = one sweep */

    for (int t = 1; t <= Tmax; t++) {
        /* pick an occupied site */
        int rx, ry;
        do {
            rx = (int)(uniform() * L);
            ry = (int)(uniform() * L);
            if (rx == L) rx = L - 1;     /* guard the 1.0 edge case */
            if (ry == L) ry = L - 1;
        } while (l[rx][ry] == 0);

        /* pick one of the four directions, quarter each */
        double r = uniform();
        int dx = 0, dy = 0;
        if      (r < 0.25) dy = -1;      /* up    */
        else if (r < 0.50) dx = +1;      /* right */
        else if (r < 0.75) dy = +1;      /* down  */
        else               dx = -1;      /* left  */

        /* periodic boundaries: the lattice is a torus */
        int nx = (rx + dx + L) % L;
        int ny = (ry + dy + L) % L;

        /* hard-core exclusion: move only into an empty site */
        if (l[nx][ny] == 0) {
            int lab = l[rx][ry];
            l[nx][ny] = lab;
            l[rx][ry] = 0;
            px[lab] = nx;
            py[lab] = ny;
            ux[lab] += dx;               /* unwrapped: keeps counting */
            uy[lab] += dy;
            accepted += 1;
        }

        if (t % sweep == 0) {
            double msd = 0.0;
            for (int k = 1; k <= n; k++) {
                double ddx = ux[k] - x0[k];
                double ddy = uy[k] - y0[k];
                msd += ddx * ddx + ddy * ddy;
            }
            int sw = t / sweep;
            printf("%8d %10d %9.4f\n", sw, accepted, msd / n);

            /* whole lattice, for scripts/animate_gas.py */
            if (snap_every > 0 && sw % snap_every == 0) {
                printf("# snapshot %d\n", sw);
                for (int i = 0; i < L; i++) {
                    for (int j = 0; j < L; j++)
                        printf("%d ", l[i][j] ? 1 : 0);
                    printf("\n");
                }
                printf("# end\n");
            }
        }
    }

    printf("\nfinal lattice\n");
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) printf("%*d ", w, l[i][j]);
        printf("\n");
    }

    /* the number of particles can never change: a move only relocates one */
    int check = 0;
    for (int i = 0; i < L; i++)
        for (int j = 0; j < L; j++)
            if (l[i][j] != 0) check += 1;
    printf("\nparticles: %d at the start, %d at the end\n", n, check);
    printf("accepted moves: %d of %d attempts (%.1f%%)\n",
           accepted, Tmax, Tmax ? 100.0 * accepted / Tmax : 0.0);

done:
    for (int i = 0; i < L; i++) free(l[i]);
    free(l);
    free(px); free(py); free(ux); free(uy); free(x0); free(y0);
    return 0;
}
