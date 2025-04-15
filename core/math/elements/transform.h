#pragma once

// used: matResult
#include "matrix.h"
// used: quaternion
#include "quaternion.h"

class CTransform
{
public:
	VectorAligned_t vecPosition;
	alignas(16) Quaternion quatOrientation;
public:
	Matrix3x4_t ToMatrix() const;
};

static_assert(alignof(CTransform) == 16);
