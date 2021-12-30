#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

smooth in vec2 fragTC[3];
flat in int mtrl[3];
in vec4 worldPos[3];
in vec4 screenPos[3];
in vec3 skyboxTC[3];
in vec3 eyePos[3];
in vec3 lightViewPos[3];

uniform vec4 clipPlane;

smooth out vec2 fragTCOut;
flat out int mtrlOut;
out vec4 screenPosOut;
out vec3 skyboxTCOut;
out vec3 normal;
out vec3 eyePosOut;
out vec3 lightViewPosOut;

const int MAT_WATER_SURF = 1;
const int MAT_WATER = 2;

void main() {
    // Normal calculation
    vec3 n = cross(worldPos[1].xyz - worldPos[0].xyz, worldPos[2].xyz - worldPos[0].xyz);
    n = normalize(n);

    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        gl_ClipDistance[0] = dot(worldPos[i], clipPlane);

        // Pass everything onto fragment shader
        fragTCOut = fragTC[i];
        mtrlOut = mtrl[i];
        // Secondary fix of water body/surface material interpolation
        if (mtrl[i] == MAT_WATER_SURF && n.y == 0.0f)
            mtrlOut = MAT_WATER;
        screenPosOut = screenPos[i];
        skyboxTCOut = skyboxTC[i];
        normal = n;
        eyePosOut = eyePos[i];
        lightViewPosOut = lightViewPos[i];

        EmitVertex();
    }
}
