/*
 * avian_norm.h — AVIAN-MORPH C11 kernel ABI, normalization family
 * (L2 normalization to a unit vector, f32).
 *
 * The normalization semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h).
 * This header defines the normalization entry point.
 *
 * Contract:
 *   out[i] = in[i] / norm,  norm = sqrt(sum of in[i]^2),
 * for i in [0, n). The input is N f32 values. The output is N
 * f32 values. The caller passes n, the element count.
 *
 * The norm is computed internally: the sum of squares is
 * accumulated left to right with fused multiply-add (fmaf),
 * starting from +0.0f, then norm = sqrtf(sum). The division is
 * one correctly-rounded f32 division per element.
 *
 * Preconditions, checked by the kernel (deterministic check order,
 * same as the scale, convolution, reduction, gather, geometry,
 * and complex kernels):
 *   1. NULL. If n > 0, input.data and output.data must both be
 *      non-NULL, else AVIAN_ERR_NULL. A zero element count is
 *      valid and writes nothing, even if a data pointer is NULL.
 *   2. COUNT. output.count >= n and input.count >= n, else
 *      AVIAN_ERR_COUNT.
 *   3. OVERLAP. The written output range
 *      [output.data, output.data + n) must not overlap the read
 *      input range [input.data, input.data + n), else
 *      AVIAN_ERR_OVERLAP. In-place normalization
 *      (output.data == input.data) is rejected.
 *   4. NONFINITE. Every input element must be finite, the sum of
 *      squares must be finite, and the norm must be finite, else
 *      AVIAN_ERR_NONFINITE. A sum of squares that overflows f32
 *      (norm would be +inf) is rejected here.
 *   5. ZERO NORM. If the computed norm is 0.0f, the kernel
 *      returns AVIAN_ERR_COUNT (a domain error: the input does
 *      not satisfy the precondition that its norm is a
 *      representable positive f32). This happens when the input
 *      is all zeros, and also when every input is nonzero but so
 *      small that the sum of squares underflows f32 to +0.0f
 *      (each square is below the smallest f32 subnormal). In
 *      both cases the f32-computable norm is zero and the
 *      division is undefined.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 *
 * The kernel allocates nothing. It is a pure function of its
 * buffers and its element count.
 */

#ifndef AVIAN_NORM_H
#define AVIAN_NORM_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avian_normalize_f32: L2 normalization to a unit vector
 * (single precision).
 *
 * The sum of squares is accumulated left to right with fused
 * multiply-add (fmaf), starting from +0.0f:
 *   acc = fmaf(in[i], in[i], acc).
 * A fused multiply-add computes a*b + c with a single rounding
 * (round-to-nearest-even). The norm is sqrtf(acc), the
 * correctly-rounded f32 square root. Each output is one
 * correctly-rounded f32 division, out[i] = in[i] / norm.
 * The accumulation order is fixed, so the kernel is
 * deterministic: the same inputs always give the same outputs
 * and the same status code.
 *
 * The division cannot overflow: every |in[i]| is at most the
 * norm (the sum of squares is at least in[i]^2), so every
 * |out[i]| is at most 1. The norm is checked finite and nonzero
 * before anything is written, so no division by zero and no
 * infinite result is ever produced.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: the inputs are finite, the sum of squares is
 * checked finite, and the norm is checked finite. A zero norm
 * is rejected as AVIAN_ERR_COUNT (see the contract above).
 * A zero input component divided by the (positive) norm keeps
 * its sign: +0.0f / norm is +0.0f and -0.0f / norm is -0.0f
 * (IEEE division sign rules). This differs from the other
 * kernels, where the accumulation from +0.0f forces a zero
 * result to +0.0f; here the output is a direct division, so
 * the sign of a zero input survives. A nonzero result keeps
 * its sign.
 */
avian_status avian_normalize_f32(
    avian_f32_output output,
    avian_f32_input input,
    size_t n
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_NORM_H */
