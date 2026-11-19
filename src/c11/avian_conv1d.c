/*
 * avian_conv1d.c — C11 implementation of the avian_conv1d_f32 kernel.
 *
 * The kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, overlap, finiteness of
 * the coefficients, the inputs, and every accumulation) before any
 * output is written, so a failed call leaves the output buffer
 * untouched. There is no allocation anywhere in the kernel. The
 * cost is a second pass over the input; correctness and a clean
 * error semantic are worth it here. This is the same pattern as
 * avian_scale.c.
 *
 * The accumulation uses fmaf (fused multiply-add) in the fixed
 * order k0, k1, k2, matching the CBQN reference, which accumulates
 * left to right. For the exact fixtures the arithmetic is exact,
 * so the CBQN (f64) result and the C (f32, fmaf) result agree
 * bit-exactly.
 */

#include "avian_conv1d.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c, duplicated here so
 * each kernel file stays self-contained. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

avian_status avian_conv1d_f32(avian_f32_output output,
                              avian_f32_input input,
                              avian_conv1d_kernel_f32 kernel)
{
    size_t i;

    if (input.count == 0) {
        return AVIAN_OK;
    }
    if (input.data == NULL || output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count < input.count) {
        return AVIAN_ERR_COUNT;
    }
    if (ranges_overlap(output.data, input.count * sizeof(float),
                       input.data, input.count * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    if (!isfinite(kernel.k0) || !isfinite(kernel.k1) ||
        !isfinite(kernel.k2)) {
        return AVIAN_ERR_NONFINITE;
    }
    /* Validation pass: every input finite, every accumulation finite.
     * Nothing is written here. Every input element is checked as the
     * centre tap of its own output, so all inputs are covered; a
     * non-finite neighbour is also caught here because it makes the
     * accumulation non-finite. A non-finite intermediate makes the
     * final accumulation non-finite (infinity plus a finite value
     * stays infinity, infinity plus minus-infinity is NaN), so
     * checking after each fused multiply-add is sufficient. */
    for (i = 0; i < input.count; i++) {
        float xm = (i == 0) ? 0.0f : input.data[i - 1];
        float x0 = input.data[i];
        float xp = (i + 1 == input.count) ? 0.0f : input.data[i + 1];
        float acc;
        if (!isfinite(x0)) {
            return AVIAN_ERR_NONFINITE;
        }
        acc = fmaf(kernel.k0, xm, 0.0f);
        if (!isfinite(acc)) {
            return AVIAN_ERR_NONFINITE;
        }
        acc = fmaf(kernel.k1, x0, acc);
        if (!isfinite(acc)) {
            return AVIAN_ERR_NONFINITE;
        }
        acc = fmaf(kernel.k2, xp, acc);
        if (!isfinite(acc)) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass. Same accumulation, now writing the result. */
    for (i = 0; i < input.count; i++) {
        float xm = (i == 0) ? 0.0f : input.data[i - 1];
        float x0 = input.data[i];
        float xp = (i + 1 == input.count) ? 0.0f : input.data[i + 1];
        float acc = fmaf(kernel.k0, xm, 0.0f);
        acc = fmaf(kernel.k1, x0, acc);
        acc = fmaf(kernel.k2, xp, acc);
        output.data[i] = acc;
    }
    return AVIAN_OK;
}
