/*
 * avian_array.h — AVIAN-MORPH C11 kernel ABI (small, explicit).
 *
 * The array semantics live in q and CBQN. The C kernel only consumes
 * validated buffer descriptions. This header defines those descriptions
 * and the first kernel family: elementwise scale.
 *
 * Contract (both kernels):
 *   output[i] = input[i] * scale, for i in [0, input.count).
 *
 * Preconditions, checked by the kernel (deterministic check order):
 *   1. If input.count > 0, input.data and output.data are non-NULL.
 *   2. output.count >= input.count (the output holds the result).
 *   3. The written output range [output.data, output.data + input.count)
 *      does not overlap the read input range
 *      [input.data, input.data + input.count).
 *   4. f32 only: scale is finite and every input element is finite.
 *   5. i32 only: every product fits in int32 (checked exactly).
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code. A zero count is valid and writes nothing.
 *
 * The kernel allocates nothing. It is a pure function of its buffers.
 */

#ifndef AVIAN_ARRAY_H
#define AVIAN_ARRAY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Status codes. 0 is success; negative values are errors. */
typedef enum {
    AVIAN_OK            = 0,
    AVIAN_ERR_NULL      = -1,  /* NULL data pointer with count > 0 */
    AVIAN_ERR_COUNT     = -2,  /* output.count < input.count */
    AVIAN_ERR_OVERLAP   = -3,  /* written output overlaps the input */
    AVIAN_ERR_NONFINITE = -4,  /* f32: non-finite scale or input */
    AVIAN_ERR_OVERFLOW  = -5   /* i32: a product does not fit in int32 */
} avian_status;

/* Human-readable name for a status code (for test and log output). */
const char *avian_status_name(avian_status s);

/* Single-precision float buffers. */
typedef struct {
    const float *data;
    size_t count;
} avian_f32_input;

typedef struct {
    float *data;
    size_t count;
} avian_f32_output;

/*
 * avian_scale_f32: output[i] = input[i] * scale (single precision).
 *
 * The multiplication is one correctly-rounded IEEE 754 single-precision
 * multiply per element. NaN and infinity are rejected (AVIAN_ERR_NONFINITE),
 * never produced: the inputs are finite and the product of two finite
 * single-precision values is finite unless it overflows to infinity,
 * in which case the result is +inf/-inf and the kernel reports
 * AVIAN_ERR_NONFINITE (the product is checked for finiteness).
 * Signed zero is preserved: 0.0 * scale is 0.0, -0.0 * scale is -0.0
 * (for finite scale). Rounding is round-to-nearest-even, the default
 * IEEE 754 mode. Reduction order is elementwise and independent; there
 * is no cross-element reduction, so order does not affect the result.
 */
avian_status avian_scale_f32(
    avian_f32_output output,
    avian_f32_input input,
    float scale
);

/* 32-bit signed integer buffers. */
typedef struct {
    const int32_t *data;
    size_t count;
} avian_i32_input;

typedef struct {
    int32_t *data;
    size_t count;
} avian_i32_output;

/*
 * avian_scale_i32: output[i] = input[i] * scale (exact integers).
 *
 * The product is computed exactly (via a 64-bit intermediate) and
 * checked against the int32 range before it is written. If any product
 * does not fit in int32, the kernel writes nothing and returns
 * AVIAN_ERR_OVERFLOW. There is no wrapping and no undefined behaviour:
 * signed overflow is impossible here because the check happens on the
 * 64-bit product before the narrowing store. This is the checked
 * variant; there is no unchecked i32 scale kernel in this ABI.
 */
avian_status avian_scale_i32(
    avian_i32_output output,
    avian_i32_input input,
    int32_t scale
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_ARRAY_H */
