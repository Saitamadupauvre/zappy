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

    // Builds a rotation that aligns local +Y with targetUp, then rotates yawRad around that up axis.
    static Matrix4x4 RotationAlignUp(const Vector3f& targetUp, float yawRad = 0.0f) {
        Vector3f up  = targetUp.normalized();
        Vector3f ref = (std::abs(up.y) < 0.9f) ? Vector3f{0.f, 1.f, 0.f} : Vector3f{1.f, 0.f, 0.f};
        Vector3f right   = ref.cross(up).normalized();
        Vector3f fwd     = up.cross(right);
        if (yawRad != 0.0f) {
            float c = std::cos(yawRad), s = std::sin(yawRad);
            Vector3f r2 = {right.x*c + fwd.x*s, right.y*c + fwd.y*s, right.z*c + fwd.z*s};
            Vector3f f2 = {-right.x*s + fwd.x*c, -right.y*s + fwd.y*c, -right.z*s + fwd.z*c};
            right = r2; fwd = f2;
        }
        // Row-major, row-vector: row 0/1/2 = local X/Y/Z axes in world space.
        Matrix4x4 mat = Identity();
        mat.m[0] = right.x; mat.m[1] = right.y; mat.m[2] = right.z;
        mat.m[4] = up.x;    mat.m[5] = up.y;    mat.m[6] = up.z;
        mat.m[8] = fwd.x;   mat.m[9] = fwd.y;   mat.m[10] = fwd.z;
        return mat;
    }

    // Aligns local +Y with `upDir` and local +Z with the component of `fwdHint`
    // lying in the tangent plane, then rotates yawRad around the up axis.
    // Unlike RotationAlignUp this takes an explicit forward, so the facing stays
    // continuous as the surface normal varies (e.g. across a torus) instead of
    // snapping when a hard-coded reference axis flips.
    static Matrix4x4 OrientationFromUpForward(const Vector3f& upDir,
                                              const Vector3f& fwdHint,
                                              float yawRad = 0.0f) {
        Vector3f up  = upDir.normalized();
        Vector3f fwd = fwdHint - up * up.dot(fwdHint);   // project onto tangent plane
        if (fwd.lengthSquared() < 1e-6f) {               // hint parallel to up → pick any tangent
            Vector3f ref = (std::abs(up.y) < 0.9f) ? Vector3f{0.f, 1.f, 0.f} : Vector3f{1.f, 0.f, 0.f};
            fwd = ref - up * up.dot(ref);
        }
        fwd = fwd.normalized();
        Vector3f right = up.cross(fwd).normalized();
        fwd = right.cross(up);                           // re-orthonormalize
        if (yawRad != 0.0f) {
            float c = std::cos(yawRad), s = std::sin(yawRad);
            Vector3f r2 = right * c + fwd * s;
            Vector3f f2 = right * (-s) + fwd * c;
            right = r2; fwd = f2;
        }
        Matrix4x4 mat = Identity();
        mat.m[0] = right.x; mat.m[1] = right.y; mat.m[2] = right.z;
        mat.m[4] = up.x;    mat.m[5] = up.y;    mat.m[6] = up.z;
        mat.m[8] = fwd.x;   mat.m[9] = fwd.y;   mat.m[10] = fwd.z;
        return mat;
    }

    static Matrix4x4 RotationY(float angleRad) {
        float s = std::sin(angleRad);
        float c = std::cos(angleRad);
        Matrix4x4 mat = Identity();
        mat.m[0] = c;  mat.m[2] = s;
        mat.m[8] = -s; mat.m[10] = c;
        return mat;
    }

    static Matrix4x4 RotationX(float angleRad) {
        float s = std::sin(angleRad);
        float c = std::cos(angleRad);
        Matrix4x4 mat = Identity();
        mat.m[5] = c;  mat.m[6] = s;
        mat.m[9] = -s; mat.m[10] = c;
        return mat;
    }

    static Matrix4x4 RotationZ(float angleRad) {
        float s = std::sin(angleRad);
        float c = std::cos(angleRad);
        Matrix4x4 mat = Identity();
        mat.m[0] = c;  mat.m[1] = s;
        mat.m[4] = -s; mat.m[5] = c;
        return mat;
    }

    // Local rotation offset from Euler angles (radians), applied X then Y then Z.
    static Matrix4x4 RotationEuler(float xRad, float yRad, float zRad) {
        return RotationX(xRad) * RotationY(yRad) * RotationZ(zRad);
    }

    Vector3f transform(const Vector3f& vec) const {
    float x = vec.x * m[0] + vec.y * m[4] + vec.z * m[8]  + m[12];
    float y = vec.x * m[1] + vec.y * m[5] + vec.z * m[9]  + m[13];
    float z = vec.x * m[2] + vec.y * m[6] + vec.z * m[10] + m[14];
    
    return { x, y, z };
    }

};

} // namespace graphic