#pragma once
#include <cstdint>
#include <iostream>

namespace ECS
{
    using Entity = uint32_t;
    constexpr Entity NULL_ENTITY = 0xFFFFFFFF;
} // namespace ECS
