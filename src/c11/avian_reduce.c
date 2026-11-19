/*
 * avian_reduce.c — C11 implementation of the
 * avian_reduce_segments_f32 kernel.
 *
 * The kernel validates first, then computes. The validation pass
 * checks every precondition (null, count, length total, overlap,
 * finiteness of the inputs and of every accumulation) before any
 * output is written, so a failed call leaves the output buffer
 * untouched. There is no allocation anywhere in the kernel. The
 * cost is a second pass over the input; correctness and a clean
 * error semantic are worth it here. This is the same pattern as
 * avian_scale.c and avian_conv1d.c.
 *
 * The length total is checked as it grows: the running total is
 * compared against input.count after each length is added, so it
 * can never exceed input.count and cannot wrap. A negative length
 * is rejected before it is added.
 */

#include "avian_reduce.h"

#include <math.h>
#include <stdint.h>

/* Do the byte ranges [a, a + a_len) and [b, b + b_len) intersect?
 * The caller guarantees a_len and b_len do not make the sums wrap.
 * This is the same check as in avian_scale.c and avian_conv1d.c,
 * duplicated here so each kernel file stays self-contained. */
static int ranges_overlap(const void *a, size_t a_len,
                          const void *b, size_t b_len)
{
    uintptr_t a0 = (uintptr_t)a;
    uintptr_t a1 = a0 + (uintptr_t)a_len;
    uintptr_t b0 = (uintptr_t)b;
    uintptr_t b1 = b0 + (uintptr_t)b_len;
    return a0 < b1 && b0 < a1;
}

avian_status avian_reduce_segments_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_i32_input lengths)
{
    size_t nseg = lengths.count;
    size_t s, i;

    /* Both counts zero: valid, nothing to do, NULL data is fine. */
    if (nseg == 0 && input.count == 0) {
        return AVIAN_OK;
    }
    if (nseg > 0 && lengths.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (input.count > 0 && input.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (nseg > 0 && output.data == NULL) {
        return AVIAN_ERR_NULL;
    }
    if (output.count < nseg) {
        return AVIAN_ERR_COUNT;
    }
    /* Length validation: each length non-negative, and the total
     * of all lengths equals input.count. The running total is
     * compared against input.count as it grows, so it can never
     * exceed input.count and cannot wrap. A negative length is a
     * length error, reported as AVIAN_ERR_COUNT (see the header). */
    {
        size_t total = 0;
        for (s = 0; s < nseg; s++) {
            int32_t len = lengths.data[s];
            if (len < 0) {
                return AVIAN_ERR_COUNT;
            }
            total += (size_t)len;
            if (total > input.count) {
                return AVIAN_ERR_COUNT;
            }
        }
        if (total != input.count) {
            return AVIAN_ERR_COUNT;
        }
    }
    /* Overlap: the written output must not overlap the input or
     * the lengths buffer. Both are read throughout the computation,
     * so writing into either would corrupt the result. An empty
     * range never overlaps, so the guards on the counts are only
     * a short-circuit; ranges_overlap itself is safe on empty
     * ranges. */
    if (nseg > 0 && input.count > 0) {
        if (ranges_overlap(output.data, nseg * sizeof(float),
                           input.data, input.count * sizeof(float))) {
            return AVIAN_ERR_OVERLAP;
        }
    }
    if (nseg > 0) {
        if (ranges_overlap(output.data, nseg * sizeof(float),
                           lengths.data, nseg * sizeof(int32_t))) {
            return AVIAN_ERR_OVERLAP;
        }
    }
    /* Validation pass: every input finite, every accumulation
     * finite. Nothing is written here. An empty segment accumulates
     * nothing and stays +0.0f, which is finite. */
    {
        size_t off = 0;
        for (s = 0; s < nseg; s++) {
            size_t len = (size_t)lengths.data[s];
            float acc = 0.0f;
            for (i = 0; i < len; i++) {
                float x = input.data[off + i];
                if (!isfinite(x)) {
                    return AVIAN_ERR_NONFINITE;
                }
                acc = acc + x;
                if (!isfinite(acc)) {
                    return AVIAN_ERR_NONFINITE;
                }
            }
            off += len;
        }
    }
    /* Compute pass. Same accumulation, now writing the result. */
    {
        size_t off = 0;
        for (s = 0; s < nseg; s++) {
            size_t len = (size_t)lengths.data[s];
            float acc = 0.0f;
            for (i = 0; i < len; i++) {
                acc = acc + input.data[off + i];
            }
            output.data[s] = acc;
            off += len;
        }
    }
    return AVIAN_OK;
}
