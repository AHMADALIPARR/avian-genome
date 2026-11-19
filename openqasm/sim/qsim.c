/* qsim.c — AVIAN-MORPH reference state-vector simulator.
 *
 * Implements the gates in qsim.h. See the header for the
 * convention (qubit k is bit k of the basis-state index).
 */

#include "qsim.h"

#include <math.h>
#include <stdlib.h>

int qsim_init(qsim_state *s, int n) {
    if (s == NULL || n < 1 || n > 20) {
        return -1;
    }
    s->n = n;
    s->dim = (size_t)1 << n;
    s->amp = (double complex *)calloc(s->dim, sizeof(double complex));
    if (s->amp == NULL) {
        return -1;
    }
    s->amp[0] = 1.0 + 0.0 * I;
    return 0;
}

void qsim_free(qsim_state *s) {
    if (s != NULL) {
        free(s->amp);
        s->amp = NULL;
        s->n = 0;
        s->dim = 0;
    }
}

void qsim_ry(qsim_state *s, int q, double theta) {
    double c = cos(theta / 2.0);
    double sn = sin(theta / 2.0);
    size_t mask = (size_t)1 << q;
    size_t i;
    for (i = 0; i < s->dim; i++) {
        if ((i & mask) == 0) {
            size_t j = i | mask;
            double complex a0 = s->amp[i];
            double complex a1 = s->amp[j];
            s->amp[i] = c * a0 - sn * a1;
            s->amp[j] = sn * a0 + c * a1;
        }
    }
}

void qsim_rz(qsim_state *s, int q, double phi) {
    double complex e0 = cexp(-I * phi / 2.0);
    double complex e1 = cexp(I * phi / 2.0);
    size_t mask = (size_t)1 << q;
    size_t i;
    for (i = 0; i < s->dim; i++) {
        if ((i & mask) == 0) {
            s->amp[i] *= e0;
        } else {
            s->amp[i] *= e1;
        }
    }
}

void qsim_cx(qsim_state *s, int c, int t) {
    size_t cmask = (size_t)1 << c;
    size_t tmask = (size_t)1 << t;
    size_t i;
    for (i = 0; i < s->dim; i++) {
        if ((i & cmask) != 0 && (i & tmask) == 0) {
            size_t j = i | tmask;
            double complex tmp = s->amp[i];
            s->amp[i] = s->amp[j];
            s->amp[j] = tmp;
        }
    }
}

double qsim_prob_one(const qsim_state *s, int q) {
    size_t mask = (size_t)1 << q;
    double p = 0.0;
    size_t i;
    for (i = 0; i < s->dim; i++) {
        if ((i & mask) != 0) {
            double complex a = s->amp[i];
            p += creal(a) * creal(a) + cimag(a) * cimag(a);
        }
    }
    return p;
}

double qsim_norm(const qsim_state *s) {
    double p = 0.0;
    size_t i;
    for (i = 0; i < s->dim; i++) {
        double complex a = s->amp[i];
        p += creal(a) * creal(a) + cimag(a) * cimag(a);
    }
    return p;
}
