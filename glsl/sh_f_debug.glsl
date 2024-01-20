#version 330

smooth in vec2 fragTC;		// Interpolated texture coordinates

uniform sampler2D tex;		// Texture sampler

out vec4 outCol;	// Final pixel color

void main() {
	outCol = vec4(texture(tex, fragTC).rrr, 1.0f);
	//outCol = texture(tex, fragTC);
}
