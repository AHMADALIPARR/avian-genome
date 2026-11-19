# conv1d.bqn — CBQN reference for the avian_conv1d_f32 transform.
#
# ConvFn is the reference: dyadic, 𝕨 is the 3-element kernel
# ⟨k0, k1, k2⟩, 𝕩 is the input array, result is the 1D convolution
# with zero padding at the edges:
#   output[i] = k0*input[i-1] + k1*input[i] + k2*input[i+1]
# where out-of-range inputs (i-1 < 0 or i+1 >= count) read as 0.
# Output length equals input length. This is the oracle the C
# kernel must match: K_CBQN(x) ≡ K_C(x) wherever the values are
# exactly representable in f32.
#
# The accumulation is left to right: ((k0*xm) + (k1*x)) + (k2*xp),
# matching the C kernel, which accumulates with fused multiply-add
# in the same order. For the exact fixtures the order does not
# matter (the arithmetic is exact), but the reference and the
# kernel agree on the order anyway.
#
# Usage: cbqn -f conv1d.bqn <index>
#   index 0 = small f32 fixture (kernel ⟨0.5, 1, ¯0.25⟩, input 1,2,4,8,16)
#   index 1 = large f32 fixture (kernel ⟨0.5, 1, 0.5⟩, input 0..999)
# The script prints the fixture to stdout, one value per line:
#   lines 1-3: k0, k1, k2
#   line 4: the element count
#   next count lines: the input values
#   next count lines: the expected output values
# The Makefile redirects stdout into fixtures/arrays/conv1d_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the products and the sums are parenthesized explicitly. Writing
# (⊑k)×xm + (1⊑k)×x + (2⊑k)×xp without parentheses parses as
# k0*(xm + k1*(x + k2*xp)), which is wrong. The explicit t0, t1, t2
# and the (t0+t1)+t2 grouping give the intended left-to-right sum.

ConvFn ← {k𝕊x:
  n ← ≠x
  xm ← n↑(0∾x)
  xp ← (-n)↑(x∾0)
  t0 ← (⊑k)×xm
  t1 ← (1⊑k)×x
  t2 ← (2⊑k)×xp
  (t0+t1)+t2
}

PrintFixture ← {𝕊 k‿inp:
  exp ← k ConvFn inp
  •Out¨ •Fmt¨ k
  •Out •Fmt ≠inp
  •Out¨ •Fmt¨ inp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨⟨0.5, 1, ¯0.25⟩‿⟨1, 2, 4, 8, 16⟩, ⟨0.5, 1, 0.5⟩‿(↕1000)⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
