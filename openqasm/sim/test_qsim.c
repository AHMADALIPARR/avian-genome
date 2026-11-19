/* test_qsim.c — AVIAN-MORPH reference simulator tests.
 *
 * Verifies the OpenQASM circuit templates in openqasm/ by
 * simulating them with the reference simulator (qsim) and
 * comparing the measurement probabilities against values
 * computed by hand. No framework; plain asserts with a count.
 *
 * The expected values are derived by hand from the gate
 * sequence. See CONTRACT.md for the derivations.
 *
 * Neuron (3 qubits): ry(theta) q0, rz(phi) q1, cx q0,q1,
 * cx q1,q2. Measure q2.
 *   theta=0,    phi=0   -> P(q2=1) = 0
 *   theta=pi,   phi=0   -> P(q2=1) = 1
 *   theta=pi/2, phi=0   -> P(q2=1) = 1/2
 *   theta=pi/2, phi=pi/2 -> P(q2=1) = 1/2
 *
 * Retinal pair (6 qubits): neuron A on q0..q2, coupling
 * cx q2,q3, neuron B on q3..q5. Measure q2 (spikeA) and
 * q5 (spikeB). All phi values 0 unless stated.
 *   thetaA=0,    thetaB=0 -> P(q2=1)=0,   P(q5=1)=0
 *   thetaA=pi,   thetaB=0 -> P(q2=1)=1,   P(q5=1)=1
 *   thetaA=pi/2, thetaB=0 -> P(q2=1)=1/2, P(q5=1)=1/2
 *   thetaA=pi,   thetaB=pi -> P(q2=1)=1,  P(q5=1)=0
 *
 * Every circuit is also checked for normalization
 * (sum of |amp|^2 = 1).
 */

#include "qsim.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* M_PI is not defined by strict ISO C11 math.h. Define it here. */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int passed = 0;
static int failed = 0;

#define TOL 1e-12

static void check(const char *name, double got, double want) {
    if (fabs(got - want) <= TOL) {
        passed++;
    } else {
        failed++;
        printf("FAIL %s: got %.15f want %.15f\n", name, got, want);
    }
}

static void check_norm(const char *name, const qsim_state *s) {
    check(name, qsim_norm(s), 1.0);
}

/* Run the 3-qubit neuron template and return P(q2 = 1). */
static double neuron(double theta, double phi) {
    qsim_state s;
    double p;
    if (qsim_init(&s, 3) != 0) {
        printf("FAIL neuron: init\n");
        failed++;
        return -1.0;
    }
    qsim_ry(&s, 0, theta);
    qsim_rz(&s, 1, phi);
    qsim_cx(&s, 0, 1);
    qsim_cx(&s, 1, 2);
    check_norm("neuron norm", &s);
    p = qsim_prob_one(&s, 2);
    qsim_free(&s);
    return p;
}

/* Run the 6-qubit retinal-pair template. Returns P(q2=1) in
 * *pa and P(q5=1) in *pb. */
static void retinal_pair(double thetaA, double phiA,
                         double thetaB, double phiB,
                         double *pa, double *pb) {
    qsim_state s;
    if (qsim_init(&s, 6) != 0) {
        printf("FAIL retinal_pair: init\n");
        failed++;
        *pa = -1.0;
        *pb = -1.0;
        return;
    }
    qsim_ry(&s, 0, thetaA);
    qsim_rz(&s, 1, phiA);
    qsim_cx(&s, 0, 1);
    qsim_cx(&s, 1, 2);
    qsim_cx(&s, 2, 3);
    qsim_ry(&s, 3, thetaB);
    qsim_rz(&s, 4, phiB);
    qsim_cx(&s, 3, 4);
    qsim_cx(&s, 4, 5);
    check_norm("retinal_pair norm", &s);
    *pa = qsim_prob_one(&s, 2);
    *pb = qsim_prob_one(&s, 5);
    qsim_free(&s);
}

int main(void) {
    double pa, pb;

    printf("qsim tests\n");

    check("neuron theta=0", neuron(0.0, 0.0), 0.0);
    check("neuron theta=pi", neuron(M_PI, 0.0), 1.0);
    check("neuron theta=pi/2", neuron(M_PI / 2.0, 0.0), 0.5);
    check("neuron theta=pi/2 phi=pi/2",
          neuron(M_PI / 2.0, M_PI / 2.0), 0.5);

    retinal_pair(0.0, 0.0, 0.0, 0.0, &pa, &pb);
    check("pair 0,0 spikeA", pa, 0.0);
    check("pair 0,0 spikeB", pb, 0.0);

    retinal_pair(M_PI, 0.0, 0.0, 0.0, &pa, &pb);
    check("pair pi,0 spikeA", pa, 1.0);
    check("pair pi,0 spikeB", pb, 1.0);

    retinal_pair(M_PI / 2.0, 0.0, 0.0, 0.0, &pa, &pb);
    check("pair pi/2,0 spikeA", pa, 0.5);
    check("pair pi/2,0 spikeB", pb, 0.5);

    retinal_pair(M_PI, 0.0, M_PI, 0.0, &pa, &pb);
    check("pair pi,pi spikeA", pa, 1.0);
    check("pair pi,pi spikeB", pb, 0.0);

    printf("checks passed: %d\n", passed);
    printf("checks failed: %d\n", failed);
    if (failed == 0) {
        printf("ALL TESTS PASSED\n");
        return 0;
    }
    return 1;
}
