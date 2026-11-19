/* qsim_run.c — AVIAN-MORPH reference simulator driver.
 *
 * A command-line driver for the reference simulator (qsim).
 * It runs one of the OpenQASM circuit templates with concrete
 * angles and prints the measurement probabilities. It exists
 * so the Ruby orchestrator (ruby/retinal_tectal.rb) can drive
 * the verified simulator without re-implementing it.
 *
 * Usage:
 *   qsim_run neuron <theta> <phi>
 *     Runs the 3-qubit neuron template. Prints:
 *       spike <P(q2 = 1)>
 *   qsim_run pair <thetaA> <phiA> <thetaB> <phiB>
 *     Runs the 6-qubit retinal-pair template. Prints:
 *       spikeA <P(q2 = 1)>
 *       spikeB <P(q5 = 1)>
 *
 * Angles are in radians. Output is printed with 15 decimal
 * places. Exits 0 on success, 1 on usage or simulation error.
 */

#include "qsim.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_neuron(double theta, double phi) {
    qsim_state s;
    double p;
    if (qsim_init(&s, 3) != 0) {
        fprintf(stderr, "qsim_run: init failed\n");
        return 1;
    }
    qsim_ry(&s, 0, theta);
    qsim_rz(&s, 1, phi);
    qsim_cx(&s, 0, 1);
    qsim_cx(&s, 1, 2);
    p = qsim_prob_one(&s, 2);
    qsim_free(&s);
    printf("spike %.15f\n", p);
    return 0;
}

static int run_pair(double thetaA, double phiA,
                    double thetaB, double phiB) {
    qsim_state s;
    double pa, pb;
    if (qsim_init(&s, 6) != 0) {
        fprintf(stderr, "qsim_run: init failed\n");
        return 1;
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
    pa = qsim_prob_one(&s, 2);
    pb = qsim_prob_one(&s, 5);
    qsim_free(&s);
    printf("spikeA %.15f\n", pa);
    printf("spikeB %.15f\n", pb);
    return 0;
}

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "neuron") == 0 && argc == 4) {
        return run_neuron(strtod(argv[2], NULL),
                          strtod(argv[3], NULL));
    }
    if (argc >= 2 && strcmp(argv[1], "pair") == 0 && argc == 6) {
        return run_pair(strtod(argv[2], NULL),
                        strtod(argv[3], NULL),
                        strtod(argv[4], NULL),
                        strtod(argv[5], NULL));
    }
    fprintf(stderr,
        "usage: qsim_run neuron <theta> <phi>\n"
        "       qsim_run pair <thetaA> <phiA> <thetaB> <phiB>\n");
    return 1;
}
