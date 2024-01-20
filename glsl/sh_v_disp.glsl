#version 330

layout(location = 0) in vec3 pos;		// Position
layout(location = 1) in vec2 tc;		// Texture coordinates
layout(location = 2) in int material;

smooth out vec2 fragTC;		// Interpolated texture coordinate

uniform mat4 xform;			// Transformation matrix
uniform mat4 lightViewXform;

uniform sampler2D waterTex;		// Texture samplers
uniform sampler2D islandsTex;

uniform vec3 camPos;

flat out int mtrl;
out vec4 worldPos;
out vec4 screenPos;
out vec3 skyboxTC;
out vec3 eyePos;
out vec3 lightViewPos;

const int MAT_WATER_SURF = 1;
const int MAT_WATER = 2;
const int MAT_TERR = 5;

void main() {
	worldPos = vec4(pos, 1.0f);

	mtrl = material;

	// Initial fix of water body/surface material interpolation
	if (mtrl == MAT_WATER_SURF && pos.y != 0.0f)
		mtrl = MAT_WATER;

	// Offset water surface
	if (mtrl == MAT_WATER_SURF) {
		float offset = texture2D(waterTex, (worldPos.xz + 1.0f) * 0.5f).r * 0.16f;
		worldPos.y += offset;
	}

	// Offset Terrain
	if (mtrl == MAT_TERR) {
		float offset = (1.0f - texture2D(islandsTex, (worldPos.xz + 1.0f) * 0.5f).r);
		worldPos.y += offset;
	}

	// Transform vertex position
	screenPos = xform * worldPos;
	gl_Position = screenPos;

	// For water fresnel calculation
	eyePos = camPos - screenPos.xyz;

	// Interpolate texture coordinates
	fragTC = tc;
	skyboxTC = pos;

	vec4 lightViewPosW = lightViewXform * worldPos;
	lightViewPos = 0.5f + lightViewPosW.xyz / lightViewPosW.w * 0.5f;
}
