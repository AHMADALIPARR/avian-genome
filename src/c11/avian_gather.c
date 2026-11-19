/*
 * avian_gather.c — C11 implementation of the avian_gather_f32 and
 * avian_scatter_f32 kernels.
 *
 * Each kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, index range, overlap,
 * finiteness) before any output is written, so a failed call
 * leaves the output buffer untouched. For scatter, "untouched"
 * includes the zero fill: a failed scatter call does not zero
 * the output. There is no allocation anywhere in the kernels.
 * This is the same pattern as avian_scale.c, avian_conv1d.c,
 * and avian_reduce.c.
 *
 * Both operations copy values only. There is no arithmetic, so
 * there is no rounding and no overflow. The only numerical
 * concern is non-finite input values, which are rejected.
 */

#include "avian_gather.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c, avian_conv1d.c, and
 * avian_reduce.c, duplicated here so each kernel file stays
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

avian_status avian_gather_f32(avian_f32_output output,
                              avian_f32_input input,
                              avian_i32_input indices)
{
    size_t i;

    /* Empty gather: no indices, nothing to read or write. */
    if (indices.count == 0) {
        return AVIAN_OK;
    }
    if (indices.data == NULL || output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (input.count > 0 && input.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    /* Output too small: one output value per index. */
    if (output.count < indices.count) {
        return AVIAN_ERR_COUNT;
    }
    /* Index range: every index must satisfy
     * 0 <= index < input.count. A negative index is out of
     * range. The comparison is done on the int32 value first,
     * so the cast to size_t is only reached for non-negative
     * indices and cannot wrap. */
    for (i = 0; i < indices.count; i++) {
        int32_t idx = indices.data[i];
        if (idx < 0 || (size_t)idx >= input.count) {
            return AVIAN_ERR_COUNT;
        }
    }
    /* Overlap: the written output range must not overlap the
     * read input range or the read indices range. Both are read
     * throughout the computation. */
    if (input.count > 0) {
        if (ranges_overlap(output.data, indices.count * sizeof(float),
                           input.data, input.count * sizeof(float))) {
            return AVIAN_ERR_OVERLAP;
        }
    }
    if (ranges_overlap(output.data, indices.count * sizeof(float),
                       indices.data, indices.count * sizeof(int32_t))) {
        return AVIAN_ERR_OVERLAP;
    }
    /* Validation pass: every input element finite. Nothing is
     * written here. */
    for (i = 0; i < input.count; i++) {
        if (!isfinite(input.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass. */
    for (i = 0; i < indices.count; i++) {
        output.data[i] = input.data[indices.data[i]];
    }
    return AVIAN_OK;
}

avian_status avian_scatter_f32(avian_f32_output output,
                               avian_f32_input input,
                               avian_i32_input indices)
{
    size_t i;

    /* Empty scatter: no input and no output, nothing to do. */
    if (input.count == 0 && output.count == 0) {
        return AVIAN_OK;
    }
    if (input.count > 0 && input.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (input.count > 0 && indices.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count > 0 && output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    /* Count: each input element needs exactly one index. */
    if (indices.count != input.count) {
        return AVIAN_ERR_COUNT;
    }
    /* Index range: every index must satisfy
     * 0 <= index < output.count. A negative index is out of
     * range. An output.count of 0 with input to scatter is
     * caught here, because every index is then out of range. */
    for (i = 0; i < input.count; i++) {
        int32_t idx = indices.data[i];
        if (idx < 0 || (size_t)idx >= output.count) {
            return AVIAN_ERR_COUNT;
        }
    }
    /* Overlap: the written output range (the whole output,
     * because the zero fill writes every element) must not
     * overlap the read input range or the read indices range.
     * Both are read throughout the computation. */
    if (input.count > 0) {
        if (ranges_overlap(output.data, output.count * sizeof(float),
                           input.data, input.count * sizeof(float))) {
            return AVIAN_ERR_OVERLAP;
        }
    }
    if (input.count > 0) {
        if (ranges_overlap(output.data, output.count * sizeof(float),
                           indices.data, indices.count * sizeof(int32_t))) {
            return AVIAN_ERR_OVERLAP;
        }
    }
    /* Validation pass: every input element finite. Nothing is
     * written here, including no zero fill. */
    for (i = 0; i < input.count; i++) {
        if (!isfinite(input.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass: zero fill first, then the writes in input
     * order, so the last writer to an index wins. */
    for (i = 0; i < output.count; i++) {
        output.data[i] = 0.0f;
    }
    for (i = 0; i < input.count; i++) {
        output.data[indices.data[i]] = input.data[i];
    }
    return AVIAN_OK;
}
