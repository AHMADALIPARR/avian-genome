# reduce_segments.bqn — CBQN reference for the avian_reduce_segments_f32 transform.
#
# SegSum is the reference: dyadic, 𝕨 is the segment-length list,
# 𝕩 is the input array, result is one sum per segment, in order.
#   output[s] = sum of input[off[s] .. off[s]+len[s]-1]
# where off is the exclusive prefix sum of the lengths. An empty
# segment (length 0) sums to 0. This is the oracle the C kernel
# must match: K_CBQN(x) ≡ K_C(x) wherever the values are exactly
# representable in f32.
#
# The accumulation is left to right, starting from 0, matching the
# C kernel. For the exact fixtures the arithmetic is exact, so the
# order does not matter, but the reference and the kernel agree on
# the order anyway.
#
# Usage: cbqn -f reduce_segments.bqn <index>
#   index 0 = small f32 fixture (lengths ⟨2, 0, 3⟩, input ⟨1, 2, 4, 8, 16⟩)
#   index 1 = large f32 fixture (lengths ⟨250, 250, 250, 250⟩, input ↕1000)
# The script prints the fixture to stdout, one value per line:
#   line 1: the number of segments
#   next nseg lines: the segment lengths
#   next line: the input count
#   next count lines: the input values
#   next nseg lines: the expected output values (one sum per segment)
# The Makefile redirects stdout into fixtures/arrays/reduce_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the slice and the sum are parenthesized explicitly.

SegSum ← {l𝕊x:
  off ← 0∾(¯1↓(+`l))
  {𝕊 s: +´((s⊑l)↑((s⊑off)↓x))}¨↕≠l
}

PrintFixture ← {𝕊 l‿inp:
  exp ← l SegSum inp
  •Out •Fmt ≠l
  •Out¨ •Fmt¨ l
  •Out •Fmt ≠inp
  •Out¨ •Fmt¨ inp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨⟨2, 0, 3⟩‿⟨1, 2, 4, 8, 16⟩, ⟨250, 250, 250, 250⟩‿(↕1000)⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
