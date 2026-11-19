q geometry layer — boundary
============================

What the q layer is
-------------------
In the AVIAN-MORPH architecture, q holds the genomic geometry:
genomic coordinate maps, spatial indexing, adjacency tensors,
and geometric transformations. The genome defines where each
neuron sits in the tissue, which neurons are adjacent, and how
a genomic coordinate maps to a circuit parameter (the theta
and phi angles in the OpenQASM templates).

The pipeline is: genome (q) -> morphology (Self) -> circuit
parameters (Mercury/OpenQASM) -> kernels (C11/Vulkan). The q
layer is the first step. It turns a genome into a spatial
layout and a set of per-neuron parameters.

Why it is not built here
-------------------------
q/kdb+ is commercial software. It is not installed on this
machine, and there is no q interpreter available here. No q
code is written, run, or verified in this repository. Nothing
is substituted for it: no Python, no other array language,
and no hand-rolled re-implementation is presented as q.

What stands in for it
----------------------
For the first milestone (integration/full_pipeline/MILESTONE.md),
the genome is a plain parameter hash in ruby/retinal_tectal.rb.
It carries the angles (theta, phi) for each neuron directly.
That is a stand-in for the orchestration layer only. It does
not compute genomic coordinates, spatial indexing, or
adjacency tensors. The spatial layout of the two-neuron
circuit is fixed by the template (neuron A on qubits 0..2,
neuron B on qubits 3..5, coupling cx q2,q3), not derived
from a genome.

What would unblock it
----------------------
A licensed q/kdb+ installation on a machine that runs this
repository, or an explicit decision by the owner to define
the genomic geometry in another language already in the
stack. Until then the q layer stays a documented boundary,
not a silent substitution.

Status: UNVERIFIED (toolchain missing, commercial).
See verification/VERIFICATION_STATUS.md.
