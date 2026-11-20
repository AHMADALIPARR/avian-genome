SAW verification layer — boundary
===================================

What the SAW layer is
---------------------
In the AVIAN-MORPH architecture, SAW (Software Analysis
Workbench) holds machine-level contracts: equivalence proofs,
bounds, and arithmetic properties over LLVM IR. SAW verifies
supported LLVM representations against formal specifications
so that the C11 kernels can be shown to match their contracts
at the bit level.

The pipeline places SAW after the C11 kernel ABI. The kernels
are already checked by execution against the CBQN reference;
SAW would add formal equivalence proofs for the same functions.

Why it is not built here
------------------------
The SAW toolchain is not installed on this machine. No SAWScript,
no proof log, and no equivalence result exists for any kernel.
Nothing is substituted for it: no other proof assistant is used,
and no informal argument is presented as a SAW proof.

What stands in for it
---------------------
Nothing. The C11 kernels are verified by execution (make test,
4304 checks, 0 failures) and by bit-exact comparison against
fresh CBQN fixtures. That is empirical evidence, not a formal
proof. The contracts in src/c11/CONTRACT*.md state the intended
semantics; they are not SAW proofs.

What would unblock it
---------------------
A working SAW installation on a machine that runs this
repository, together with LLVM bitcode produced from the C11
kernels under the same flags used for testing. Until then the
SAW layer stays a documented boundary, not a silent substitution.

Status: UNVERIFIED (toolchain missing).
See verification/VERIFICATION_STATUS.md.
