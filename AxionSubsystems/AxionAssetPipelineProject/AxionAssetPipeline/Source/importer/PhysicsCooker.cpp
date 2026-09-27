#include "PhysicsCooker.h"
#include "AxionEngine/Source/core/Logging.h"

#include <fstream>

#include <physx/include/PxPhysicsAPI.h>
#include <physx/include/cooking/PxCooking.h>

namespace Axion::AAP {

	using namespace physx;

	bool PhysicsCooker::cookConvex(const Ref<Mesh>& visualMesh, uint32_t vertexLimit, const std::filesystem::path& outputPath) {
		if (!visualMesh || visualMesh->getVertices().empty()) {
			AX_CORE_LOG_ERROR("PhysicsCooker: Invalid visual mesh provided for Convex cooking.");
			return false;
		}

		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = static_cast<PxU32>(visualMesh->getVertices().size());
		convexDesc.points.stride = sizeof(Axion::Vertex);
		convexDesc.points.data = visualMesh->getVertices().data();
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.vertexLimit = vertexLimit;

		PxDefaultMemoryOutputStream writeBuffer;
		PxConvexMeshCookingResult::Enum result;
		PxCookingParams params(PxTolerancesScale{});

		if (!PxCookConvexMesh(params, convexDesc, writeBuffer, &result)) {
			AX_CORE_LOG_ERROR("PhysicsCooker: Failed to cook Convex Mesh! Result code: {}", (int)result);
			return false;
		}

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out.is_open()) return false;

		out.write(reinterpret_cast<const char*>(writeBuffer.getData()), writeBuffer.getSize());
		out.close();

		AX_CORE_LOG_INFO("Successfully cooked Convex Mesh to: {}", outputPath.string());
		return true;
	}

	bool PhysicsCooker::cookTriangle(const Ref<Mesh>& visualMesh, const std::filesystem::path& outputPath) {
		if (!visualMesh || visualMesh->getVertices().empty() || visualMesh->getIndices().empty()) {
			AX_CORE_LOG_ERROR("PhysicsCooker: Invalid visual mesh provided for Triangle cooking.");
			return false;
		}

		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = static_cast<PxU32>(visualMesh->getVertices().size());
		meshDesc.points.stride = sizeof(Axion::Vertex);
		meshDesc.points.data = visualMesh->getVertices().data();
		meshDesc.triangles.count = static_cast<PxU32>(visualMesh->getIndices().size() / 3);
		meshDesc.triangles.stride = 3 * sizeof(uint32_t);
		meshDesc.triangles.data = visualMesh->getIndices().data();

		PxDefaultMemoryOutputStream writeBuffer;
		PxTriangleMeshCookingResult::Enum result;
		PxCookingParams params(PxTolerancesScale{});

		if (!PxCookTriangleMesh(params, meshDesc, writeBuffer, &result)) {
			AX_CORE_LOG_ERROR("PhysicsCooker: Failed to cook Triangle Mesh! Result code: {}", (int)result);
			return false;
		}

		std::ofstream out(outputPath, std::ios::out | std::ios::binary);
		if (!out.is_open()) return false;

		out.write(reinterpret_cast<const char*>(writeBuffer.getData()), writeBuffer.getSize());
		out.close();

		AX_CORE_LOG_INFO("Successfully cooked Triangle Mesh to: {}", outputPath.string());
		return true;
	}

}
