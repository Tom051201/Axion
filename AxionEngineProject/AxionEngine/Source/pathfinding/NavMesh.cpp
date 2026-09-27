#include "axpch.h"
#include "NavMesh.h"

#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

namespace Axion {

	NavMesh::NavMesh(dtNavMesh* mesh, dtNavMeshQuery* query)
		: m_navMesh(mesh), m_navQuery(query) {}

	NavMesh::~NavMesh() {
		release();
	}

	void NavMesh::release() {
		if (m_navMesh) {
			dtFreeNavMesh(m_navMesh);
			m_navMesh = nullptr;
		}
		if (m_navQuery) {
			dtFreeNavMeshQuery(m_navQuery);
			m_navQuery = nullptr;
		}
	}

}
