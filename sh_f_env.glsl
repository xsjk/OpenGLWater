#version 330

in vec3 worldPos;
in float depth;

out vec4 outCol;	// Final pixel color

void main() {
	outCol = vec4(worldPos, depth);
}
