#pragma once
#include "axpch.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Vec2 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Vec2 {
		float x, y;

		Vec2() : x(0.0f), y(0.0f) {}
		Vec2(float x, float y) : x(x), y(y) {}

		static Vec2 zero() { return Vec2(0.0f, 0.0f); }
		static Vec2 one() { return Vec2(1.0f, 1.0f); }
		
		// Converting to and from XMVECTOR
		DirectX::XMVECTOR toXM() const {
			return DirectX::XMVectorSet(x, y, 0.0f, 0.0f);
		}

		static Vec2 fromXM(DirectX::XMVECTOR vector) {
			DirectX::XMFLOAT2 result;
			DirectX::XMStoreFloat2(&result, vector);
			return Vec2(result.x, result.y);
		}

		// Operator overloads
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

		// Math operations
		float dot(const Vec2& other) const {
			return x * other.x + y * other.y;
		}

		float length() const {
			return std::sqrt(x * x + y * y);
		}

		Vec2 normalized() const {
			float len = length();
			return (len != 0) ? (*this / len) : Vec2::zero();
		}

		float* data() { return &x; }
		const float* data() const { return &x; }

	};

	////////////////////////////////////////////////////////////////////////////////
	///// Vec3 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Vec3 {
		float x, y, z;

		Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

		static Vec3 zero() { return Vec3(0.0f, 0.0f, 0.0f); }
		static Vec3 one() { return Vec3(1.0f, 1.0f, 1.0f); }

		// Converting to and from XMVECTOR
		DirectX::XMVECTOR toXM() const {
			return DirectX::XMVectorSet(x, y, z, 0.0f);
		}
		
		static Vec3 fromXM(DirectX::XMVECTOR vector) {
			DirectX::XMFLOAT3 result;
			DirectX::XMStoreFloat3(&result, vector);
			return Vec3(result.x, result.y, result.z);
		}

		// Operator overloads
		Vec3 operator+(const Vec3& other) const { return fromXM(DirectX::XMVectorAdd(toXM(), other.toXM())); }
		Vec3 operator-(const Vec3& other) const { return fromXM(DirectX::XMVectorSubtract(toXM(), other.toXM())); }
		Vec3 operator*(float scalar) const { return fromXM(DirectX::XMVectorScale(toXM(), scalar)); }
		Vec3 operator/(float scalar) const { return fromXM(DirectX::XMVectorScale(toXM(), 1.0f / scalar));}
		Vec3& operator+=(const Vec3& other) { *this = *this + other; return *this; }
		Vec3& operator-=(const Vec3& other) { *this = *this - other; return *this; }
		Vec3& operator*=(float scalar) { *this = *this * scalar; return *this; }
		Vec3& operator/=(float scalar) { *this = *this / scalar; return *this; }
		bool operator==(const Vec3& other) { return x == other.x && y == other.y && z == other.z; }
		bool operator!=(const Vec3& other) { return x != other.x || y != other.y || z != other.z; }

		// Math operations
		float dot(const Vec3& other) const {
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(toXM(), other.toXM()));
		}

		Vec3 cross(const Vec3& other) const {
			return fromXM(DirectX::XMVector3Cross(toXM(), other.toXM()));
		}

		float length() const {
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(toXM()));
		}

		Vec3 normalized() const {
			return fromXM(DirectX::XMVector3Normalize(toXM()));
		}

		float distance(const Vec3& other) const {
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(toXM(), other.toXM())));
		}

		float* data() { return &x; }
		const float* data() const { return &x; }

	};

	////////////////////////////////////////////////////////////////////////////////
	///// Vec4 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Vec4 {
		
		float x, y, z, w;

		Vec4() : x(0), y(0), z(0), w(0) {}
		Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Vec4(float xyzw[4]) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3]) {}

		static Vec4 zero() { return Vec4(0.0f, 0.0f, 0.0f, 0.0f); }
		static Vec4 one() { return Vec4(1.0f, 1.0f, 1.0f, 1.0f); }

		// Converting to and from XMVECTOR
		DirectX::XMVECTOR toXM() const {
			return DirectX::XMVectorSet(x, y, z, w);
		}

		static Vec4 fromXM(DirectX::XMVECTOR vector) {
			DirectX::XMFLOAT4 result;
			DirectX::XMStoreFloat4(&result, vector);
			return Vec4(result.x, result.y, result.z, result.w);
		}

		// Operator overloads
		Vec4 operator+(const Vec4& other) const {return fromXM(DirectX::XMVectorAdd(toXM(), other.toXM())); }
		Vec4 operator-(const Vec4& other) const { return fromXM(DirectX::XMVectorSubtract(toXM(), other.toXM())); }
		Vec4 operator*(float scalar) const {return fromXM(DirectX::XMVectorScale(toXM(), scalar)); }
		Vec4 operator/(float scalar) const { return fromXM(DirectX::XMVectorScale(toXM(), 1.0f / scalar)); }
		Vec4& operator+=(const Vec4& other) { *this = *this + other; return *this; }
		Vec4& operator-=(const Vec4& other) { *this = *this - other; return *this; }
		Vec4& operator*=(float scalar) { *this = *this * scalar; return *this; }
		Vec4& operator/=(float scalar) { *this = *this / scalar; return *this; }
		bool operator==(const Vec4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
		bool operator!=(const Vec4& other) const { return x != other.x || y != other.y || z != other.z || w != other.w; }

		// Math operations
		float dot(const Vec4& other) const {
			return DirectX::XMVectorGetX(DirectX::XMVector4Dot(toXM(), other.toXM()));
		}

		float length() const {
			return DirectX::XMVectorGetX(DirectX::XMVector4Length(toXM()));
		}

		Vec4 normalized() const {
			return fromXM(DirectX::XMVector4Normalize(toXM()));
		}

		DirectX::XMFLOAT4 toFloat4() const {
			return DirectX::XMFLOAT4(x, y, z, w);
		}

		float* data() { return &x; }
		const float* data() const { return &x; }

	};

	////////////////////////////////////////////////////////////////////////////////
	///// Mat4 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Mat4 {

		DirectX::XMMATRIX matrix;

		Mat4() : matrix(DirectX::XMMatrixIdentity()) {}

		explicit Mat4(const DirectX::XMMATRIX& m) : matrix(m) {}

		DirectX::XMMATRIX toXM() const {
			return matrix; 
		}

		static Mat4 fromXM(const DirectX::XMMATRIX& m) {
			return Mat4(m);
		}

		static Mat4 identity() {
			return Mat4(DirectX::XMMatrixIdentity());
		}

		static Mat4 translation(const Vec3& pos) {
			return Mat4(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
		}

		static Mat4 rotationX(float radians) {
			return Mat4(DirectX::XMMatrixRotationX(radians));
		}

		static Mat4 rotationY(float radians) {
			return Mat4(DirectX::XMMatrixRotationY(radians));
		}

		static Mat4 rotationZ(float radians) {
			return Mat4(DirectX::XMMatrixRotationZ(radians));
		}

		static Mat4 scale(const Vec3& scale) {
			return Mat4(DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
		}

		static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
			return Mat4(DirectX::XMMatrixLookAtLH(eye.toXM(), target.toXM(), up.toXM()));
		}

		static Mat4 orthographic(float width, float height, float nearZ, float farZ) {
			return Mat4(DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ));
		}

		static Mat4 orthographic(float left, float right, float bottom, float top, float nearZ, float farZ) {
			return Mat4(DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearZ, farZ));
		}

		static Mat4 perspective(float fovY, float aspect, float nearZ, float farZ) {
			return Mat4(DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
		}

		Mat4 transposed() const {
			return Mat4(DirectX::XMMatrixTranspose(matrix));
		}

		Mat4 inverse() const {
			return Mat4(DirectX::XMMatrixInverse(nullptr, matrix));
		}

		float determinant() const {
			return DirectX::XMVectorGetX(DirectX::XMMatrixDeterminant(matrix));
		}

		Vec4 transform(const Vec4& vector) const {
			return Vec4::fromXM(DirectX::XMVector4Transform(vector.toXM(), matrix));
		}

		DirectX::XMFLOAT4X4 toFloat4x4() const {
			DirectX::XMFLOAT4X4 result;
			DirectX::XMStoreFloat4x4(&result, matrix);
			return result;
		}

		static Mat4 TRS(const Vec3& translation, const Vec3& rotationEuler, const Vec3& scale) {
			Mat4 t = Mat4::translation(translation);
			Mat4 r = Mat4::rotationZ(rotationEuler.z) *
					 Mat4::rotationY(rotationEuler.y) *
					 Mat4::rotationX(rotationEuler.x);
			Mat4 s = Mat4::scale(scale);
			return t * r * s;
		}

		Mat4 operator*(const Mat4& other) const { return Mat4(DirectX::XMMatrixMultiply(matrix, other.matrix)); }
		Mat4& operator*=(const Mat4& other) { matrix = DirectX::XMMatrixMultiply(matrix, other.matrix); return *this; }

	};

}
