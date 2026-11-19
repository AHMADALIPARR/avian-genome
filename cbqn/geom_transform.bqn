# geom_transform.bqn — CBQN reference for the avian_geom_transform_f32 transform.
#
# GeomFn is the reference: dyadic, 𝕨 is the matrix (9 values,
# row-major) and translation (3 values) as a pair, 𝕩 is the flat
# input array of 3N values (x,y,z per point). Result is the flat
# output array of 3N values:
#   out[3i+r] = M[3r+0]*in[3i+0] + M[3r+1]*in[3i+1] + M[3r+2]*in[3i+2] + t[r]
# for each point i and each row r in 0..2. This is the oracle the C
# kernel must match: K_CBQN(x) ≡ K_C(x) wherever the values are
# exactly representable in f32.
#
# The accumulation is left to right within each output component:
# ((M[r0]*x + M[r1]*y) + M[r2]*z) + t[r], matching the C kernel,
# which accumulates with fused multiply-add in the same order. For
# the exact fixtures the arithmetic is exact, so the order does not
# matter, but the reference and the kernel agree on the order anyway.
#
# The x, y, z coordinates are extracted from the flat input with
# Select (⊏) on computed index lists, and the output is rebuilt
# by interleaving the three component vectors point by point.
# Pick (⊑) with a scalar index does not work on a multi-dimensional
# array in this CBQN build, so no matrix is formed and no row is
# picked; the coordinates are taken from the flat list directly.
#
# Usage: cbqn -f geom_transform.bqn <index>
#   index 0 = small f32 fixture (matrix ⟨1, 0.5, 0, 0, 2, ¯0.25, 0, 0, 1⟩,
#             translation ⟨1, ¯2, 0.5⟩, points ⟨1,2,4⟩, ⟨¯1,0,2⟩, ⟨0,0,0⟩,
#             ⟨2,¯4,8⟩)
#   index 1 = large f32 fixture (100 points, diagonal scaling matrix
#             ⟨0.5, 0, 0, 0, 2, 0, 0, 0, 1⟩, translation ⟨1, 0, ¯1⟩,
#             input ↕300)
# The script prints the fixture to stdout, one value per line:
#   line 1: the point count N
#   lines 2-10: the matrix M[0..8] (row-major)
#   lines 11-13: the translation t[0..2]
#   next 3N lines: the input values (flat, x,y,z per point)
#   next 3N lines: the expected output values (flat, x,y,z per point)
# The Makefile redirects stdout into fixtures/arrays/geom_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the products and the sums are parenthesized explicitly. Writing
# (⊑m)×xs + (1⊑m)×ys + (2⊑m)×zs + (⊑t) without parentheses would
# parse as a nested product, which is wrong. The explicit ax, ay, az
# and the ((ax+ay)+az)+(⊑t) grouping give the intended left-to-right
# sum.

GeomFn ← {mt𝕊x:
  m ← ⊑mt
  t ← 1⊑mt
  n ← (≠x)÷3
  idx ← 3×↕n
  xs ← idx⊏x
  ys ← (1+idx)⊏x
  zs ← (2+idx)⊏x
  ax ← (⊑m)×xs
  ay ← (1⊑m)×ys
  az ← (2⊑m)×zs
  ox ← ((ax+ay)+az)+(⊑t)
  bx ← (3⊑m)×xs
  by ← (4⊑m)×ys
  bz ← (5⊑m)×zs
  oy ← ((bx+by)+bz)+(1⊑t)
  cx ← (6⊑m)×xs
  cy ← (7⊑m)×ys
  cz ← (8⊑m)×zs
  oz ← ((cx+cy)+cz)+(2⊑t)
  ∾{𝕊 i: ⟨i⊑ox, i⊑oy, i⊑oz⟩}¨↕n
}

PrintFixture ← {𝕊 m‿t‿inp:
  exp ← (m‿t) GeomFn inp
  •Out •Fmt (≠inp)÷3
  •Out¨ •Fmt¨ m
  •Out¨ •Fmt¨ t
  •Out¨ •Fmt¨ inp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨⟨1, 0.5, 0, 0, 2, ¯0.25, 0, 0, 1⟩‿⟨1, ¯2, 0.5⟩‿⟨1, 2, 4, ¯1, 0, 2, 0, 0, 0, 2, ¯4, 8⟩, ⟨0.5, 0, 0, 0, 2, 0, 0, 0, 1⟩‿⟨1, 0, ¯1⟩‿(↕300)⟩
idx ← •BQN ⊑•args
PrintFixture idx ⊑ datas
