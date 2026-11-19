/*
 * avian_geom.c — C11 implementation of the avian_geom_transform_f32
 * kernel.
 *
 * The kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, overlap, finiteness of
 * the inputs, the matrix, the translation, and every accumulation)
 * before any output is written, so a failed call leaves the output
 * buffer untouched. There is no allocation anywhere in the kernel.
 * The cost is a second pass over the input; correctness and a clean
 * error semantic are worth it here. This is the same pattern as
 * avian_scale.c, avian_conv1d.c, avian_reduce.c, and avian_gather.c.
 *
 * The accumulation uses fmaf (fused multiply-add) in the fixed
 * order M row term by term, then the translation, matching the
 * CBQN reference, which accumulates left to right. For the exact
 * fixtures the arithmetic is exact, so the CBQN (f64) result and
 * the C (f32, fmaf) result agree bit-exactly.
 */

#include "avian_geom.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c, avian_conv1d.c,
 * avian_reduce.c, and avian_gather.c, duplicated here so each
 * kernel file stays self-contained. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

/* One output component: accumulate the row r of the matrix times
 * the point (x, y, z), then add the translation t[r]. The
 * accumulation is left to right with fused multiply-add, starting
 * from +0.0f. Returns 0 when every step is finite, 1 otherwise. */
static int accumulate_component(float *acc_out,
                                const float *m, const float *t,
                                size_t r,
                                float x, float y, float z)
{
    float acc;
    acc = fmaf(m[3 * r + 0], x, 0.0f);
    if (!isfinite(acc)) {
        return 1;
    }
    acc = fmaf(m[3 * r + 1], y, acc);
    if (!isfinite(acc)) {
        return 1;
    }
    acc = fmaf(m[3 * r + 2], z, acc);
    if (!isfinite(acc)) {
        return 1;
    }
    acc = fmaf(1.0f, t[r], acc);
    if (!isfinite(acc)) {
        return 1;
    }
    *acc_out = acc;
    return 0;
}

avian_status avian_geom_transform_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_f32_input matrix,
    avian_f32_input translation,
    size_t n)
{
    size_t m;
    size_t i, r;

    if (n == 0) {
        return AVIAN_OK;
    }
    /* 3*n must be representable: if it overflows, the output
     * cannot hold 3*n values, so this is a count error. */
    if (n > SIZE_MAX / 3) {
        return AVIAN_ERR_COUNT;
    }
    m = 3 * n;
    if (input.data == NULL || output.data == NULL ||
        matrix.data == NULL || translation.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count < m || input.count < m) {
        return AVIAN_ERR_COUNT;
    }
    if (matrix.count < 9 || translation.count < 3) {
        return AVIAN_ERR_COUNT;
    }
    if (ranges_overlap(output.data, m * sizeof(float),
                       input.data, m * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    if (ranges_overlap(output.data, m * sizeof(float),
                       matrix.data, 9 * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    if (ranges_overlap(output.data, m * sizeof(float),
                       translation.data, 3 * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    /* Validation pass: every matrix entry and every translation
     * entry finite, every input coordinate finite, and every
     * accumulation finite. Nothing is written here. Every input
     * coordinate is checked explicitly; a non-finite coordinate
     * would also make its accumulation non-finite, but the
     * explicit check names the cause. A non-finite intermediate
     * makes the final accumulation non-finite (infinity plus a
     * finite value stays infinity, infinity plus minus-infinity
     * is NaN), so checking after each fused multiply-add is
     * sufficient. */
    for (i = 0; i < 9; i++) {
        if (!isfinite(matrix.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    for (i = 0; i < 3; i++) {
        if (!isfinite(translation.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    for (i = 0; i < n; i++) {
        float x = input.data[3 * i + 0];
        float y = input.data[3 * i + 1];
        float z = input.data[3 * i + 2];
        float acc;
        if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
            return AVIAN_ERR_NONFINITE;
        }
        for (r = 0; r < 3; r++) {
            if (accumulate_component(&acc, matrix.data,
                                     translation.data, r, x, y, z)) {
                return AVIAN_ERR_NONFINITE;
            }
        }
    }
    /* Compute pass. Same accumulation, now writing the result. */
    for (i = 0; i < n; i++) {
        float x = input.data[3 * i + 0];
        float y = input.data[3 * i + 1];
        float z = input.data[3 * i + 2];
        float acc;
        for (r = 0; r < 3; r++) {
            accumulate_component(&acc, matrix.data,
                                 translation.data, r, x, y, z);
            output.data[3 * i + r] = acc;
        }
    }
    return AVIAN_OK;
}
