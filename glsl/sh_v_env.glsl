#version 330

layout(location = 0) in vec3 pos;		// Position
layout(location = 1) in vec2 tc;		// Texture coordinates
layout(location = 2) in int material;

uniform mat4 xform;			// Transformation matrix

uniform sampler2D islandsTex;		// Texture samplers

out vec3 worldPos;
out float depth;

void main() {
	worldPos = pos;
	// Offset Terrain
	if (material == 5) {
		float offset = (1.0f - texture2D(islandsTex, (worldPos.xz + 1.0f) * 0.5f).r);
		worldPos.y += offset;
	}
	vec4 screenPos = xform * vec4(worldPos, 1.0f);

	// Transform vertex position
	gl_Position = screenPos;

	depth = screenPos.z;
}
