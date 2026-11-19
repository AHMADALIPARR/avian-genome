# normalize.bqn — CBQN reference for the avian_normalize_f32 kernel.
#
# NormFn is the reference: monadic, 𝕩 is the input vector of N
# f32 values, result is the L2-normalized vector (a unit vector):
#   out[i] = in[i] / norm,  norm = sqrt(sum of in[i]^2)
# for i in [0, N). The norm is computed internally. This is the
# oracle the C kernel must match: K_CBQN(x) ≡ K_C(x) wherever the
# values are exactly representable in f32.
#
# The accumulation is left to right: the sum of squares is
# (+´ x×x), the square root is √, and the division is x÷n. The C
# kernel accumulates the sum of squares with fused multiply-add
# (fmaf) in f32, takes sqrtf, and divides in f32. CBQN computes
# in double precision (f64): the sum of squares, the square root,
# and the division are all f64, cast to f32 on output.
#
# For the exact fixtures the arithmetic is exact, so the reference
# and the kernel agree bit-exactly. For inexact values the two can
# differ by 1 ULP, and the cause is the NORM, not the division:
# the C kernel's norm is sqrtf (the correctly-rounded f32 square
# root), while the reference's norm is the f64 square root
# (essentially exact). The f32 norm differs from the exact norm
# by up to half an f32 ULP, and the division then rounds a
# quotient computed from the f32-rounded norm, while the
# reference rounds a quotient computed from the (essentially
# exact) f64 norm. When the exact quotient sits within ~1 f32
# ULP of a rounding boundary, the two land on adjacent f32
# values. A concrete case, found by a probe (see
# src/c11/CONTRACT_norm.md): the input vector 0..99. The sum of
# squares is exactly 328350 in both paths, and
# sqrtf(328350) = (float)sqrt(328350.0) = 0x440f412c, which is
# 1.376745e-05 SMALLER than the exact sqrt (long double). The
# exact quotient for element 1 sits 5.78e-12 below the midpoint
# between the two adjacent f32 values 0x3ae4bd58 and 0x3ae4bd59,
# while the quotient by the slightly smaller f32 norm sits
# 4.19e-11 above the exact quotient, i.e. above the midpoint.
# So the C kernel gives 0.00174514495 (0x3ae4bd59) and the
# reference (parsed to f32) gives 0.00174514484 (0x3ae4bd58),
# differing by exactly 1 ULP. Element 2 differs the same way:
# C 0.00349028991 (0x3b64bd59), reference 0.00349028967
# (0x3b64bd58). The C test pins the kernel to the f32-norm
# value on this case; it does not force the reference and the
# kernel to agree where they genuinely differ.
#
# The fixtures use values whose norm is exact (a perfect square
# sum of squares), so the fixture comparison is bit-exact. The
# small fixture has two records: ⟨3, ¯4, 0⟩ (norm 5, has a
# negative element) and ⟨1, 0, 0⟩ (already a unit vector, which
# normalizes to itself). The large fixture is one record:
# ⟨3, ¯4, 0, 0⟩ repeated 25 times (100 elements, sum of squares
# 625, norm exactly 25).
#
# Usage: cbqn -f normalize.bqn <index>
#   index 0 = small f32 fixture (two records, see above)
#   index 1 = large f32 fixture (one record, 100 elements)
# The script prints the fixture to stdout, one value per line.
# The fixture is a sequence of records, back to back until EOF;
# each record is:
#   line 1: the element count N
#   next N lines: the input values
#   next N lines: the expected output values
# The Makefile redirects stdout into fixtures/arrays/norm_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the sum of squares is written (+´ x×x) with the product
# parenthesized by the fold, and the division is x÷n with n the
# scalar norm. The explicit grouping gives the intended order.

NormFn ← {𝕊 x:
  ss ← +´ x×x
  n ← √ss
  x÷n
}

PrintRecord ← {𝕊 v:
  exp ← NormFn v
  •Out •Fmt ≠v
  •Out¨ •Fmt¨ v
  •Out¨ •Fmt¨ exp
}

PrintFixture ← {𝕊 vecs:
  PrintRecord¨ vecs
}

datas ← ⟨⟨⟨3, ¯4, 0⟩, ⟨1, 0, 0⟩⟩, ⟨100⥊⟨3, ¯4, 0, 0⟩⟩⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
