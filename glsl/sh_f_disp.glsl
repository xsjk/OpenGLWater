#version 330

smooth in vec2 fragTCOut;		// Interpolated texture coordinates

flat in int mtrlOut;
in vec4 screenPosOut;
in vec3 skyboxTCOut;
in vec3 normal;
in vec3 eyePosOut;
in vec3 lightViewPosOut;

uniform sampler2D waterTex;		// Texture samplers
uniform sampler2D wallTex;
uniform sampler2D terrTex;
uniform sampler2D refractionTex;
uniform sampler2D reflectionTex;
uniform samplerCube skybox;
uniform sampler2D causticsTex;

uniform vec3 lightDir;

out vec4 outCol;	// Final pixel color

const int MAT_WATER_SURF = 1;
const int MAT_WATER = 2;
const int MAT_WALL = 3;
const int MAT_SKYBOX = 4;
const int MAT_TERR = 5;

float blur(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
	float intensity = 0.;
	vec2 off1 = vec2(1.3846153846) * direction;
	vec2 off2 = vec2(3.2307692308) * direction;
	intensity += texture2D(image, uv).x * 0.2270270270;
	intensity += texture2D(image, uv + (off1 / resolution)).x * 0.3162162162;
	intensity += texture2D(image, uv - (off1 / resolution)).x * 0.3162162162;
	intensity += texture2D(image, uv + (off2 / resolution)).x * 0.0702702703;
	intensity += texture2D(image, uv - (off2 / resolution)).x * 0.0702702703;
	return intensity;
}

float causticsIntensity() {
	float lightIntensity = 0.2f;
	float causticsDepth = texture2D(causticsTex, lightViewPosOut.xy).g;

	if (causticsDepth > lightViewPosOut.z - 0.005f) {
		lightIntensity = -dot(lightDir, normal) * 0.5f;
		float causticsIntensity = 1.0f * (
			blur(causticsTex, lightViewPosOut.xy, vec2(512), vec2(0., 0.5)) +
			blur(causticsTex, lightViewPosOut.xy, vec2(512), vec2(0.5, 0.)));
		lightIntensity += causticsIntensity;
	}
	return lightIntensity;
}

void main() {
	vec3 waterColor = vec3(0.1f, 0.43f, 0.5f);

	switch (mtrlOut) {
		case MAT_WATER_SURF:
		{
			vec4 tangent = vec4(1.0f, 0.0f, 0.0f, 0.0f);
			vec4 viewNormal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
			vec4 bitangent = vec4(0.0f, 0.0f, 1.0f, 0.0f);
			vec4 viewDir = normalize(vec4(eyePosOut, 1.0f));
			vec4 viewTanSpace = normalize(vec4(dot(viewDir, tangent), dot(viewDir, bitangent),
			dot(viewDir, viewNormal), 1.0f));
			vec4 viewReflection = normalize(reflect(-1.0f * viewTanSpace, vec4(normal, 1.0f)));
			float fresnel = dot(vec4(normal, 1.0f), viewReflection);

			vec2 refractCoord = screenPosOut.xy / screenPosOut.w / 2.0f + 0.5f;
			vec2 reflectCoord = refractCoord;
			reflectCoord.y = 1.0f - refractCoord.y;
			vec4 offset = vec4(normal, 1.0f) * 0.05f;
			vec4 refraction = texture2D(refractionTex, refractCoord + offset.xz);
			refraction = mix(refraction, vec4(waterColor, 1.0f), 0.8f);
			vec4 reflection = texture2D(reflectionTex, reflectCoord + offset.xz);

			vec3 specular = vec3(clamp(pow(dot(viewTanSpace, vec4(normal, 1.0f)), 255.0), 0.0, 1.0)) * 2.5f;

			outCol = mix(reflection, refraction, fresnel);
			break;
		}

		case MAT_WATER:
		{
			outCol = vec4(waterColor, 0.4f);
			break;
		}

		case MAT_WALL:
		{
			vec3 color = texture2D(wallTex, fragTCOut).xyz * causticsIntensity();
			outCol = vec4(color, 1.0f);
			break;
		}

		case MAT_SKYBOX:
		{
			outCol = texture(skybox, skyboxTCOut);
			break;
		}

		case MAT_TERR:
		{
			vec3 color = texture2D(terrTex, fragTCOut).xyz * 0.7f * causticsIntensity();
			outCol = vec4(color, 1.0f);
			break;
		}

		case 0:
		{
			vec3 color = vec3(1.0f) * causticsIntensity();
			outCol = vec4(color, 1.0f);
			break;
		}
	}
}
