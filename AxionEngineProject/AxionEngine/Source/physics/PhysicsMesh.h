#pragma once

#include "AxionEngine/Source/core/Ref.h"

namespace Axion {

	class PhysicsMesh : public RefCounted {
	public:

		enum class Type {
			Convex = 0,
			Triangle = 1
		};

		PhysicsMesh(Type type, void* runtimeMesh);
		~PhysicsMesh();

		void release();

		Type getType() const { return m_type; }
		void* getRuntimeMesh() const { return m_runtimeMesh; }

	private:

		Type m_type = Type::Convex;
		void* m_runtimeMesh = nullptr;

	};

}
