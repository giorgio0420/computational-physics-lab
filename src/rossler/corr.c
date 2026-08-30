/* Cross-correlations between x, y and z of the Rossler system (report 1.3).
 *
 * Integrates with RK2, throws away the transient, keeps a window of the
 * stationary regime, and correlates each pair of variables against a shifting
 * lag k. The lag at which a pair correlates most strongly is the phase
 * difference between them.
 *
 * usage: corr [a] [b] [c] [dt] [T]
 *        defaults: a=0.1 b=0.1 c=1 dt=0.01 T=2000
 *
 * Output: one row per lag - the lag in steps, the same lag in time units, and
 * the three correlations, each normalised to lie in [-1, 1].
 *
 * The normalisation is the textbook one,
 *
 *     C_uv(k) = < (u_i - <u>) (v_{i+k} - <v>) > / (sigma_u sigma_v)
 *
 * so C_uu(0) is exactly 1 and the numbers mean the same thing for every pair.
 * The 2021 version divided by hand-tuned constants instead, which made the
 * curves the right shape but the wrong height; these are not directly
 * comparable to the y-axis of the figures in the report.
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WINDOW 10000     /* samples correlated, once the transient is gone */
#define KMAX     600     /* largest lag, in integration steps              */

static double mean(const double *v, int n)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) s += v[i];
    return s / n;
}

static double sdev(const double *v, int n, double m)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) s += (v[i] - m) * (v[i] - m);
    return sqrt(s / n);
}

/* normalised cross-correlation of u and v at lag k */
static double correlate(const double *u, const double *v, int n, int k,
                        double mu, double su, double mv, double sv)
{
    double s = 0.0;
    for (int i = 0; i < n; i++) s += (u[i] - mu) * (v[i + k] - mv);
    return s / (n * su * sv);
}

int main(int argc, char *argv[argc])
{
    double a = 0.1, b = 0.1, c = 1.0, dt = 0.01, T = 2000.0;

    if (argc > 6) {
        fprintf(stderr, "usage: %s [a] [b] [c] [dt] [T]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc > 1) a  = atof(argv[1]);
    if (argc > 2) b  = atof(argv[2]);
    if (argc > 3) c  = atof(argv[3]);
    if (argc > 4) dt = atof(argv[4]);
    if (argc > 5) T  = atof(argv[5]);

    int n = (int)(T / dt);
    int kept = WINDOW + KMAX;          /* the lagged copy needs the extra tail */
    if (n <= kept) {
        fprintf(stderr, "T/dt must exceed %d - raise T or lower dt\n", kept);
        exit(EXIT_FAILURE);
    }

    double *xs_ = malloc(kept * sizeof(double));
    double *ys_ = malloc(kept * sizeof(double));
    double *zs_ = malloc(kept * sizeof(double));
    if (!xs_ || !ys_ || !zs_) exit(EXIT_FAILURE);

    double x = 0.0, y = 0.0, z = 0.0, t = 0.0;
    double xm = x;                          /* previous step, for the maxima */
    double t0x = 0, t1x = 0, periodox = 0;
    int qx = 0, g = 0;

    printf("#a=%g, b=%g, c=%g, dt=%g, T=%g\n", a, b, c, dt, T);

    for (int i = 0; i < n; i++) {
        double kx1 = (-y - z) * dt;
        double ky1 = (x + a * y) * dt;
        double kz1 = (b + x * z - c * z) * dt;
        double kx2 = ((-y - ky1 / 2) + (-z - kz1 / 2)) * dt;
        double ky2 = ((x + kx1 / 2) + a * (y + ky1 / 2)) * dt;
        double kz2 = (b + (x + kx1 / 2) * (z + kz1 / 2) - c * (z + kz1 / 2)) * dt;

        double xn = x + (kx1 + kx2) / 2;
        double yn = y + (ky1 + ky2) / 2;
        double zn = z + (kz1 + kz2) / 2;
        t += dt;

        /* period from two successive maxima of x: a maximum is higher than
         * BOTH neighbours. The 2021 version tested x < xn && x > xm, which is
         * merely "rising", so it fired on consecutive steps and reported a
         * period of one dt. */
        if (i > n / 2 && x > xn && x > xm) {
            if (qx == 0) {
                qx = 1;
                t0x = t;
            } else if (qx == 1) {
                qx = 2;
                t1x = t;
                periodox = t1x - t0x;
            }
        }

        /* keep the tail of the run, which is well past the transient */
        if (i >= n - kept) {
            xs_[g] = xn;
            ys_[g] = yn;
            zs_[g] = zn;
            g++;
        }

        xm = x;
        x = xn;  y = yn;  z = zn;
    }

    if (periodox > 0)
        printf("#period from successive maxima of x: %.5f  (%d steps)\n",
               periodox, (int)(periodox / dt));
    printf("#window %d samples, lags 0 to %d steps (%.2f time units)\n",
           WINDOW, KMAX, KMAX * dt);

    double mx = mean(xs_, WINDOW), sx = sdev(xs_, WINDOW, mx);
    double my = mean(ys_, WINDOW), sy = sdev(ys_, WINDOW, my);
    double mz = mean(zs_, WINDOW), sz = sdev(zs_, WINDOW, mz);

    /* a signal correlated with itself at zero lag must give exactly 1 */
    double self = correlate(xs_, xs_, WINDOW, 0, mx, sx, mx, sx);
    printf("#self-correlation of x at lag 0: %.6f  (must be 1)\n", self);

    printf("#k  lag_time  corr_xy  corr_xz  corr_yz\n");

    int best_xy = 0, best_xz = 0, best_yz = 0;
    double top_xy = -2, top_xz = -2, top_yz = -2;

    for (int k = 0; k <= KMAX; k++) {
        double cxy = correlate(xs_, ys_, WINDOW, k, mx, sx, my, sy);
        double cxz = correlate(xs_, zs_, WINDOW, k, mx, sx, mz, sz);
        double cyz = correlate(ys_, zs_, WINDOW, k, my, sy, mz, sz);

        if (cxy > top_xy) { top_xy = cxy; best_xy = k; }
        if (cxz > top_xz) { top_xz = cxz; best_xz = k; }
        if (cyz > top_yz) { top_yz = cyz; best_yz = k; }

        printf("%d  %.4f  %.6f  %.6f  %.6f\n", k, k * dt, cxy, cxz, cyz);
    }

    printf("\n#strongest correlation, i.e. the phase difference\n");
    printf("#x-y  lag %3d steps = %.3f time units, corr %.4f\n",
           best_xy, best_xy * dt, top_xy);
    printf("#x-z  lag %3d steps = %.3f time units, corr %.4f\n",
           best_xz, best_xz * dt, top_xz);
    printf("#y-z  lag %3d steps = %.3f time units, corr %.4f\n",
           best_yz, best_yz * dt, top_yz);

    free(xs_); free(ys_); free(zs_);
    return 0;
}
