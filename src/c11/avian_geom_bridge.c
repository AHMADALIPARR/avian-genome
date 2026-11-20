#include "avian_geom_bridge.h"

avian_status avian_geom_twin_coordinates(
    avian_f32_output output,
    avian_f32_input  input,
    avian_f32_input  matrix,
    avian_f32_input  translation,
    size_t n
) {
    return avian_geom_transform_f32(output, input, matrix, translation, n);
}
