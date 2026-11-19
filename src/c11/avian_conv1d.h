/*
 * avian_conv1d.h — AVIAN-MORPH C11 kernel ABI, receptive-field
 * convolution family (1D, 3-tap, f32).
 *
 * The convolution semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h) and a
 * 3-element kernel. This header defines the kernel description and
 * the convolution entry point.
 *
 * Contract:
 *   output[i] = k0*input[i-1] + k1*input[i] + k2*input[i+1],
 *   for i in [0, input.count).
 * Out-of-range inputs (i-1 < 0 or i+1 >= count) read as 0 (zero
 * padding). Output length equals input length.
 *
 * Preconditions, checked by the kernel (deterministic check order,
 * same as the scale kernel):
 *   1. If input.count > 0, input.data and output.data are non-NULL.
 *   2. output.count >= input.count (the output holds the result).
 *   3. The written output range [output.data, output.data + input.count)
 *      does not overlap the read input range
 *      [input.data, input.data + input.count).
 *   4. All three kernel coefficients are finite, every input element
 *      is finite, and every accumulated output is finite.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 * A zero count is valid and writes nothing.
 *
 * The kernel allocates nothing. It is a pure function of its buffers
 * and its kernel coefficients.
 */

#ifndef AVIAN_CONV1D_H
#define AVIAN_CONV1D_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A 3-tap convolution kernel: coefficients k0, k1, k2. */
typedef struct {
    float k0;
    float k1;
    float k2;
} avian_conv1d_kernel_f32;

/*
 * avian_conv1d_f32: 1D convolution with a 3-element kernel and zero
 * padding at the edges (single precision).
 *
 * Each tap is a fused multiply-add (fmaf), accumulated left to right:
 *   acc = fmaf(k0, xm, 0.0f);
 *   acc = fmaf(k1, x0, acc);
 *   acc = fmaf(k2, xp, acc);
 * where xm is input[i-1] (or 0.0f at i == 0), x0 is input[i], and xp
 * is input[i+1] (or 0.0f at i == count-1). A fused multiply-add
 * computes k*x + acc with a single rounding (round-to-nearest-even),
 * so each tap contributes one correctly-rounded multiply-add.
 * The accumulation order is fixed (k0, then k1, then k2), so the
 * kernel is deterministic: the same inputs always give the same
 * outputs and the same status code.
 *
 * There is no cross-element reduction beyond the 3-tap window:
 * output[i] depends only on input[i-1], input[i], and input[i+1].
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: the coefficients and the inputs are finite, and an
 * accumulation that overflows to infinity (or produces NaN) is
 * rejected before anything is written. Signed zero is preserved
 * through the accumulation in the usual IEEE 754 way; a window
 * of all zeros produces +0.0f.
 */
avian_status avian_conv1d_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_conv1d_kernel_f32 kernel
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_CONV1D_H */
