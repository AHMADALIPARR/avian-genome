/*
 * avian_complex.h — AVIAN-MORPH C11 kernel ABI, complex rotation
 * family (quantum-layer rotation, f32).
 *
 * The rotation semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h).
 * This header defines the rotation entry point. The rotation is
 * passed as two f32 values, not as an angle: the caller is
 * responsible for forming the rotation (for example from cos and
 * sin, or from a unit-magnitude complex number); the kernel only
 * multiplies. This keeps the C and CBQN implementations
 * bit-exact on exact values: no cos/sin is evaluated inside the
 * kernel, so there is no cross-implementation transcendental
 * difference to reconcile.
 *
 * Contract:
 *   out_re[i] = in_re[i]*rot_re - in_im[i]*rot_im
 *   out_im[i] = in_re[i]*rot_im + in_im[i]*rot_re
 * for i in [0, n). The input is N complex numbers, stored flat
 * as 2N f32 values in interleaved order
 * (re0, im0, re1, im1, ...). The output is N complex numbers,
 * stored flat the same way (2N values). The caller passes n,
 * the complex count.
 *
 * Preconditions, checked by the kernel (deterministic check order,
 * same as the scale, convolution, reduction, gather, and geometry
 * kernels):
 *   1. NULL. If n > 0, input.data and output.data must both be
 *      non-NULL, else AVIAN_ERR_NULL. A zero complex count is
 *      valid and writes nothing, even if a data pointer is NULL.
 *   2. COUNT. output.count >= 2*n and input.count >= 2*n (the
 *      output and the input each hold 2 values per complex
 *      number), else AVIAN_ERR_COUNT. If n > SIZE_MAX/2, 2*n
 *      cannot be represented and the kernel returns
 *      AVIAN_ERR_COUNT.
 *   3. OVERLAP. The written output range
 *      [output.data, output.data + 2*n) must not overlap the read
 *      input range [input.data, input.data + 2*n), else
 *      AVIAN_ERR_OVERLAP. In-place rotation
 *      (output.data == input.data) is rejected.
 *   4. NONFINITE. Both rotation components must be finite, every
 *      input component must be finite, and every product and
 *      every accumulation must be finite, else
 *      AVIAN_ERR_NONFINITE.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 *
 * The kernel allocates nothing. It is a pure function of its
 * buffers, its rotation, and its complex count.
 */

#ifndef AVIAN_COMPLEX_H
#define AVIAN_COMPLEX_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avian_complex_rotate_f32: complex rotation (single precision).
 *
 * Each output component is accumulated with fused multiply-add
 * (fmaf), in the fixed order shown in the contract above. The
 * inner product is computed first (rounded once), then the outer
 * fused multiply-add combines the other product with that
 * rounded inner product:
 *   out_re = fmaf(in_re, rot_re, fmaf(-in_im, rot_im, 0.0f));
 *   out_im = fmaf(in_re, rot_im, fmaf(in_im, rot_re, 0.0f));
 * A fused multiply-add computes a*b + c with a single rounding
 * (round-to-nearest-even). The inner fused multiply-add has a
 * zero addend, so it is exactly the correctly-rounded product.
 * The accumulation order is fixed, so the kernel is
 * deterministic: the same inputs always give the same outputs
 * and the same status code.
 *
 * There is no cross-element reduction: output number i depends
 * only on input number i and the rotation.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: the rotation components and the inputs are finite,
 * and an accumulation that overflows to infinity (or produces
 * NaN) is rejected before anything is written. A zero output
 * component is +0.0f: the inner product of a zero is +0.0f (a
 * zero product plus the +0.0f addend), and the outer fused
 * multiply-add of a zero product plus +0.0f is +0.0f, so the
 * sign of a zero input component or a zero rotation component
 * does not survive the sum.
 */
avian_status avian_complex_rotate_f32(
    avian_f32_output output,
    avian_f32_input input,
    float rot_re,
    float rot_im,
    size_t n
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_COMPLEX_H */
