#ifndef AVIAN_GEOM_BRIDGE_H
#define AVIAN_GEOM_BRIDGE_H

/*
 * avian_geom_bridge.h — thin call-through to the verified geom kernel.
 *
 * Matches src/c11/avian_geom.h exactly:
 *   avian_status avian_geom_transform_f32(
 *       avian_f32_output output,
 *       avian_f32_input  input,
 *       avian_f32_input  matrix,       // 9 f32, row-major 3x3
 *       avian_f32_input  translation,  // 3 f32
 *       size_t n);                     // point count
 *
 * The twin path uses this so genomic coordinates go through the
 * same kernel the tests already prove, not a second implementation.
 */

#include "avian_array.h"
#include "avian_geom.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Owns no memory. Forwards to avian_geom_transform_f32. Returns its status. */
avian_status avian_geom_twin_coordinates(
    avian_f32_output output,
    avian_f32_input  input,
    avian_f32_input  matrix,
    avian_f32_input  translation,
    size_t n
);

#ifdef __cplusplus
}
#endif

#endif /* AVIAN_GEOM_BRIDGE_H */
