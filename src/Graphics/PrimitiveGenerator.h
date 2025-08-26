#pragma once

#include "Liara_Model.h"

/**
 * @brief Generate primitive mesh data
 */
namespace Liara::Graphics::PrimitiveGenerator
{
    [[nodiscard]] MeshData GenerateQuad();
    [[nodiscard]] MeshData GenerateCube();
    [[nodiscard]] MeshData GenerateSphere(uint32_t segments = 32);
    [[nodiscard]] MeshData GeneratePlane(float size = 1.0f);
    [[nodiscard]] MeshData GenerateCylinder(float height = 1.0f, uint32_t segments = 32);
}