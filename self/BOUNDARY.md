Self morphic layer — boundary
===============================

What the Self layer is
----------------------
In the AVIAN-MORPH architecture, Self holds the morphic organism
model: prototype neurons, delegation, adaptive morphology, and
object identity. Each neuron is a Self prototype that preserves
biological identity, morphology, connectivity, and developmental
state across the pipeline.

The pipeline is: genome (q) -> morphology (Self) -> circuit
parameters (Mercury/OpenQASM) -> kernels (C11/Vulkan). The Self
layer sits after genomic geometry. It turns a spatial layout and
per-neuron parameters into living prototypes that the rest of the
stack can compose and transform.

Why it is not built here
------------------------
The Self language and its runtime are not installed on this
machine. No Self code is written, run, or verified in this
repository. Nothing is substituted for it: no Ruby objects are
presented as Self prototypes, and no other prototype system is
offered in its place.

What stands in for it
---------------------
For the first milestone (integration/full_pipeline/MILESTONE.md),
the neurons are plain Ruby objects inside ruby/retinal_tectal.rb.
They carry stimulus angles and drive the OpenQASM templates.
That is a stand-in for the orchestration layer only. It does not
implement Self prototypes, delegation, adaptive morphology, or
object identity. The two-neuron circuit is fixed by the template,
not grown from morphic prototypes.

What would unblock it
---------------------
A working Self installation on a machine that runs this
repository, or an explicit decision by the owner to define the
morphic layer in another language already in the stack. Until
then the Self layer stays a documented boundary, not a silent
substitution.

Status: UNVERIFIED (toolchain missing).
See verification/VERIFICATION_STATUS.md.
