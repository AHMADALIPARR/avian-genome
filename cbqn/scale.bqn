# scale.bqn — CBQN reference for the avian_scale transform.
#
# ScaleFn is the reference: dyadic, 𝕨 is the scale, 𝕩 is the array,
# result is 𝕨 × 𝕩 elementwise. This is the oracle the C kernel must
# match: K_CBQN(x) ≡ K_C(x) for the integer kernel (bit-exact), and
# for the float kernel wherever the product is exactly representable.
#
# Usage: cbqn -f scale.bqn <index>
#   index 0 = f32 fixture (scale 2.5, input 1..8)
#   index 1 = i32 fixture (scale 3, input with negatives)
#   index 2 = f32 large fixture (scale 0.5, input 0..999)
# The script prints the fixture to stdout, one value per line:
#   line 1: the scale
#   line 2: the element count
#   next count lines: the input values
#   next count lines: the expected output values
# The Makefile redirects stdout into fixtures/arrays/scale_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is documented in
# src/c11/CONTRACT.md; it is a formatting artifact, not a value change.

ScaleFn ← ×

PrintFixture ← {𝕊 sc‿inp:
  exp ← sc ScaleFn inp
  •Out •Fmt sc
  •Out •Fmt ≠inp
  •Out¨ •Fmt¨ inp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨2.5‿⟨1, 2, 3, 4, 5, 6, 7, 8⟩, 3‿⟨0, 1, 2, 3, 4, ¯5, 100, ¯100⟩, 0.5‿(↕1000)⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
