#pragma once
#include "axpch.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// Vec3 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Vec3 {
		float x, y, z;

		Vec3() : x(0), y(0), z(0) {}
		Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

		static Vec3 zero() { return Vec3(0, 0, 0); }
		static Vec3 one() { return Vec3(1, 1, 1); }

		DirectX::XMVECTOR toXM() const { return DirectX::XMLoadFloat3(reinterpret_cast<const DirectX::XMFLOAT3*>(this)); }
		
		static Vec3 fromXM(DirectX::XMVECTOR v) {
			Vec3 result;
			DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&result), v);
			return result;
		}

		Vec3 operator+(const Vec3& rhs) const {
			return fromXM(DirectX::XMVectorAdd(toXM(), rhs.toXM()));
		}

		Vec3 operator-(const Vec3& rhs) const {
			return fromXM(DirectX::XMVectorSubtract(toXM(), rhs.toXM()));
		}

		Vec3 operator*(float scalar) const {
			return fromXM(DirectX::XMVectorScale(toXM(), scalar));
		}

		float dot(const Vec3& rhs) const {
			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(toXM(), rhs.toXM()));
		}

		Vec3 cross(const Vec3& rhs) const {
			return fromXM(DirectX::XMVector3Cross(toXM(), rhs.toXM()));
		}

		float length() const {
			return DirectX::XMVectorGetX(DirectX::XMVector3Length(toXM()));
		}

		Vec3 normalize() const {
			return fromXM(DirectX::XMVector3Normalize(toXM()));
		}

	};

	////////////////////////////////////////////////////////////////////////////////
	///// Vec4 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Vec4 {
		
		float x, y, z, w;

		Vec4() : x(0), y(0), z(0), w(0) {}
		Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

		DirectX::XMVECTOR toXM() const { return DirectX::XMLoadFloat4(reinterpret_cast<const DirectX::XMFLOAT4*>(this)); }

		static Vec4 fromXM(DirectX::XMVECTOR v) {
			Vec4 result;
			DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&result), v);
			return result;
		}

	};

	////////////////////////////////////////////////////////////////////////////////
	///// Mat4 /////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	struct Mat4 {

		float m[4][4];

		Mat4() {
			memset(m, 0, sizeof(m));
		}

		DirectX::XMMATRIX toXM() const {
			return DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(m));
		}

		static Mat4 fromXM(const DirectX::XMMATRIX& xm) {
			Mat4 result;
			DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&result.m), xm);
			return result;
		}

		static Mat4 identity() {
			return fromXM(DirectX::XMMatrixIdentity());
		}

		static Mat4 translation(const Vec3& pos) {
			return fromXM(DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));
		}

		static Mat4 rotationX(float radians) {
			return fromXM(DirectX::XMMatrixRotationX(radians));
		}

		static Mat4 rotationY(float radians) {
			return fromXM(DirectX::XMMatrixRotationY(radians));
		}

		static Mat4 rotationZ(float radians) {
			return fromXM(DirectX::XMMatrixRotationZ(radians));
		}

		static Mat4 scale(const Vec3& scale) {
			return fromXM(DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
		}

		static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
			return fromXM(DirectX::XMMatrixLookAtLH(eye.toXM(), target.toXM(), up.toXM()));
		}

		static Mat4 orthographic(float width, float height, float nearZ, float farZ) {
			return fromXM(DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ));
		}

		static Mat4 orthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ) {
			return Mat4::fromXM(DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, nearZ, farZ));
		}

		static Mat4 perspective(float fovY, float aspect, float nearZ, float farZ) {
			return fromXM(DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
		}

		Mat4 operator*(const Mat4& rhs) const {
			return fromXM(toXM() * rhs.toXM());
		}

	};

}
