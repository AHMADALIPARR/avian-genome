/*
 * avian_scale.c — C11 implementation of the avian_scale kernels.
 *
 * Each kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, overlap, finiteness for
 * f32, range for i32) before any output is written, so a failed
 * call leaves the output buffer untouched. There is no allocation
 * anywhere in the kernel. The cost is a second pass over the input;
 * correctness and a clean error semantic are worth it here.
 */

#include "avian_array.h"

#include <math.h>
#include <stdint.h>

const char *avian_status_name(avian_status s)
{
    switch (s) {
    case AVIAN_OK:            return "AVIAN_OK";
    case AVIAN_ERR_NULL:      return "AVIAN_ERR_NULL";
    case AVIAN_ERR_COUNT:     return "AVIAN_ERR_COUNT";
    case AVIAN_ERR_OVERLAP:   return "AVIAN_ERR_OVERLAP";
    case AVIAN_ERR_NONFINITE: return "AVIAN_ERR_NONFINITE";
    case AVIAN_ERR_OVERFLOW:  return "AVIAN_ERR_OVERFLOW";
    default:                  return "AVIAN_ERR_UNKNOWN";
    }
}

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

avian_status avian_scale_f32(avian_f32_output output,
                             avian_f32_input input,
                             float scale)
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
    if (!isfinite(scale)) {
        return AVIAN_ERR_NONFINITE;
    }
    /* Validation pass: every input finite, every product finite.
     * Nothing is written here. */
    for (i = 0; i < input.count; i++) {
        float p;
        if (!isfinite(input.data[i])) {
            return AVIAN_ERR_NONFINITE;
        }
        p = input.data[i] * scale;
        if (!isfinite(p)) {
            return AVIAN_ERR_NONFINITE;
        }
    }
    /* Compute pass. */
    for (i = 0; i < input.count; i++) {
        output.data[i] = input.data[i] * scale;
    }
    return AVIAN_OK;
}

avian_status avian_scale_i32(avian_i32_output output,
                             avian_i32_input input,
                             int32_t scale)
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
    if (ranges_overlap(output.data, input.count * sizeof(int32_t),
                       input.data, input.count * sizeof(int32_t))) {
        return AVIAN_ERR_OVERLAP;
    }
    /* Validation pass: every product fits in int32. The product is
     * formed in 64 bits, so the check itself cannot overflow, and no
     * narrowing store happens until the product is known to fit. */
    for (i = 0; i < input.count; i++) {
        int64_t p = (int64_t)input.data[i] * (int64_t)scale;
        if (p > INT32_MAX || p < INT32_MIN) {
            return AVIAN_ERR_OVERFLOW;
        }
    }
    /* Compute pass. */
    for (i = 0; i < input.count; i++) {
        output.data[i] = (int32_t)((int64_t)input.data[i] * (int64_t)scale);
    }
    return AVIAN_OK;
}
