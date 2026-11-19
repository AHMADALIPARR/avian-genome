// neuron_3q.qasm — AVIAN-MORPH OpenQASM 3 circuit template.
//
// One biological neuron, one small quantum circuit template.
// The dendritic inputs determine the gate parameters; the membrane
// state controls the next classical-to-quantum encoding step.
// The measurement provides a classical output (a spike).
//
// This is a circuit template, not a complete spiking-neuron model.
// The biological membrane dynamics, refractoriness, and synaptic
// plasticity belong in the morphic state-transition system.
//
// Qubits:
//   neuron[0] — dendritic input (rotation angle theta)
//   neuron[1] — dendritic input (rotation angle phi)
//   neuron[2] — output (measured as output_spike)
//
// Gates:
//   ry(theta) on neuron[0], rz(phi) on neuron[1],
//   cx neuron[0], neuron[1], cx neuron[1], neuron[2].
//
// Verified by the reference simulator in openqasm/sim:
//   theta = 0   -> P(output_spike = 1) = 0
//   theta = pi  -> P(output_spike = 1) = 1
//   theta = pi/2 -> P(output_spike = 1) = 1/2 (any phi)
//
// Mercury constraint: the Mercury compiler (mmc) is not installed
// on this machine, so the typed gate IR validation is not run.
// These templates are validated by the reference simulator
// instead. See openqasm/sim/CONTRACT.md.

OPENQASM 3.0;
include "stdgates.inc";
input angle theta;
input angle phi;
qubit[3] neuron;
bit output_spike;
ry(theta) neuron[0];
rz(phi) neuron[1];
cx neuron[0], neuron[1];
cx neuron[1], neuron[2];
output_spike = measure neuron[2];
