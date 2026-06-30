#pragma once

#include <cstdint>

namespace Silica {

	struct Vec2 {
		float x = 0.0f;
		float y = 0.0f;

		constexpr Vec2() = default;
		constexpr Vec2(float x, float y) : x(x), y(y) {}
		constexpr Vec2(int x, int y) : x((float)x), y((float)y) {}

		static constexpr Vec2 zero() { return Vec2(0.0f, 0.0f); }
		static constexpr Vec2 one() { return Vec2(1.0f, 1.0f); }

		constexpr Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
		constexpr Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
		constexpr Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
		constexpr Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
		constexpr Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
		constexpr Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
		constexpr Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
		constexpr Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
		constexpr bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
		constexpr bool operator!=(const Vec2& other) const { return x != other.x || y != other.y; }
	};



	struct Vec3 {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		constexpr Vec3() = default;
		constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
		constexpr Vec3(int x, int y, int z) : x((float)x), y((float)y), z((float)z) {}

		static constexpr Vec3 zero() { return Vec3(0.0f, 0.0f, 0.0f); }
		static constexpr Vec3 one() { return Vec3(1.0f, 1.0f, 1.0f); }

		constexpr Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
		constexpr Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
		constexpr Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
		constexpr Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }
		constexpr Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
		constexpr Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
		constexpr Vec3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
		constexpr Vec3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
		constexpr bool operator==(const Vec3& other) const { return x == other.x && y == other.y && z == other.z; }
		constexpr bool operator!=(const Vec3& other) const { return x != other.x || y != other.y || z != other.z; }
	};



	struct Rect {
		float left = 0.0f;
		float right = 0.0f;
		float top = 0.0f;
		float bottom = 0.0f;

		constexpr Rect() = default;
		constexpr Rect(float left, float right, float top, float bottom) : left(left), right(right), top(top), bottom(bottom) {}

		constexpr inline float getWidth() const { return right - left; }
		constexpr inline float getHeight() const { return bottom - top; }

		constexpr inline bool contains(const Vec2& point) const {
			return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
		}

		constexpr inline Rect intersect(const Rect& other) const {
			return Rect(
				(left > other.left) ? left : other.left,
				(right < other.right) ? right : other.right,
				(top > other.top) ? top : other.top,
				(bottom < other.bottom) ? bottom : other.bottom
			);
		}

		constexpr inline bool operator==(const Rect& other) const {
			return left == other.left && right == other.right && top == other.top && bottom == other.bottom;
		}

		constexpr inline bool operator!=(const Rect& other) const {
			return !(*this == other);
		}

	};



	struct EventReply {
		bool isHandled = false;

		static EventReply handled() { return { true }; }
		static EventReply unhandled() { return { false }; }
	};



	struct Color {
		uint32_t value;

		constexpr Color() : value(0x00000000) {}
		constexpr Color(uint32_t hex) : value(hex) {}
		constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : value((a << 24) | (b << 16) | (g << 8) | r) {}

		constexpr operator uint32_t() const { return value; }

		constexpr uint8_t r() const { return value & 0xFF; }
		constexpr uint8_t g() const { return (value >> 8) & 0xFF; }
		constexpr uint8_t b() const { return (value >> 16) & 0xFF; }
		constexpr uint8_t a() const { return (value >> 24) & 0xFF; }

		static constexpr Color white() { return Color(255, 255, 255); }
		static constexpr Color black() { return Color(0, 0, 0); }
		static constexpr Color red() { return Color(255, 0, 0); }
		static constexpr Color green() { return Color(0, 255, 0); }
		static constexpr Color blue() { return Color(0, 0, 255); }
		static constexpr Color transparent() { return Color(0, 0, 0, 0); }

	};

}
