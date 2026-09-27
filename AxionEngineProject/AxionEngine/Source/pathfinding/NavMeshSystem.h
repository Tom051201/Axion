#pragma once

#include <vector>
#include <filesystem>

#include "AxionEngine/Source/core/Math.h"
#include "AxionEngine/Source/pathfinding/NavMesh.h"

class dtNavMesh;
class dtNavMeshQuery;
class rcContext;

namespace Axion {

	class Scene;

	struct NavMeshBuildSettings {
		float agentHeight = 2.0f;
		float agentRadius = 0.5f;
		float agentMaxClimb = 0.3f;
		float agentMaxSlope = 45.0f;
		float cellSize = 0.3f;
		float cellHeight = 0.2f;
	};

	struct NavMeshEdge {
		Vec3 p0, p1;
	};

	class NavMeshSystem {
	public:

		static void initialize();
		static void shutdown();

		static Ref<NavMesh> bakeScene(Scene* scene, const NavMeshBuildSettings& settings = NavMeshBuildSettings());

		static std::vector<Vec3> calculatePath(Ref<NavMesh> navMesh, const Vec3& start, const Vec3& end);
		static std::vector<NavMeshEdge> getDebugEdges(Ref<NavMesh> navMesh);

		static void serializeToStream(Ref<NavMesh> navMesh, std::ostream& out);
		static Ref<NavMesh> deserializeFromStream(std::istream& in);

		static Vec3 getNearestPoint(Ref<NavMesh> navMesh, const Vec3& point);
		static Vec3 getRandomPointAroundCircle(Ref<NavMesh> navMesh, const Vec3& center, float radius);
		static bool raycast(Ref<NavMesh> navMesh, const Vec3& start, const Vec3& end, Vec3& outHitPoint);

	private:

		static rcContext* s_ctx;

	};

}
