#type vertex
#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aCol;
layout(location = 2) in vec2 aUV;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vUV;

layout(std140, binding = 0) uniform SceneBuffer {
	mat4 u_viewProjection;
};

layout(std140, binding = 1) uniform ObjectBuffer {
	vec4 u_color;
	mat4 u_modelMatrix;
};

void main() {
	vec4 worldPos = u_modelMatrix * vec4(aPos, 1.0);
	gl_Position = u_viewProjection * worldPos;

	vColor = u_color;
	vUV = aUV;
}


#type fragment
#version 450 core

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vUV;

layout(location = 0) out vec4 FragColor;

void main() {
	FragColor = vColor;
}
