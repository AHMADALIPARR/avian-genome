Mercury quantum-gate layer — boundary
=======================================

What the Mercury layer is
-------------------------
In the AVIAN-MORPH architecture, Mercury holds symbolic quantum
gate intelligence: a typed circuit AST, gate rules, constraints,
and deterministic validation. Mercury controls circuit semantics
with a typed gate IR before the circuits are emitted as OpenQASM.

The pipeline is: genome (q) -> morphology (Self) -> circuit
parameters (Mercury/OpenQASM) -> kernels (C11/Vulkan). The Mercury
layer sits between morphology and the concrete OpenQASM templates.
It validates that every gate is well-typed and that the circuit
obeys the intended constraints.

Why it is not built here
------------------------
The Mercury compiler (mmc) is not installed on this machine.
No Mercury code is written, run, or verified in this repository.
Nothing is substituted for it: the OpenQASM templates are not
claimed to have been type-checked by Mercury.

What stands in for it
---------------------
For the first milestone (integration/full_pipeline/MILESTONE.md),
the OpenQASM templates (openqasm/neuron_3q.qasm and
openqasm/retinal_pair.qasm) are validated by the reference
simulator in openqasm/sim/. The simulator reproduces the
hand-derived measurement probabilities to a tolerance of 1e-12
and confirms that every simulated state is normalized. That is
a stand-in for runtime behaviour only. It does not implement a
typed gate IR, gate rules, or deterministic static validation.

What would unblock it
---------------------
A working Mercury installation (mmc) on a machine that runs this
repository, or an explicit decision by the owner to define the
typed gate layer in another language already in the stack. Until
then the Mercury layer stays a documented boundary, not a silent
substitution.

Status: UNVERIFIED (toolchain missing).
See verification/VERIFICATION_STATUS.md.
