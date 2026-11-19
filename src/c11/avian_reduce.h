/*
 * avian_reduce.h — AVIAN-MORPH C11 kernel ABI, segmented reduction
 * family (dendritic_reduce, f32).
 *
 * The reduction semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h) and a
 * segment-length list. This header defines the reduction entry
 * point. The segment lengths reuse avian_i32_input from
 * avian_array.h: data is the length list, count is the number of
 * segments.
 *
 * Contract:
 *   output[s] = sum of input[off[s] .. off[s]+len[s]-1],
 *   for s in [0, lengths.count),
 * where off is the exclusive prefix sum of the lengths and len[s]
 * is lengths.data[s]. Each segment is accumulated left to right
 * in f32, starting from +0.0f. An empty segment (length 0) sums
 * to +0.0f. The total of all segment lengths must equal
 * input.count. There is no cross-segment accumulation.
 *
 * Preconditions, checked by the kernel (deterministic check order,
 * same as the scale and convolution kernels):
 *   1. If lengths.count == 0 and input.count == 0, the call is
 *      valid and writes nothing, even if a data pointer is NULL.
 *   2. If lengths.count > 0, lengths.data must be non-NULL; if
 *      input.count > 0, input.data must be non-NULL; if
 *      lengths.count > 0, output.data must be non-NULL; else
 *      AVIAN_ERR_NULL.
 *   3. output.count >= lengths.count (the output holds one sum
 *      per segment), else AVIAN_ERR_COUNT.
 *   4. Every segment length must be non-negative, and the total
 *      of all lengths must equal input.count, else
 *      AVIAN_ERR_COUNT. A negative length is a length error and
 *      is reported with AVIAN_ERR_COUNT; the status enum has no
 *      separate invalid-value code, and the length is part of
 *      the count contract.
 *   5. The written output range
 *      [output.data, output.data + lengths.count) must not
 *      overlap the read input range
 *      [input.data, input.data + input.count) or the read
 *      lengths range
 *      [lengths.data, lengths.data + lengths.count),
 *      else AVIAN_ERR_OVERLAP. In-place reduction
 *      (output.data == input.data) is rejected, and writing
 *      into the lengths buffer is rejected.
 *   6. Every input element must be finite, and every
 *      accumulation must be finite, else AVIAN_ERR_NONFINITE.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 *
 * The kernel allocates nothing. It is a pure function of its
 * buffers and its length list.
 */

#ifndef AVIAN_REDUCE_H
#define AVIAN_REDUCE_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avian_reduce_segments_f32: segmented sum reduction (single
 * precision).
 *
 * Each segment is accumulated left to right in f32, starting
 * from +0.0f:
 *   acc = 0.0f;
 *   acc = acc + input[off + 0];
 *   acc = acc + input[off + 1];
 *   ...
 * Each addition is one correctly-rounded IEEE 754 single-precision
 * add (round-to-nearest-even). The accumulation order is fixed,
 * so the kernel is deterministic: the same inputs always give the
 * same outputs and the same status code.
 *
 * There is no cross-segment accumulation: output[s] depends only
 * on the input elements of segment s.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: the inputs are finite, and an accumulation that
 * overflows to infinity (or produces NaN) is rejected before
 * anything is written. A zero sum is +0.0f: the accumulation
 * starts from +0.0f, and adding a zero of either sign to +0.0f
 * gives +0.0f, so the sign of a zero input does not survive the
 * sum. An empty segment therefore sums to +0.0f.
 */
avian_status avian_reduce_segments_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_i32_input lengths
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_REDUCE_H */
