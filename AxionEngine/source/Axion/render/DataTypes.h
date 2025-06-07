#pragma once

namespace Axion {

	struct Vertex {
		float position[3];
		float color[4];
		float texCoords[2];

		Vertex(float x, float y, float z, float r, float g, float b, float a, float tx, float ty) {
			position[0] = x; position[1] = y; position[2] = z;
			color[0] = r; color[1] = g; color[2] = b; color[3] = a;
			texCoords[0] = tx; texCoords[1] = ty;
		}

		Vertex(float x, float y, float z, float r, float g, float b, float a) {
			position[0] = x; position[1] = y; position[2] = z;
			color[0] = r; color[1] = g; color[2] = b; color[3] = a;
			texCoords[0] = 0; texCoords[1] = 0;
		}
	};

}
