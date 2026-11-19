# complex_rotate.bqn — CBQN reference for the avian_complex_rotate_f32 kernel.
#
# ComplexRotateFn is the reference: dyadic, 𝕨 is the rotation
# ⟨rot_re, rot_im⟩, 𝕩 is the flat input array of 2N values
# (re0, im0, re1, im1, ...). Result is the flat output array of
# 2N values:
#   out_re[i] = in_re[i]*rot_re - in_im[i]*rot_im
#   out_im[i] = in_re[i]*rot_im + in_im[i]*rot_re
# for each complex number i in [0, n). This is the oracle the C
# kernel must match: K_CBQN(x) ≡ K_C(x) wherever the values are
# exactly representable in f32.
#
# The accumulation is grouped per component, matching the C
# kernel's two-step fused multiply-add order:
#   out_re = (in_re*rot_re) + ((-in_im)*rot_im)
#   out_im = (in_re*rot_im) + (in_im*rot_re)
# The C kernel computes each component as two fused multiply-adds:
# the inner product first (rounded once), then the outer fused
# multiply-add of the other product plus that rounded inner
# product. CBQN has no fused multiply-add primitive, so the
# reference computes the products and the sum in double precision
# (f64), which is the exact sum rounded to f64.
#
# For the exact fixtures the arithmetic is exact, so the reference
# and the kernel agree bit-exactly. For inexact values the two
# roundings can differ by 1 ULP: the C kernel rounds the inner
# product to f32 before the outer fused multiply-add, while the
# reference keeps the exact inner product. A concrete case, found
# by a probe (see src/c11/CONTRACT_complex.md): input
# (-0.753165364, -2.34717751), rotation
# (0.991736054, -1.76565647). The C kernel gives
# (-4.89125013, -0.997949183); the reference (parsed to f32)
# gives (-4.89125061, -0.997949243). Both components differ by
# exactly 1 ULP. The C test pins the kernel to the two-step
# fused multiply-add value on that case; it does not force the
# reference and the kernel to agree where they genuinely differ.
#
# The real and imaginary parts are extracted from the flat input
# with Select (⊏) on computed index lists, and the output is
# rebuilt by interleaving the two component vectors number by
# number. Pick (⊑) with a scalar index does not work on a
# multi-dimensional array in this CBQN build, so no matrix is
# formed; the parts are taken from the flat list directly.
#
# Usage: cbqn -f complex_rotate.bqn <index>
#   index 0 = small f32 fixture (rotation ⟨0.5, ¯0.5⟩, input
#             ⟨1, 2, ¯1, 0.5, 0, ¯2, 2, ¯4⟩)
#   index 1 = large f32 fixture (100 complex numbers, rotation
#             ⟨0.5, ¯0.25⟩, input ↕200)
# The script prints the fixture to stdout, one value per line:
#   line 1: the complex count N
#   lines 2-3: rot_re, rot_im
#   next 2N lines: the input values (flat, re,im per number)
#   next 2N lines: the expected output values (flat, re,im per number)
# The Makefile redirects stdout into fixtures/arrays/complex_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the products and the sums are parenthesized explicitly. Writing
# re×rre + (-im)×rim without parentheses would parse as a nested
# product, which is wrong. The explicit t0, t1 and the t0+t1
# grouping give the intended sum.

ComplexRotateFn ← {rot𝕊x:
  rre ← ⊑rot
  rim ← 1⊑rot
  n ← (≠x)÷2
  idx ← 2×↕n
  re ← idx⊏x
  im ← (1+idx)⊏x
  t0 ← re×rre
  t1 ← (-im)×rim
  ore ← t0+t1
  u0 ← re×rim
  u1 ← im×rre
  oim ← u0+u1
  ∾{𝕊 i: ⟨i⊑ore, i⊑oim⟩}¨↕n
}

PrintFixture ← {𝕊 rot‿inp:
  exp ← rot ComplexRotateFn inp
  •Out •Fmt (≠inp)÷2
  •Out¨ •Fmt¨ rot
  •Out¨ •Fmt¨ inp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨⟨0.5, ¯0.5⟩‿⟨1, 2, ¯1, 0.5, 0, ¯2, 2, ¯4⟩, ⟨0.5, ¯0.25⟩‿(↕200)⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
