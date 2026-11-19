/*
 * avian_complex.c — C11 implementation of the avian_complex_rotate_f32
 * kernel.
 *
 * The kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, overlap, finiteness of
 * the rotation, the inputs, and every accumulation) before any
 * output is written, so a failed call leaves the output buffer
 * untouched. There is no allocation anywhere in the kernel. The
 * cost is a second pass over the input; correctness and a clean
 * error semantic are worth it here. This is the same pattern as
 * avian_scale.c, avian_conv1d.c, avian_reduce.c, avian_gather.c,
 * and avian_geom.c.
 *
 * The accumulation uses fmaf (fused multiply-add) in the fixed
 * order of the contract: the inner product first (rounded once),
 * then the outer fused multiply-add of the other product plus
 * that rounded inner product. The CBQN reference computes the
 * exact sum in double precision, so on inexact values the two
 * can differ by 1 ULP (the reference keeps the exact inner
 * product; the kernel rounds it first). See
 * src/c11/CONTRACT_complex.md for the concrete case. On the
 * exact fixtures the arithmetic is exact, so the CBQN (f64)
 * result and the C (f32, fmaf) result agree bit-exactly.
 */

#include "avian_complex.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c, avian_conv1d.c,
 * avian_reduce.c, avian_gather.c, and avian_geom.c, duplicated here
 * so each kernel file stays self-contained. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

/* One output component: out = fmaf(a, b, fmaf(c, d, 0.0f)).
 * The inner fused multiply-add has a zero addend, so it is exactly
 * the correctly-rounded product c*d. Returns 0 when every step is
 * finite, 1 otherwise. */
static int accumulate_component(float *acc_out,
                                float a, float b, float c, float d)
{
    float inner = fmaf(c, d, 0.0f);
    float acc;
    if (!isfinite(inner)) {
        return 1;
    }
    acc = fmaf(a, b, inner);
    if (!isfinite(acc)) {
        return 1;
    }
    *acc_out = acc;
    return 0;
}

avian_status avian_complex_rotate_f32(
    avian_f32_output output,
    avian_f32_input input,
    float rot_re,
    float rot_im,
    size_t n)
{
    size_t m;
    size_t i;

    if (n == 0) {
        return AVIAN_OK;
    }
    /* 2*n must be representable: if it overflows, the output
     * cannot hold 2*n values, so this is a count error. */
    if (n > SIZE_MAX / 2) {
        return AVIAN_ERR_COUNT;
    }
    m = 2 * n;
    if (input.data == NULL || output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count < m || input.count < m) {
        return AVIAN_ERR_COUNT;
    }
    if (ranges_overlap(output.data, m * sizeof(float),
                       input.data, m * sizeof(float))) {
        return AVIAN_ERR_OVERLAP;
    }
    if (!isfinite(rot_re) || !isfinite(rot_im)) {
        return AVIAN_ERR_NONFINITE;
    }
    /* Validation pass: every input component finite, and every
     * accumulation finite. Nothing is written here. Every input
     * component is checked explicitly; a non-finite component
     * would also make its accumulation non-finite, but the
     * explicit check names the cause. A non-finite intermediate
     * makes the final accumulation non-finite (infinity plus a
     * finite value stays infinity, infinity plus minus-infinity
     * is NaN), so checking after each fused multiply-add is
     * sufficient. */
    for (i = 0; i < n; i++) {
        float re = input.data[2 * i + 0];
        float im = input.data[2 * i + 1];
        float acc;
        if (!isfinite(re) || !isfinite(im)) {
            return AVIAN_ERR_NONFINITE;
        }
        if (accumulate_component(&acc, re, rot_re, -im, rot_im)) {
            return AVIAN_ERR_NONFINITE;
        }
        if (accumulate_component(&acc, re, rot_im, im, rot_re)) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass. Same accumulation, now writing the result.
     * The validation pass already proved every accumulation
     * finite, so accumulate_component always returns 0 and sets
     * acc here. The initializer only silences the compiler's
     * flow analysis (it cannot prove the helper always writes
     * through its pointer); it is never read. */
    for (i = 0; i < n; i++) {
        float re = input.data[2 * i + 0];
        float im = input.data[2 * i + 1];
        float acc = 0.0f;
        accumulate_component(&acc, re, rot_re, -im, rot_im);
        output.data[2 * i + 0] = acc;
        accumulate_component(&acc, re, rot_im, im, rot_re);
        output.data[2 * i + 1] = acc;
    }
    return AVIAN_OK;
}
