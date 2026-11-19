/*
 * avian_norm.c — C11 implementation of the avian_normalize_f32
 * kernel.
 *
 * The kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, overlap, finiteness of
 * the inputs, the sum of squares, and the norm, and a nonzero
 * norm) before any output is written, so a failed call leaves
 * the output buffer untouched. There is no allocation anywhere
 * in the kernel. The cost is a second pass over the input;
 * correctness and a clean error semantic are worth it here.
 * This is the same pattern as avian_scale.c, avian_conv1d.c,
 * avian_reduce.c, avian_gather.c, avian_geom.c, and
 * avian_complex.c.
 *
 * The accumulation uses fmaf (fused multiply-add) left to
 * right from +0.0f for the sum of squares, then sqrtf for the
 * norm, then one f32 division per element. The CBQN reference
 * computes the sum, the square root, and the division in double
 * precision (f64), so on inexact values the two can differ by
 * 1 ULP: the C kernel's norm is sqrtf (the correctly-rounded
 * f32 square root), while the reference's norm is the f64 square
 * root, and the division then rounds a quotient computed from
 * the f32-rounded norm. See src/c11/CONTRACT_norm.md for the
 * concrete case. On the exact fixtures the arithmetic is exact,
 * so the CBQN (f64) result and the C (f32) result agree
 * bit-exactly.
 */

#include "avian_norm.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c, avian_conv1d.c,
 * avian_reduce.c, avian_gather.c, avian_geom.c, and
 * avian_complex.c, duplicated here so each kernel file stays
 * self-contained. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

avian_status avian_normalize_f32(
    avian_f32_output output,
    avian_f32_input input,
    size_t n)
{
    size_t i;
    float sum;
    float norm;

    if (n == 0) {
        return AVIAN_OK;
    }
    if (input.data == NULL || output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count < n || input.count < n) {
        return AVIAN_ERR_COUNT;
    }
    if (ranges_overlap(output.data, n * sizeof(float),
                       input.data, n * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    /* Validation pass: every input finite, the sum of squares
     * finite, the norm finite and nonzero, and every quotient
     * finite. Nothing is written here. The sum of squares is a
     * sum of non-negative terms, so it cannot become NaN from
     * finite inputs (no +inf plus -inf); it can only overflow
     * to +inf, which the finiteness check catches. The norm is
     * the square root of a finite non-negative sum, so it is
     * finite whenever the sum is; the explicit check names the
     * cause. A zero norm means the f32-computable norm is zero:
     * either the input is all zeros, or every square underflowed
     * f32 to +0.0f. Both are the same domain error
     * (AVIAN_ERR_COUNT). Every |in[i]| is at most the norm, so
     * every quotient is at most 1 in magnitude and cannot
     * overflow; the explicit check proves it rather than
     * assuming it. */
    sum = 0.0f;
    for (i = 0; i < n; i++) {
        if (!isfinite(input.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
        sum = fmaf(input.data[i], input.data[i], sum);
    }
    if (!isfinite(sum)) {
        return AVIAN_ERR_NONFINITE;
    }
    norm = sqrtf(sum);
    if (!isfinite(norm)) {
        return AVIAN_ERR_NONFINITE;
    }
    if (norm == 0.0f) {
        return AVIAN_ERR_COUNT;
    }
    for (i = 0; i < n; i++) {
        float q = input.data[i] / norm;
        if (!isfinite(q)) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass. Same arithmetic, now writing the result.
     * The validation pass already proved the norm finite and
     * nonzero and every quotient finite, so the divisions here
     * cannot fail. */
    for (i = 0; i < n; i++) {
        output.data[i] = input.data[i] / norm;
    }
    return AVIAN_OK;
}
