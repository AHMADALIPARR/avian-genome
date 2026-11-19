/*
 * avian_gather.h — AVIAN-MORPH C11 kernel ABI, sparse gather/scatter
 * family (synaptic_sparse, f32).
 *
 * The gather/scatter semantics live in q and CBQN. The C kernel only
 * consumes validated buffer descriptions (see avian_array.h) and an
 * index array. This header defines the gather and scatter entry
 * points. The index array reuses avian_i32_input from avian_array.h:
 * data is the index list, count is the number of indices.
 *
 * Contract (gather):
 *   output[i] = input[index[i]], for i in [0, index.count).
 * Output length equals index count. Each index must be a
 * non-negative integer less than input.count. Gather copies values
 * only; there is no arithmetic, so the comparison with the CBQN
 * reference is bit-exact for any finite input.
 *
 * Contract (scatter):
 *   output[index[i]] = input[i], for i in [0, input.count).
 * The output starts as all +0.0f, then each input element is
 * written to its index. If two input elements target the same
 * index, the LAST one in input order wins. An index that is never
 * written stays +0.0f. Output length is given by the caller and
 * must be at least 1 when there is input to scatter. Each index
 * must be a non-negative integer less than output.count.
 *
 * Preconditions, checked by the kernels (deterministic check order,
 * same as the scale, convolution, and reduction kernels):
 *   1. NULL. A data pointer must be non-NULL when its count is
 *      greater than zero. For gather, if index.count > 0 then
 *      indices.data and output.data must be non-NULL, and if
 *      input.count > 0 then input.data must be non-NULL. For
 *      scatter, if input.count > 0 then input.data and
 *      indices.data must be non-NULL, and if output.count > 0
 *      then output.data must be non-NULL. Else AVIAN_ERR_NULL.
 *      An empty gather (index.count == 0) is valid and writes
 *      nothing, even if a data pointer is NULL. An empty scatter
 *      (input.count == 0 and output.count == 0) is valid and
 *      writes nothing, even if a data pointer is NULL.
 *   2. COUNT. For gather, output.count >= index.count (the
 *      output holds one value per index), else AVIAN_ERR_COUNT.
 *      For scatter, indices.count must equal input.count (each
 *      input element needs exactly one index), else
 *      AVIAN_ERR_COUNT.
 *   3. INDEX. Every index must be in range, else AVIAN_ERR_COUNT.
 *      The status enum has no separate invalid-index code, and
 *      an index is part of the count contract: for gather the
 *      index must satisfy 0 <= index < input.count; for scatter
 *      it must satisfy 0 <= index < output.count. A negative
 *      index is out of range. (For scatter, an output.count of
 *      0 with input to scatter is caught here, because every
 *      index is then out of range.)
 *   4. OVERLAP. The written output range must not overlap the
 *      read input range or the read indices range, else
 *      AVIAN_ERR_OVERLAP. Both the input and the indices are
 *      read throughout the computation, so writing into either
 *      would corrupt the result. For gather the written range
 *      is [output.data, output.data + index.count); for scatter
 *      it is [output.data, output.data + output.count)
 *      (the zero fill writes the whole output).
 *   5. NONFINITE. Every input element must be finite, else
 *      AVIAN_ERR_NONFINITE. Gather and scatter copy values
 *      only, so a non-finite value can only enter the output by
 *      being copied from a non-finite input; it is rejected
 *      before anything is written.
 *
 * On any failed check the kernel writes nothing and returns the
 * corresponding error code (see avian_status in avian_array.h).
 * For scatter, "writes nothing" includes the zero fill: a failed
 * scatter call leaves the output buffer untouched, it does not
 * zero it and then fail.
 *
 * The kernels allocate nothing. They are pure functions of their
 * buffers and their index array.
 */

#ifndef AVIAN_GATHER_H
#define AVIAN_GATHER_H

#include "avian_array.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * avian_gather_f32: sparse gather (single precision).
 *
 *   output[i] = input[index[i]], for i in [0, index.count).
 *
 * Gather copies values only. There is no arithmetic, so there is
 * no rounding, no overflow, and no arithmetic that can produce
 * NaN or infinity. A gathered value is bit-identical to the input
 * value it copies, including the sign of zero: gathering -0.0f
 * gives -0.0f, gathering +0.0f gives +0.0f. The kernel is
 * deterministic: the same inputs always give the same outputs
 * and the same status code. The order of the indices determines
 * the order of the output; duplicate indices are allowed and
 * each occurrence copies the same input value.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: every input element must be finite before anything
 * is written.
 */
avian_status avian_gather_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_i32_input indices
);

/*
 * avian_scatter_f32: sparse scatter (single precision).
 *
 *   output[index[i]] = input[i], for i in [0, input.count),
 *   with output starting as all +0.0f.
 *
 * The output is zero-filled first (+0.0f in every element), then
 * each input element is written to its index, in input order, so
 * when two input elements target the same index the LAST one in
 * input order wins. An index that is never written stays +0.0f.
 * Scatter copies values only. There is no arithmetic, so there is
 * no rounding, no overflow, and no arithmetic that can produce
 * NaN or infinity. A scattered value is bit-identical to the
 * input value it copies, including the sign of zero: scattering
 * -0.0f writes -0.0f, scattering +0.0f writes +0.0f. The
 * zero fill writes +0.0f (positive zero). The kernel is
 * deterministic: the same inputs always give the same outputs
 * and the same status code.
 *
 * NaN and infinity are rejected (AVIAN_ERR_NONFINITE), never
 * produced: every input element must be finite before anything
 * is written, including before the zero fill.
 */
avian_status avian_scatter_f32(
    avian_f32_output output,
    avian_f32_input input,
    avian_i32_input indices
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_GATHER_H */
