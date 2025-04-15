
#include "quaternion.h"
#include "matrix.h"
#include "vector.h"
Quaternion::Quaternion(const Matrix3x4_t* matrix) {
    float m00 = matrix->arrData[0][0];
    float m11 = matrix->arrData[1][1];
    float m22 = matrix->arrData[2][2];
    float sum = m00 + m11 + m22;
    if (sum >= 0.f) {
        float t = sqrtf(sum + 1.f);
        w = 0.5f * t;
        t = 0.5f / t;
        x = (matrix->arrData[2][1] - matrix->arrData[1][2]) * t;
        y = (matrix->arrData[0][2] - matrix->arrData[2][0]) * t;
        z = (matrix->arrData[1][0] - matrix->arrData[0][1]) * t;
    }
    else if (m00 > m11 && m00 > m22) {
        float t = sqrtf(1.f + m00 - m11 - m22);
        x = 0.5f * t;
        t = 0.5f / t;
        y = (matrix->arrData[0][1] + matrix->arrData[1][0]) * t;
        z = (matrix->arrData[2][0] + matrix->arrData[0][2]) * t;
        w = (matrix->arrData[2][1] - matrix->arrData[1][2]) * t;
    }
    else if (m11 > m22) {
        float t = sqrtf(1.f + m11 - m00 - m22);
        y = 0.5f * t;
        t = 0.5f / t;
        x = (matrix->arrData[0][1] + matrix->arrData[1][0]) * t;
        z = (matrix->arrData[1][2] + matrix->arrData[2][1]) * t;
        w = (matrix->arrData[0][2] - matrix->arrData[2][0]) * t;
    }
    else {
        float t = sqrtf(1.f + m22 - m00 - m11);
        z = 0.5f * t;
        t = 0.5f / t;
        x = (matrix->arrData[2][0] + matrix->arrData[0][2]) * t;
        y = (matrix->arrData[1][2] + matrix->arrData[2][1]) * t;
        w = (matrix->arrData[1][0] - matrix->arrData[0][1]) * t;
    }
}

Matrix3x4_t Quaternion::ToMatrix(const Vector_t& v) const {
    const Quaternion& q = *this;

    Matrix3x4_t matrix;
    matrix[0][0] = 1.0 - 2.0 * q[1] * q[1] - 2.0 * q[2] * q[2];
    matrix[1][0] = 2.0 * q[0] * q[1] + 2.0 * q[3] * q[2];
    matrix[2][0] = 2.0 * q[0] * q[2] - 2.0 * q[3] * q[1];

    matrix[0][1] = 2.0 * q[0] * q[1] - 2.0 * q[3] * q[2];
    matrix[1][1] = 1.0 - 2.0 * q[0] * q[0] - 2.0 * q[2] * q[2];
    matrix[2][1] = 2.0 * q[1] * q[2] + 2.0 * q[3] * q[0];

    matrix[0][2] = 2.0 * q[0] * q[2] + 2.0 * q[3] * q[1];
    matrix[1][2] = 2.0 * q[1] * q[2] - 2.0 * q[3] * q[0];
    matrix[2][2] = 1.0 - 2.0 * q[0] * q[0] - 2.0 * q[1] * q[1];

    matrix.SetAxis(3, v);

    return matrix;
}