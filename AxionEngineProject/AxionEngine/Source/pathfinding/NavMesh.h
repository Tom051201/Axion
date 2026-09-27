#pragma once

#include "AxionEngine/Source/core/Ref.h"

class dtNavMesh;
class dtNavMeshQuery;

namespace Axion {

	class NavMesh : public RefCounted {
	public:

		NavMesh(dtNavMesh* mesh, dtNavMeshQuery* query);
		~NavMesh();

		void release();

		dtNavMesh* getDetourMesh() const { return m_navMesh; }
		dtNavMeshQuery* getDetourQuery() const { return m_navQuery; }

	private:

		dtNavMesh* m_navMesh = nullptr;
		dtNavMeshQuery* m_navQuery = nullptr;

	};

}
