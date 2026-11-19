Contract — AVIAN-MORPH reference circuit simulator
====================================================

Scope
-----
This contract covers the OpenQASM circuit templates in
openqasm/neuron_3q.qasm and openqasm/retinal_pair.qasm, and
the reference state-vector simulator in openqasm/sim that
verifies them by execution.

The templates are the circuit layer of the architecture.
Mercury controls circuit semantics with a typed gate IR, but
the Mercury compiler (mmc) is not installed on this machine,
so no typed gate IR validation is run. The templates are
validated by this reference simulator instead. That boundary
is stated here and in verification/VERIFICATION_STATUS.md;
it is not hidden.

The simulator
-------------
qsim is an exact state-vector simulator for the gates the
templates use: ry, rz, and cx (CNOT). It holds 2^n complex
amplitudes in double precision. Qubit k is bit k of the
basis-state index (qubit 0 is the least significant bit).
The state starts in |0...0>. It is a reference simulator,
not a performance kernel, and it uses only the C standard
library.

Expected values (derived by hand)
---------------------------------
Neuron (3 qubits): ry(theta) q0, rz(phi) q1, cx q0,q1,
cx q1,q2. Measure q2.
  - theta = 0: q0 stays |0>. Both CNOTs do nothing.
    P(q2 = 1) = 0.
  - theta = pi: ry(pi) flips q0 to |1>. cx q0,q1 flips q1
    to |1>. cx q1,q2 flips q2 to |1>. P(q2 = 1) = 1.
  - theta = pi/2: ry(pi/2) puts q0 in (|0> + |1>)/sqrt(2).
    rz(phi) on q1 does nothing because q1 is |0> when it
    is applied. cx q0,q1 entangles q0 and q1, giving
    (|00> + |11>)/sqrt(2). cx q1,q2 gives
    (|000> + |111>)/sqrt(2). P(q2 = 1) = 1/2 for any phi.

Retinal pair (6 qubits): neuron A on q0..q2, coupling
cx q2,q3, neuron B on q3..q5. Measure q2 (spikeA) and
q5 (spikeB). All phi values are 0.
  - thetaA = 0, thetaB = 0: everything stays |0>.
    P(spikeA = 1) = 0, P(spikeB = 1) = 0.
  - thetaA = pi, thetaB = 0: A spikes (q2 = |1>). The
    coupling flips q3 to |1>. B's cx q3,q4 flips q4 to
    |1>, and cx q4,q5 flips q5 to |1>.
    P(spikeA = 1) = 1, P(spikeB = 1) = 1.
  - thetaA = pi/2, thetaB = 0: A is in superposition.
    The coupling entangles B's input with A's output,
    and the final state is (|000000> + |111111>)/sqrt(2).
    P(spikeA = 1) = 1/2, P(spikeB = 1) = 1/2.
  - thetaA = pi, thetaB = pi: A spikes (q2 = |1>) and
    the coupling flips q3 to |1>. But B's ry(pi) on q3
    flips it back to |0>, so B's CNOTs do nothing.
    P(spikeA = 1) = 1, P(spikeB = 1) = 0. This case
    tests that the coupling and B's rotation interact
    rather than simply adding.

Every circuit is also checked for normalization: the sum
of |amp|^2 is 1 after the gate sequence.

Verification boundary
---------------------
VERIFIED (by execution on this machine):
  - The simulator reproduces the hand-derived measurement
    probabilities for both templates, at the tested
    parameter values, to a tolerance of 1e-12.
  - Every simulated state is normalized.

NOT VERIFIED:
  - Mercury typed gate IR validation: the Mercury compiler
    (mmc) is not installed here. No Mercury module exists
    and no Mercury check is run.
  - The OpenQASM templates are not run on a real quantum
    device or a third-party OpenQASM simulator. They are
    validated by this reference simulator only.
  - The biological claim is not verified here. The circuits
    are computational counterparts to the avian neurons,
    not a claim that real bird neurons are quantum
    computers.
