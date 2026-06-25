#pragma once

#include <cmath>

namespace graphic {

/**
 * @struct Vector3f
 * @brief Represents a 3D vector with floating-point coordinates.
 */
struct Vector3f {
    float x; ///< X coordinate.
    float y; ///< Y coordinate.
    float z; ///< Z coordinate.

    Vector3f operator-(const Vector3f& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vector3f operator+(const Vector3f& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vector3f operator*(float scalar) const { return {x * scalar, y * scalar, z * scalar}; }

    float lengthSquared() const { return x * x + y * y + z * z; }
    float length() const        { return std::sqrt(lengthSquared()); }

    Vector3f normalized() const {
        float len = length();
        return (len > 0.0f) ? (*this * (1.0f / len)) : zero();
    }

    float distanceTo(const Vector3f& other) const {
        return (*this - other).length();
    }

    float dot(const Vector3f& b) const { return x*b.x + y*b.y + z*b.z; }

    Vector3f cross(const Vector3f& b) const {
        return {y*b.z - z*b.y, z*b.x - x*b.z, x*b.y - y*b.x};
    }

    static constexpr Vector3f zero()    { return {0.0f, 0.0f, 0.0f}; }
    static constexpr Vector3f one()     { return {1.0f, 1.0f, 1.0f}; }
    static constexpr Vector3f up()      { return {0.0f, 1.0f, 0.0f}; }
    static constexpr Vector3f forward() { return {0.0f, 0.0f, 1.0f}; }
    static constexpr Vector3f right()   { return {1.0f, 0.0f, 0.0f}; }
};

/**
 * @struct Vector2f
 * @brief Represents a 2D vector with floating-point coordinates.
 */
struct Vector2f {
    float x; ///< X coordinate.
    float y; ///< Y coordinate.

    Vector2f operator+(const Vector2f& other) const { return {x + other.x, y + other.y}; }
    Vector2f operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vector2f operator/(float scalar) const { return {x / scalar, y / scalar}; }

};

} // namespace graphic