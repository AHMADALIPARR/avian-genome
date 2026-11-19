# gather_scatter.bqn — CBQN reference for the avian_gather_f32 and
# avian_scatter_f32 transforms.
#
# Gather is the reference for gather: dyadic, 𝕨 is the index array,
# 𝕩 is the input array, result is the input indexed by the indices:
#   output[i] = input[index[i]], for i in [0, index count).
# Output length equals index count. This is the oracle the C kernel
# must match: K_CBQN(x) ≡ K_C(x) wherever the values are exactly
# representable in f32. Gather copies values only; there is no
# arithmetic, so the comparison is bit-exact for any finite input.
#
# Scatter is the reference for scatter: m is the output count, 𝕨 is
# the output count and 𝕩 is the input and index arrays as a pair.
# Result is an m-element array, all +0.0 to start, then each input
# element is written to its index:
#   output[index[i]] = input[i], for i in [0, input count).
# If two input elements target the same index, the LAST one in input
# order wins. An index that is never written stays +0.0. This is
# the oracle the C kernel must match, bit-exact for any finite input.
# A valid scatter call has the input and index arrays the same
# length. The reference takes the first ≠idx elements of the input
# (xx ← (≠idx)↑x); for a valid call that is the whole input, so it
# changes nothing. It exists only so the fixture printer can compute
# both the gather and the scatter result for a record without a
# conditional (this build cannot dispatch on a function by index,
# and ? cannot appear at the top level); the unused result is
# discarded. It never changes a valid scatter result.
#
# The scatter implementation finds, for each output position j, the
# last input position whose index equals j. occ is the occurrence
# index counted from the end (0 marks the last occurrence of a
# value), computed by reversing, taking the progressive occurrence
# index, and reversing back. last marks exactly the last occurrence
# of each distinct index value. For output position j, the result is
# the sum of input[i] over the i that are a last occurrence and have
# index[i] equal to j; at most one i satisfies both, so the sum is
# exactly that input value, or 0 when no input targets j. There is
# no conditional in the reference (CBQN ? conditionals have a
# scoping quirk in this build); the "no input targets j" case falls
# out of the sum being empty of contributors.
#
# Usage: cbqn -f gather_scatter.bqn <index>
#   index 0 = gather small (input ⟨1,2,4,8,16⟩, indices ⟨3,0,3,1⟩)
#   index 1 = scatter small (input ⟨1,2,4,8,16⟩, indices ⟨0,2,2,4,1⟩,
#             output count 5; index 2 is a duplicate, index 3 is
#             never written)
#   index 2 = gather large (input ↕1000, indices ⌽↕100)
#   index 3 = scatter large (input ↕1000, indices (↕500)∾(↕500),
#             output count 1000)
# The script prints the fixture to stdout, one value per line:
#   line 1: the input count
#   next count lines: the input values
#   next line: the index count
#   next count lines: the index values
#   next line: the output count
#   next count lines: the expected output values
# The Makefile redirects stdout into
# fixtures/arrays/gather_<name>.txt or scatter_<name>.txt.
#
# Note: CBQN formats negative numbers with the high-minus character
# (U+00AF, UTF-8 0xC2 0xAF) instead of ASCII '-'. The C test normalizes
# that byte sequence to '-' before parsing. This is a formatting
# artifact, not a value change.
#
# Note: BQN evaluates right to left with no operator precedence, so
# the products and the sums are parenthesized explicitly.

Gather ← {idx𝕊x: idx⊏x}

Scatter ← {m𝕊x‿idx:
  xx ← (≠idx)↑x
  occ ← ⌽⊒⌽idx
  last ← occ=0
  {𝕊 j: +´(last×(idx=j))×xx}¨↕m
}

PrintFixture ← {𝕊 inp‿idx‿oc‿kind:
  gexp ← idx Gather inp
  sexp ← oc Scatter inp‿idx
  exp ← kind⊑⟨gexp, sexp⟩
  •Out •Fmt ≠inp
  •Out¨ •Fmt¨ inp
  •Out •Fmt ≠idx
  •Out¨ •Fmt¨ idx
  •Out •Fmt ≠exp
  •Out¨ •Fmt¨ exp
}

datas ← ⟨⟨1,2,4,8,16⟩‿⟨3,0,3,1⟩‿4‿0, ⟨1,2,4,8,16⟩‿⟨0,2,2,4,1⟩‿5‿1, (↕1000)‿(⌽↕100)‿100‿0, (↕1000)‿((↕500)∾(↕500))‿1000‿1⟩
idx ← •BQN ⊑•args
PrintFixture idx⊑datas
