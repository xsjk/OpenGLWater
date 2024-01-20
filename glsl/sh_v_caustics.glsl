#version 330

layout(location = 0) in vec3 pos;		// Position
layout(location = 1) in vec2 tc;		// Texture coordinates
layout(location = 2) in int material;

uniform mat4 xform;			// Transformation matrix

uniform sampler2D waterTex;		// Texture samplers
uniform sampler2D envTex;

uniform vec3 lightDir;

out vec3 oldPos;
out vec3 newPos;
out float waterDepth;
out float depth;

void main() {
	vec4 worldPos = vec4(pos, 1.0f);
	vec4 waterInfo = texture2D(waterTex, (worldPos.xz + 1.0f) * 0.5f);
	float offset = waterInfo.r * 0.16f;
	if (material == 1)
		worldPos.y += offset;
	vec3 normal = normalize(vec3(waterInfo.g, 1.0f, waterInfo.b)).xyz;

	oldPos = worldPos.xyz;

	vec4 screenPos = xform * worldPos;

	vec2 currPos = screenPos.xy / screenPos.w;
	vec2 coord = 0.5f + 0.5f * currPos;

	vec3 refractedDir = refract(lightDir, normal, 0.7504f);
	vec4 screenRefDir = xform * vec4(refractedDir, 1.0f);
	
	waterDepth = 0.5f + 0.5f * screenPos.z / screenPos.w;
	float currDepth = screenPos.z;
	vec4 env = texture2D(envTex, coord);

	float factor = 1.0f / (length(screenRefDir.xy) * 512.0f);

	/*vec2 deltaDir = screenRefDir.xy * factor;
	float deltaDepth = screenRefDir.z * factor;

	for (int i = 0; i < 50; i++) {
		currPos += deltaDir;
		currDepth += deltaDepth;

		if (env.w <= currDepth)
			break;

		env = texture2D(envTex, 0.5f + 0.5f * currPos);
	}*/

	newPos = env.xyz;

	vec4 screenEnvPos = xform * vec4(newPos, 1.0f);
	depth = 0.5f + 0.5f * screenEnvPos.z / screenEnvPos.w;

	// Transform vertex position
	gl_Position = screenEnvPos;
}
