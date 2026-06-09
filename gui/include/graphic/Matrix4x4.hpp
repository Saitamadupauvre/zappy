#pragma once
#include "graphic/Vectors.hpp"

namespace graphic {

/**
 * @struct Matrix4x4
 * @brief Represents a 4x4 matrix with floating-point coordinates
 */
struct Matrix4x4 {
    float m[16];

    static Matrix4x4 Identity() {
        return {{ 1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1 }};
    }

    static Matrix4x4 Translation(float x, float y, float z) {
        Matrix4x4 mat = Identity();
        mat.m[12] = x; mat.m[13] = y; mat.m[14] = z;
        return mat;
    }

    static Matrix4x4 Scale(float x, float y, float z) {
        Matrix4x4 mat = Identity();
        mat.m[0] = x; mat.m[5] = y; mat.m[10] = z;
        return mat;
    }

    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                res.m[i*4 + j] = 0;
                for (int k = 0; k < 4; k++)
                    res.m[i*4 + j] += m[i*4 + k] * other.m[k*4 + j];
            }
        }
        return res;
    }

    Vector3f getTranslation() const {
        return { m[12], m[13], m[14] };
    }

    Vector3f getScale() const {
        return {
            std::sqrt(m[0]*m[0] + m[1]*m[1] + m[2]*m[2]),
            std::sqrt(m[4]*m[4] + m[5]*m[5] + m[6]*m[6]),
            std::sqrt(m[8]*m[8] + m[9]*m[9] + m[10]*m[10])
        };
    }

    static Matrix4x4 RotationY(float angleRad) {
        float s = std::sinf(angleRad);
        float c = std::cosf(angleRad);
        Matrix4x4 mat = Identity();
        mat.m[0] = c;  mat.m[2] = s;
        mat.m[8] = -s; mat.m[10] = c;
        return mat;
    }

};

} // namespace graphic