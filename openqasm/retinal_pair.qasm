// retinal_pair.qasm — AVIAN-MORPH OpenQASM 3 circuit template.
//
// A small avian retinal-tectal circuit: two 3-qubit neurons.
// Neuron A is a retinal ganglion cell. Neuron B is a tectal cell.
// The coupling is a controlled-NOT from A's output qubit to B's
// input qubit, representing the retinal-to-tectal projection.
// The two neurons are not one entangled register per neuron;
// each neuron keeps its own 3-qubit template, and the coupling
// is a single entangling gate between them.
//
// Qubits:
//   circuit[0..2] — neuron A (retinal ganglion)
//   circuit[3..5] — neuron B (tectal)
//   circuit[2]    — A's output (spikeA)
//   circuit[5]    — B's output (spikeB)
//
// Gates:
//   Neuron A: ry(thetaA) q0, rz(phiA) q1, cx q0,q1, cx q1,q2.
//   Coupling: cx q2,q3.
//   Neuron B: ry(thetaB) q3, rz(phiB) q4, cx q3,q4, cx q4,q5.
//   Readout: measure q2 -> spikeA, measure q5 -> spikeB.
//
// Verified by the reference simulator in openqasm/sim:
//   thetaA = 0,  thetaB = 0 -> P(spikeA=1)=0, P(spikeB=1)=0
//   thetaA = pi, thetaB = 0 -> P(spikeA=1)=1, P(spikeB=1)=1
//   thetaA = pi/2, thetaB = 0 -> P(spikeA=1)=1/2, P(spikeB=1)=1/2
//   (all with phiA = phiB = 0)
//
// Mercury constraint: the Mercury compiler (mmc) is not installed
// on this machine, so the typed gate IR validation is not run.
// These templates are validated by the reference simulator
// instead. See openqasm/sim/CONTRACT.md.

OPENQASM 3.0;
include "stdgates.inc";
input angle thetaA;
input angle phiA;
input angle thetaB;
input angle phiB;
qubit[6] circuit;
bit spikeA;
bit spikeB;
ry(thetaA) circuit[0];
rz(phiA) circuit[1];
cx circuit[0], circuit[1];
cx circuit[1], circuit[2];
cx circuit[2], circuit[3];
ry(thetaB) circuit[3];
rz(phiB) circuit[4];
cx circuit[3], circuit[4];
cx circuit[4], circuit[5];
spikeA = measure circuit[2];
spikeB = measure circuit[5];
