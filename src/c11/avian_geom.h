/*
 * avian_geom.h — AVIAN-MORPH C11 kernel ABI, genomic coordinate
 * transform family (geometry, f32).
 *
 * The transform semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h). This
 * header defines the transform entry point. The matrix and the
 * translation are passed as avian_f32_input buffers so the kernel
 * can validate them (NULL, count, overlap, finiteness) the same
 * way it validates the point buffers.
 *
 * Contract:
 *   out[3i+r] = M[3r+0]*in[3i+0] + M[3r+1]*in[3i+1]
 *             + M[3r+2]*in[3i+2] + t[r],
 *   for i in [0, n) and r in 0..2.
 * The matrix M is 9 f32, row-major: M[0..2] is row 0, M[3..5] is
 * row 1, M[6..8] is row 2. The translation t is 3 f32. The input
 * is N points, each 3 f32 (x, y, z), stored flat (3N values).
 * The output is N points, each 3 f32, stored flat (3N values).
 * The caller passes n, the point count.
 *
 * Preconditions, checked by the kernel (deterministic check order,
 * same as the scale, convolution, reduction, and gather kernels):
 *   1. NULL. If n > 0, input.data, output.data, matrix.data, and
 *      translation.data must all be non-NULL, else AVIAN_ERR_NULL.
 *      A zero point count is valid and writes nothing, even if a
 *      data pointer is NULL.
 *   2. COUNT. output.count >= 3*n and input.count >= 3*n (the
 *      output and the input each hold 3 values per point), and
 *      matrix.count >= 9 and translation.count >= 3, else
 *      AVIAN_ERR_COUNT. If n > SIZE_MAX/3, 3*n cannot be
 *      represented and the kernel returns AVIAN_ERR_COUNT.
 *   3. OVERLAP. The written output range
 *      [output.data, output.data + 3*n) must not overlap the read
 *      input range [input.data, input.data + 3*n), the read matrix
 *      range [matrix.data, matrix.data + 9), or the read translation
 *      range [translation.data, translation.data + 3), else
 *      AVIAN_ERR_OVERLAP. In-place transform
 *      (output.data == input.data) is rejected, and writing into
 *      the matrix or the translation buffer is rejected.
 *   4. NONFINITE. Every input coordinate, every matrix entry, and
 *      every translation entry must be finite, and every product
 *      and every accumulation must be finite, else
 *      AVIAN_ERR_NONFINITE.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 *
 * The kernel allocates nothing. It is a pure function of its
 * buffers and its point count.
 */

#ifndef AVIAN_GEOM_H
#define AVIAN_GEOM_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avian_geom_transform_f32: 3D affine coordinate transform
 * (single precision).
 *
 * Each output component is accumulated left to right with fused
 * multiply-add (fmaf), starting from +0.0f, in the order shown
 * in the contract above (the matrix row term by term, then the
 * translation):
 *   acc = fmaf(M[3r+0], x, 0.0f);
 *   acc = fmaf(M[3r+1], y, acc);
 *   acc = fmaf(M[3r+2], z, acc);
 *   acc = fmaf(1.0f, t[r], acc);
 * The translation step is a fused multiply-add by 1.0f, which is
 * exactly the correctly-rounded sum t[r] + acc (1.0f * t[r] is
 * exact), so it is the same as a plain addition. A fused
 * multiply-add computes a*b + c with a single rounding
 * (round-to-nearest-even), so each term contributes one
 * correctly-rounded multiply-add. The accumulation order is
 * fixed, so the kernel is deterministic: the same inputs always
 * give the same outputs and the same status code.
 *
 * There is no cross-point reduction: output point i depends only
 * on input point i, the matrix, and the translation.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: the inputs, the matrix entries, and the translation
 * entries are finite, and an accumulation that overflows to
 * infinity (or produces NaN) is rejected before anything is
 * written. A zero output is +0.0f: the accumulation starts from
 * +0.0f, and adding a zero of either sign to +0.0f gives +0.0f,
 * so the sign of a zero input coordinate, a zero matrix entry,
 * or a zero translation entry does not survive the sum.
 */
avian_status avian_geom_transform_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_f32_input matrix,
    avian_f32_input translation,
    size_t n
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_GEOM_H */
