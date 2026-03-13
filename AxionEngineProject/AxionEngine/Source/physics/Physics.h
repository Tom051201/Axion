#pragma once

#include "AxionEngine/Source/scene/Entity.h"

namespace Axion {

	enum class ForceMode {
		Force = 0,			// Mass included
		Impulse,			// Mass included
		VelocityChange,		// Mass ignored
		Acceleration		// Mass ignored
	};

	class Physics {
	public:

		static void addForce(Entity entity, const Vec3& force, ForceMode mode = ForceMode::Force);
		static void addTorque(Entity entity, const Vec3& torque, ForceMode mode = ForceMode::Force);
		static void addRadialImpulse(Entity entity, const Vec3& origin, float radius, float strength);

		static void setPosition(Entity entity, const Vec3& position);
		static void setRotation(Entity entity, const Quat& rotation);

		static void setLinearVelocity(Entity entity, const Vec3& velocity);
		static Vec3 getLinearVelocity(Entity entity);

		static void setAngularVelocity(Entity entity, const Vec3& velocity);
		static Vec3 getAngularVelocity(Entity entity);

		static void setMass(Entity entity, float mass);
		static float getMass(Entity entity);

	};

}
