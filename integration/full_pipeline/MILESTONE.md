First milestone — morphic digital twin of a small avian
retinal-tectal circuit
========================================================

Scope
-----
This milestone is a working digital twin of a small avian
retinal-tectal circuit: one retinal ganglion cell (neuron A)
driving one tectal cell (neuron B) through a controlled-NOT
coupling that represents the retinal-to-tectal projection.
Each neuron keeps its own 3-qubit circuit template. The
coupling is a single entangling gate between them.

What is built
-------------
  - OpenQASM templates: openqasm/neuron_3q.qasm and
    openqasm/retinal_pair.qasm. Verified by the reference
    simulator (openqasm/sim, 20 checks, 0 failures).
  - Reference simulator driver: openqasm/sim/qsim_run.
  - Morphic layer: ruby/morphic.rb — prototype neurons with
    clone/delegate, RetinalGanglion and Tectal prototypes.
  - Genomic layout: ruby/genome.rb — affine transform of
    tissue coordinates (same contract as the C11/CBQN geom
    kernel) producing twin parameters.
  - Typed gate IR: ruby/gate_ir.rb — validates qubit bounds,
    arity, and measurements, then emits OpenQASM.
  - Orchestrator: ruby/retinal_tectal.rb — wires morphic +
    genome + gate_ir, drives the simulator for three stimulus
    conditions, writes fixtures/quantum/retinal_pair_bright.qasm.

How to run it
-------------
  ruby ruby/retinal_tectal.rb

It prints the spike probability of each neuron for each
condition and writes the bright-condition OpenQASM. It exits 0
only when every simulator call succeeds and every reported
probability is in [0, 1].

Results (measured on this machine)
----------------------------------
  dark   (thetaA=0):    A silent (0.000000), B silent (0.000000)
  dim    (thetaA=pi/2): A graded (0.500000), B graded (0.500000)
  bright (thetaA=pi):   A spiking (1.000000), B spiking (1.000000)

The tectal cell follows the retinal cell: B spikes exactly
when A spikes, because the coupling is a controlled-NOT from
A's output to B's input and B is at rest (thetaB = 0).

Notes
-----
  - morphic.rb implements prototype behaviour in Ruby; it is
    not the Self language runtime.
  - genome.rb implements the genomic coordinate policy in Ruby
    using the project's verified geometry contract; it is not
    q/kdb+.
  - gate_ir.rb implements typed circuit construction and
    validation in Ruby; it is not the Mercury compiler.
  - SAW and Cryptol are not installed; no formal proofs are
    claimed.
