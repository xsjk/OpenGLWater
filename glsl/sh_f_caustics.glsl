#version 330

in vec3 oldPos;
in vec3 newPos;
in float waterDepth;
in float depth;

out vec4 outCol;	// Final pixel color

void main() {
	float intensity = 0.0f;

	//if (depth >= waterDepth) {
		float oldArea = length(dFdx(oldPos)) * length(dFdy(oldPos));
		float newArea = length(dFdx(newPos)) * length(dFdy(newPos));

		float ratio;

		if (newArea == 0.0f)
			ratio = 2000.0f;
		else
			ratio = oldArea / newArea;

		intensity = 0.15f * ratio;
	//}

	outCol = vec4(intensity, depth, intensity, 1.0f);
}
