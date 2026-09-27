#include "axpch.h"
#include "NavMeshSystem.h"

#include <cstring>

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>
#include <DetourNavMeshBuilder.h>

#include "AxionEngine/Source/core/AssetManager.h"
#include "AxionEngine/Source/core/BinaryHeaders.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"
#include "AxionEngine/Source/scene/Scene.h"

namespace Axion {

	rcContext* NavMeshSystem::s_ctx = nullptr;

	void NavMeshSystem::initialize() {
		s_ctx = new rcContext();
		AX_CORE_LOG_INFO("NavMeshSystem Initialized");
	}

	void NavMeshSystem::shutdown() {
		if (s_ctx) {
			delete s_ctx;
			s_ctx = nullptr;
		}
		AX_CORE_LOG_INFO("NavMeshSystem Shutdown");
	}

	Ref<NavMesh> NavMeshSystem::bakeScene(Scene* scene, const NavMeshBuildSettings& settings) {
		AX_CORE_LOG_INFO("NavMeshSystem Starting NavMesh Bake...");

		if (!scene) return nullptr;

		// ----- Gather geometry -----
		std::vector<float> verts;
		std::vector<int> tris;

		auto view = scene->getRegistry().view<RigidBodyComponent, TransformComponent>();

		for (auto [entityHandle, rigidbody, transform] : view.each()) {

			// -- Ignore dynamic physics objects --
			if (rigidbody.type != RigidBodyComponent::BodyType::Static) continue;

			Entity entity = { entityHandle, scene };
			Mat4 worldTransform = scene->getWorldTransform(entity);

			// ----- Extract Box Colliders -----
			if (entity.hasComponent<BoxColliderComponent>()) {
				auto& box = entity.getComponent<BoxColliderComponent>();

				int vOffset = (int)(verts.size() / 3);

				Vec3 h = box.halfExtents;
				Vec3 o = box.offset;

				// -- Generate 8 corners of the box in local space --
				Vec3 corners[8] = {
					Vec3(-h.x, -h.y, -h.z) + o, Vec3(h.x, -h.y, -h.z) + o,
					Vec3(h.x,  h.y, -h.z) + o, Vec3(-h.x,  h.y, -h.z) + o,
					Vec3(-h.x, -h.y,  h.z) + o, Vec3(h.x, -h.y,  h.z) + o,
					Vec3(h.x,  h.y,  h.z) + o, Vec3(-h.x,  h.y,  h.z) + o
				};

				// -- Transform 8 corners into World Space and push to Recast --
				for (int i = 0; i < 8; i++) {
					Vec4 worldPos = worldTransform * Vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
					verts.push_back(worldPos.x);
					verts.push_back(worldPos.y);
					verts.push_back(worldPos.z);
				}

				// -- Generate the 12 triangles to connect corners --
				int boxIndices[36] = {
					0, 1, 2, 0, 2, 3,
					6, 5, 4, 7, 6, 4,
					4, 5, 1, 4, 1, 0,
					3, 2, 6, 3, 6, 7,
					1, 5, 6, 1, 6, 2,
					4, 0, 3, 4, 3, 7
				};

				for (int i = 0; i < 36; i++) {
					tris.push_back(vOffset + boxIndices[i]);
				}
			}

			// ----- Extract Triangle Mesh Colliders -----
			if (entity.hasComponent<TriangleMeshColliderComponent>()) {
				auto& tmc = entity.getComponent<TriangleMeshColliderComponent>();

				if (tmc.collisionMesh.isValid()) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(tmc.collisionMesh);
					if (mesh) {
						int vOffset = (int)(verts.size() / 3);

						// -- Loop through all vertices --
						for (const auto& v : mesh->getVertices()) {
							Vec4 worldPos = worldTransform * Vec4(v.position.x, v.position.y, v.position.z, 1.0f);
							verts.push_back(worldPos.x);
							verts.push_back(worldPos.y);
							verts.push_back(worldPos.z);
						}

						// -- Loop through all indices --
						for (uint32_t index : mesh->getIndices()) {
							tris.push_back(vOffset + (int)index);
						}
					}
				}
			}

			// ----- Extract Convex Colliders -----
			if (entity.hasComponent<ConvexColliderComponent>()) {
				auto& cc = entity.getComponent<ConvexColliderComponent>();

				if (cc.collisionMesh.isValid()) {
					Ref<Mesh> mesh = AssetManager::get<Mesh>(cc.collisionMesh);
					if (mesh) {
						int vOffset = (int)(verts.size() / 3);

						// -- Loop through all vertices --
						for (const auto& v : mesh->getVertices()) {
							Vec4 worldPos = worldTransform * Vec4(v.position.x, v.position.y, v.position.z, 1.0f);
							verts.push_back(worldPos.x);
							verts.push_back(worldPos.y);
							verts.push_back(worldPos.z);
						}

						// -- Loop through all indices --
						for (uint32_t index : mesh->getIndices()) {
							tris.push_back(vOffset + (int)index);
						}
					}
				}
			}
		}

		if (verts.empty()) {
			AX_CORE_LOG_WARN("NavMeshSystem: Bake Aborted: No static geometry found!");
			return nullptr;
		}

		float bmin[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
		float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (size_t i = 0; i < verts.size(); i += 3) {
			rcVmin(bmin, &verts[i]);
			rcVmax(bmax, &verts[i]);
		}

		// ----- Recast configuration -----
		rcConfig cfg;
		memset(&cfg, 0, sizeof(cfg));
		cfg.cs = settings.cellSize;
		cfg.ch = settings.cellHeight;
		cfg.walkableSlopeAngle = settings.agentMaxSlope;
		cfg.walkableHeight = (int)ceilf(settings.agentHeight / cfg.ch);
		cfg.walkableClimb = (int)floorf(settings.agentMaxClimb / cfg.ch);
		cfg.walkableRadius = (int)ceilf(settings.agentRadius / cfg.cs);
		cfg.maxEdgeLen = (int)(12.0f / settings.cellSize);
		cfg.maxSimplificationError = 1.3f;
		cfg.minRegionArea = (int)rcSqr(8.0f);
		cfg.mergeRegionArea = (int)rcSqr(20.0f);
		cfg.maxVertsPerPoly = 6;
		cfg.detailSampleDist = 6.0f < 0.9f ? 0 : settings.cellSize * 6.0f;
		cfg.detailSampleMaxError = 1.0f;

		rcVcopy(cfg.bmin, bmin);
		rcVcopy(cfg.bmax, bmax);
		rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

		// ----- Voxelize -----
		rcHeightfield* solid = rcAllocHeightfield();
		rcCreateHeightfield(s_ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch);

		std::vector<unsigned char> triFlags(tris.size() / 3, 0);

		const int RC_WALKABLE_AREA = 63;
		for (int i = 0; i < (int)tris.size(); i += 3) {
			int v0 = tris[i] * 3;
			int v1 = tris[i + 1] * 3;
			int v2 = tris[i + 2] * 3;

			Vec3 p0(verts[v0], verts[v0 + 1], verts[v0 + 2]);
			Vec3 p1(verts[v1], verts[v1 + 1], verts[v1 + 2]);
			Vec3 p2(verts[v2], verts[v2 + 1], verts[v2 + 2]);

			Vec3 e0 = p1 - p0;
			Vec3 e1 = p2 - p0;
			Vec3 normal = e0.cross(e1).normalized();

			if (std::abs(normal.y) > 0.5f) {
				triFlags[i / 3] = RC_WALKABLE_AREA;
			}
		}

		rcRasterizeTriangles(s_ctx, verts.data(), verts.size() / 3, tris.data(), triFlags.data(), tris.size() / 3, *solid, cfg.walkableClimb);

		rcFilterLowHangingWalkableObstacles(s_ctx, cfg.walkableClimb, *solid);
		rcFilterLedgeSpans(s_ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
		rcFilterWalkableLowHeightSpans(s_ctx, cfg.walkableHeight, *solid);

		rcCompactHeightfield* chf = rcAllocCompactHeightfield();
		rcBuildCompactHeightfield(s_ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf);
		rcFreeHeightField(solid);

		rcErodeWalkableArea(s_ctx, cfg.walkableRadius, *chf);
		rcBuildDistanceField(s_ctx, *chf);
		rcBuildRegions(s_ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea);

		// ----- Polygons and Detour creation -----
		rcContourSet* cset = rcAllocContourSet();
		rcBuildContours(s_ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset);

		rcPolyMesh* pmesh = rcAllocPolyMesh();
		rcBuildPolyMesh(s_ctx, *cset, cfg.maxVertsPerPoly, *pmesh);

		if (pmesh->npolys == 0) {
			AX_CORE_LOG_ERROR("NavMesh Bake Failed: 0 polygons generated! Your floor is either too small for the Agent Radius, or missing colliders.");
			return nullptr;
		}

		rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
		rcBuildPolyMeshDetail(s_ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh);

		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);

		for (int i = 0; i < pmesh->npolys; ++i) {
			if (pmesh->areas[i] == RC_WALKABLE_AREA) {
				pmesh->areas[i] = 0;
				pmesh->flags[i] = 1;
			}
		}

		// -- Setup Detour Data Creation --
		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = pmesh->verts;
		params.vertCount = pmesh->nverts;
		params.polys = pmesh->polys;
		params.polyAreas = pmesh->areas;
		params.polyFlags = pmesh->flags;
		params.polyCount = pmesh->npolys;
		params.nvp = pmesh->nvp;
		params.detailMeshes = dmesh->meshes;
		params.detailVerts = dmesh->verts;
		params.detailVertsCount = dmesh->nverts;
		params.detailTris = dmesh->tris;
		params.detailTriCount = dmesh->ntris;
		params.walkableHeight = settings.agentHeight;
		params.walkableRadius = settings.agentRadius;
		params.walkableClimb = settings.agentMaxClimb;
		rcVcopy(params.bmin, pmesh->bmin);
		rcVcopy(params.bmax, pmesh->bmax);
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;

		// -- Convert Recast mesh to Detour binary data --
		unsigned char* navData = 0;
		int navDataSize = 0;
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
			AX_CORE_LOG_ERROR("Could not build Detour navmesh data.");
			return nullptr;
		}

		dtNavMesh* newMesh = dtAllocNavMesh();
		dtStatus status = newMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status)) {
			dtFree(navData);
			AX_CORE_LOG_ERROR("Could not initialize Detour navmesh.");
			return nullptr;
		}

		dtNavMeshQuery* newQuery = dtAllocNavMeshQuery();
		newQuery->init(newMesh, 2048);

		rcFreePolyMesh(pmesh);
		rcFreePolyMeshDetail(dmesh);

		AX_CORE_LOG_TRACE("NavMeshSystem Bake Complete!");

		return MakeRef<NavMesh>(newMesh, newQuery);
	}

	std::vector<Vec3> NavMeshSystem::calculatePath(Ref<NavMesh> navMesh, const Vec3& start, const Vec3& end) {
		std::vector<Vec3> waypoints;
		if (!navMesh || !navMesh->getDetourQuery()) {
			AX_CORE_LOG_WARN("[NavMeshSystem] Tried to calculate path, but no NavMesh is loaded!");
			return waypoints;
		}

		dtNavMeshQuery* query = navMesh->getDetourQuery();

		// -- Setup Query Filter --
		dtQueryFilter filter;
		filter.setIncludeFlags(0xFFFF);
		filter.setExcludeFlags(0);

		float extents[3] = { 5.0f, 10.0f, 5.0f };

		float startPos[3] = { start.x, start.y, start.z };
		float endPos[3] = { end.x, end.y, end.z };

		// -- Find nearest polygons to Start and End positions --
		dtPolyRef startRef;
		float nearestStart[3];
		query->findNearestPoly(startPos, extents, &filter, &startRef, nearestStart);

		dtPolyRef endRef;
		float nearestEnd[3];
		query->findNearestPoly(endPos, extents, &filter, &endRef, nearestEnd);

		if (!startRef || !endRef) {
			AX_CORE_LOG_WARN("Pathfinding failed: Start or End point is not near a walkable NavMesh.");
			return waypoints;
		}

		// -- Find the polygon path - A* Search --
		const int MAX_POLYS = 256;
		dtPolyRef path[MAX_POLYS];
		int pathCount = 0;
		query->findPath(startRef, endRef, nearestStart, nearestEnd, &filter, path, &pathCount, MAX_POLYS);

		if (pathCount > 0) {
			const int MAX_SMOOTH = 256;
			float straightPath[MAX_SMOOTH * 3];
			unsigned char straightPathFlags[MAX_SMOOTH];
			dtPolyRef straightPathPolys[MAX_SMOOTH];
			int straightPathCount = 0;

			query->findStraightPath(nearestStart, nearestEnd, path, pathCount, straightPath, straightPathFlags, straightPathPolys, &straightPathCount, MAX_SMOOTH);

			waypoints.reserve(straightPathCount);
			for (int i = 0; i < straightPathCount; ++i) {
				waypoints.push_back(Vec3(
					straightPath[i * 3],
					straightPath[i * 3 + 1],
					straightPath[i * 3 + 2]
				));
			}
		}

		return waypoints;
	}

	void NavMeshSystem::serializeToStream(Ref<NavMesh> navMesh, std::ostream& out) {
		if (!navMesh || !navMesh->getDetourMesh()) return;
		const dtNavMesh* constNavMesh = navMesh->getDetourMesh();

		uint32_t numTiles = 0;
		for (int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
			const dtMeshTile* tile = constNavMesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;
			numTiles++;
		}

		out.write(reinterpret_cast<const char*>(&numTiles), sizeof(uint32_t));

		for (int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
			const dtMeshTile* tile = constNavMesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) continue;

			dtTileRef tileRef = constNavMesh->getTileRef(tile);
			out.write(reinterpret_cast<const char*>(&tileRef), sizeof(dtTileRef));
			out.write(reinterpret_cast<const char*>(&tile->dataSize), sizeof(int));
			out.write(reinterpret_cast<const char*>(tile->data), tile->dataSize);
		}
	}

	Ref<NavMesh> NavMeshSystem::deserializeFromStream(std::istream& in) {
		uint32_t numTiles = 0;
		in.read(reinterpret_cast<char*>(&numTiles), sizeof(uint32_t));

		dtNavMesh* newMesh = dtAllocNavMesh();

		if (numTiles == 1) {
			dtTileRef tileRef;
			int dataSize;
			in.read(reinterpret_cast<char*>(&tileRef), sizeof(dtTileRef));
			in.read(reinterpret_cast<char*>(&dataSize), sizeof(int));

			unsigned char* data = (unsigned char*)dtAlloc(dataSize, DT_ALLOC_PERM);
			in.read(reinterpret_cast<char*>(data), dataSize);

			dtStatus status = newMesh->init(data, dataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status)) {
				dtFree(data);
				AX_CORE_LOG_ERROR("Failed to initialize Detour NavMesh from file.");
			}
		}
		else {
			AX_CORE_LOG_ERROR("Multi-tile NavMesh loading is not supported yet!");
		}

		dtNavMeshQuery* newQuery = dtAllocNavMeshQuery();
		newQuery->init(newMesh, 2048);
		return MakeRef<NavMesh>(newMesh, newQuery);
	}

	std::vector<NavMeshEdge> NavMeshSystem::getDebugEdges(Ref<NavMesh> navMesh) {
		std::vector<NavMeshEdge> edges;
		if (!navMesh) return edges;

		const dtNavMesh* constNavMesh = navMesh->getDetourMesh();
		for (int i = 0; i < constNavMesh->getMaxTiles(); ++i) {
			const dtMeshTile* tile = constNavMesh->getTile(i);
			if (!tile || !tile->header) continue;

			for (int j = 0; j < tile->header->polyCount; ++j) {
				const dtPoly* poly = &tile->polys[j];

				if (poly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION) continue;

				for (int k = 0; k < poly->vertCount; ++k) {
					int v0 = poly->verts[k] * 3;
					int v1 = poly->verts[(k + 1) % poly->vertCount] * 3;

					Vec3 p0(tile->verts[v0], tile->verts[v0 + 1], tile->verts[v0 + 2]);
					Vec3 p1(tile->verts[v1], tile->verts[v1 + 1], tile->verts[v1 + 2]);

					edges.push_back({ p0, p1 });
				}
			}
		}
		return edges;
	}

	static float detourRand() { return (float)rand() / (float)RAND_MAX; }

	Vec3 NavMeshSystem::getNearestPoint(Ref<NavMesh> navMesh, const Vec3& point) {
		if (!navMesh || !navMesh->getDetourQuery()) return point;

		dtQueryFilter filter; filter.setIncludeFlags(0xFFFF); filter.setExcludeFlags(0);
		float extents[3] = { 5.0f, 10.0f, 5.0f };
		float center[3] = { point.x, point.y, point.z };

		dtPolyRef nearestRef;
		float nearestPt[3];
		navMesh->getDetourQuery()->findNearestPoly(center, extents, &filter, &nearestRef, nearestPt);

		if (nearestRef) return Vec3(nearestPt[0], nearestPt[1], nearestPt[2]);
		return point;
	}

	Vec3 NavMeshSystem::getRandomPointAroundCircle(Ref<NavMesh> navMesh, const Vec3& center, float radius) {
		if (!navMesh || !navMesh->getDetourQuery()) return center;

		dtQueryFilter filter; filter.setIncludeFlags(0xFFFF); filter.setExcludeFlags(0);
		float extents[3] = { 5.0f, 10.0f, 5.0f };
		float centerPos[3] = { center.x, center.y, center.z };

		dtPolyRef startRef;
		float nearestPt[3];
		navMesh->getDetourQuery()->findNearestPoly(centerPos, extents, &filter, &startRef, nearestPt);

		if (!startRef) return center;

		dtPolyRef randomRef;
		float randomPt[3];
		navMesh->getDetourQuery()->findRandomPointAroundCircle(startRef, nearestPt, radius, &filter, detourRand, &randomRef, randomPt);

		if (randomRef) return Vec3(randomPt[0], randomPt[1], randomPt[2]);
		return center;
	}

	bool NavMeshSystem::raycast(Ref<NavMesh> navMesh, const Vec3& start, const Vec3& end, Vec3& outHitPoint) {
		if (!navMesh || !navMesh->getDetourQuery()) return false;

		dtQueryFilter filter; filter.setIncludeFlags(0xFFFF); filter.setExcludeFlags(0);
		float extents[3] = { 5.0f, 10.0f, 5.0f };
		float startPos[3] = { start.x, start.y, start.z };
		float endPos[3] = { end.x, end.y, end.z };

		dtPolyRef startRef;
		float nearestPt[3];
		navMesh->getDetourQuery()->findNearestPoly(startPos, extents, &filter, &startRef, nearestPt);

		if (!startRef) return true;

		float t = 0;
		float hitNormal[3];
		dtPolyRef path[256];
		int pathCount = 0;

		navMesh->getDetourQuery()->raycast(startRef, startPos, endPos, &filter, &t, hitNormal, path, &pathCount, 256);

		if (t < 1.0f) {
			outHitPoint = start + (end - start) * t;
			return true;
		}

		outHitPoint = end;
		return false;
	}

}
