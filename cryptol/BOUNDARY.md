Cryptol specification layer — boundary
========================================

What the Cryptol layer is
-------------------------
In the AVIAN-MORPH architecture, Cryptol holds the bit-vector
specifications that SAW (and other tools) can use as the formal
reference for the numerical kernels. Cryptol describes shapes,
ranges, and arithmetic at the bit level so that equivalence
proofs have a precise target.

The pipeline places Cryptol beside SAW: Cryptol writes the
specs, SAW discharges the proofs against LLVM IR produced from
the C11 kernels.

Why it is not built here
------------------------
The Cryptol toolchain is not installed on this machine. No
Cryptol specification exists for any kernel. Nothing is
substituted for it: the English contracts in src/c11/CONTRACT*.md
are not presented as Cryptol, and no other specification
language is offered in its place.

What stands in for it
---------------------
Nothing. The numerical contracts live in plain Markdown under
src/c11/. They are readable by humans and by the test programs,
but they are not machine-checkable Cryptol. The CBQN reference
programs serve as the executable oracle for the C kernels;
they are not Cryptol specs.

What would unblock it
---------------------
A working Cryptol installation on a machine that runs this
repository, followed by specs written for each of the seven
kernel families. Until then the Cryptol layer stays a documented
boundary, not a silent substitution.

Status: UNVERIFIED (toolchain missing).
See verification/VERIFICATION_STATUS.md.
