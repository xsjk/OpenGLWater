#version 330

smooth in vec2 fragTC;		// Interpolated texture coordinates

uniform sampler2D prevTex;	// Texture sampler
uniform sampler2D currTex;

uniform sampler2D islandsTex;

uniform vec2 mousePos;

out vec4 outCol;	// Final pixel color

void main() {
	// Get pixel location of this fragment
	ivec2 texelCoord = ivec2(fragTC * textureSize(prevTex, 0));

	int width = textureSize(prevTex, 0).x;
	int height = textureSize(prevTex, 0).y;

	// Get adjacent texels and do edge detection
	float c = texelFetch(currTex, texelCoord, 0).r;

	ivec2 coord = texelCoord + ivec2(-4, 0);
	if (coord.x < 0)
		coord.x = 0;
	float islandTexel = texelFetch(islandsTex, coord, 0).r;
	if (islandTexel < 0.5f)
		coord = texelCoord;
	float l = texelFetch(prevTex, coord, 0).r;

	coord = texelCoord + ivec2(0, -4);
	if (coord.y < 0)
		coord.y = 0;
	islandTexel = texelFetch(islandsTex, coord, 0).r;
	if (islandTexel < 0.5f)
		coord = texelCoord;
	float t = texelFetch(prevTex, coord, 0).r;

	coord = texelCoord + ivec2(4, 0);
	if (coord.x > width - 1)
		coord.x = width - 1;
	islandTexel = texelFetch(islandsTex, coord, 0).r;
	if (islandTexel < 0.5f)
		coord = texelCoord;
	float r = texelFetch(prevTex, coord, 0).r;

	coord = texelCoord + ivec2(0, 4);
	if (coord.y > height - 1)
		coord.y = height - 1;
	islandTexel = texelFetch(islandsTex, coord, 0).r;
	if (islandTexel < 0.5f)
		coord = texelCoord;
	float b = texelFetch(prevTex, coord, 0).r;
	
	float offset = 0.0f;

	// Mouse interaction
	if (mousePos.x > 0.0f && mousePos.x < 1.0f) {
		if (length(mousePos - fragTC) < 0.02f)
			offset = -0.2f;
	}

	// Wave equation
	offset += (l + t + r + b) * 0.5f - c;
	offset *= 0.998f; // Damping

	// Exclude islands
	islandTexel = texelFetch(islandsTex, texelCoord, 0).r;
	if (islandTexel < 0.5f)
		offset = 0.0f;

	coord = texelCoord + ivec2(4, 0);
	vec3 ddx = vec3(4.0f, texelFetch(prevTex, coord, 0).r - offset, 0.0f);
	coord = texelCoord + ivec2(0, 4);
	vec3 ddy = vec3(0.0f, texelFetch(prevTex, coord, 0).r - offset, 4.0f);
	vec2 normal = normalize(cross(ddy, ddx)).xz;

	outCol = vec4(offset, normal, 1.0f);
}
