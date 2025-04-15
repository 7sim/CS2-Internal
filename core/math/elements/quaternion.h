#pragma once
// used: [crt] isfinite, fmodf, remainderf
#include <cmath>
#include <algorithm>
#include "vector.h"
// Undefine conflicting macros
#undef min
#undef max
// forward declarations
struct Matrix3x4_t;
struct Vector_t;


class Quaternion {
public:
	Quaternion() = default;
	explicit Quaternion(const Matrix3x4_t* matrix);
	Matrix3x4_t ToMatrix(const Vector_t& v) const;

	float operator[](int i) const { return (&x)[i]; }
	float& operator[](int i) { return (&x)[i]; }

	float x, y, z, w;
};

