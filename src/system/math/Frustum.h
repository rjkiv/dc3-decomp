#pragma once
#include "math/Plane.h"

class Frustum {
    // total size: 0x60
public:
    void Set(float, float, float, float);

    class Plane front; // offset 0x0, size 0x10
    class Plane back; // offset 0x10, size 0x10
    class Plane left; // offset 0x20, size 0x10
    class Plane right; // offset 0x30, size 0x10
    class Plane top; // offset 0x40, size 0x10
    class Plane bottom; // offset 0x50, size 0x10
};

#include "Frustum.inl"
