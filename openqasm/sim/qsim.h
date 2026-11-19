/* qsim.h — AVIAN-MORPH reference state-vector simulator.
 *
 * A small, exact state-vector simulator for the OpenQASM circuit
 * templates in openqasm/. It implements the gates those templates
 * use: ry, rz, and cx (CNOT). It exists to verify the templates
 * by execution, because the Mercury compiler (mmc) is not installed
 * on this machine and no typed gate IR validation is run.
 *
 * Convention: qubit index k is bit k of the basis-state index
 * (qubit 0 is the least significant bit). The state starts in
 * |0...0>. All arithmetic is double-precision complex.
 *
 * This is a reference simulator, not a performance kernel. It is
 * written in C11 and uses only the C standard library.
 */

#ifndef AVIAN_QSIM_H
#define AVIAN_QSIM_H

#include <complex.h>
#include <stddef.h>

typedef struct {
    int n;               /* number of qubits */
    size_t dim;          /* 2^n */
    double complex *amp; /* amplitudes, length dim */
} qsim_state;

/* Allocate a state with n qubits, all in |0...0>. Returns 0 on
 * success, -1 on allocation failure or invalid n (n < 1 or
 * n > 20). */
int qsim_init(qsim_state *s, int n);

/* Free the state. */
void qsim_free(qsim_state *s);

/* Single-qubit Y rotation by theta on qubit q. */
void qsim_ry(qsim_state *s, int q, double theta);

/* Single-qubit Z rotation by phi on qubit q. */
void qsim_rz(qsim_state *s, int q, double phi);

/* Controlled-NOT: flip target t when control c is 1. */
void qsim_cx(qsim_state *s, int c, int t);

/* Probability that measuring qubit q gives 1. */
double qsim_prob_one(const qsim_state *s, int q);

/* Total probability (sum of |amp|^2). Returns 1 for a
 * normalized state. */
double qsim_norm(const qsim_state *s);

#endif /* AVIAN_QSIM_H */
