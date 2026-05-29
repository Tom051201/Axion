#pragma once

#include <cstdint>

namespace Silica {

	struct Vec2 {
		float x = 0.0f;
		float y = 0.0f;

		Vec2() = default;
		Vec2(float x, float y) : x(x), y(y) {}
		Vec2(int x, int y) : x((float)x), y((float)y) {}

		static Vec2 zero() { return Vec2(0.0f, 0.0f); }
		static Vec2 one() { return Vec2(1.0f, 1.0f); }

		Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
		Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
		Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
		Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
		Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
		Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
		Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
		Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
		bool operator==(const Vec2& other) const { return x == other.x && y == other.y; }
		bool operator!=(const Vec2& other) const { return x != other.x || y != other.y; }
	};



	struct Rect {
		float left = 0.0f;
		float right = 0.0f;
		float top = 0.0f;
		float bottom = 0.0f;

		Rect() = default;
		Rect(float left, float right, float top, float bottom) : left(left), right(right), top(top), bottom(bottom) {}

		inline float getWidth() const { return right - left; }
		inline float getHeight() const { return bottom - top; }

		inline bool contains(const Vec2& point) const {
			return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
		}

		inline Rect intersect(const Rect& other) const {
			return Rect(
				(left > other.left) ? left : other.left,
				(right < other.right) ? right : other.right,
				(top > other.top) ? top : other.top,
				(bottom < other.bottom) ? bottom : other.bottom
			);
		}

	};



	struct EventReply {
		bool isHandled = false;

		static EventReply handled() { return { true }; }
		static EventReply unhandled() { return { false }; }
	};



	struct Color {
		uint32_t value;

		Color() : value(0x00000000) {}
		Color(uint32_t hex) : value(hex) {}
		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
			value = (a << 24) | (b << 16) | (g << 8) | r;
		}

		operator uint32_t() const { return value; }

		uint8_t r() const { return value & 0xFF; }
		uint8_t g() const { return (value >> 8) & 0xFF; }
		uint8_t b() const { return (value >> 16) & 0xFF; }
		uint8_t a() const { return (value >> 24) & 0xFF; }

		static Color white() { return Color(255, 255, 255); }
		static Color black() { return Color(0, 0, 0); }
		static Color red() { return Color(255, 0, 0); }
		static Color green() { return Color(0, 255, 0); }
		static Color blue() { return Color(0, 0, 255); }
		static Color transparent() { return Color(0, 0, 0, 0); }

	};

}
