#include <iostream>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl_core_3_3.h"
#include <GL/freeglut.h>
#include "util.hpp"
#include "mesh.hpp"
#include "stb_image.h"

#include "PerlinNoise.hpp"

using namespace std;

// Vertex format
struct vert {
	glm::vec3 pos;		// 3D Position
	glm::vec2 tc;		// Texture coordinate
	int material;		// Material identifier
};

// Global state
GLint width, height;				// Window size
int texWidth, texHeight;			// Texture size (Both water and terrrain buffer textures)

vector<glm::u8vec3> initTexData;		// Texture data
vector<glm::u8vec3> islandsTexData;		// Terrain height buffer texture
unsigned char* wallTexData;
unsigned char* terrTexData;				// Terrain shading texture
vector<glm::u8vec3> refractionTexData;
vector<glm::u8vec3> reflectionTexData;
vector<glm::u8vec4> envMapData;
vector<glm::u8vec4> causticsMapData;

GLuint prevTexture;		// Texture objects
GLuint currTexture;
GLuint islandsTexture;
GLuint wallTexture;
GLuint terrTexture;
GLuint refractionTexture;
GLuint reflectionTexture;
GLuint skyboxTexture;
GLuint environmentMap;
GLuint causticsMap;

GLuint gpgpuShader;		// Shader programs
GLuint dispShader;
GLuint envShader;
GLuint causticsShader;
GLuint debugShader;
GLuint fbo;				// Framebuffer object

GLuint uniXform;		// Uniform shader parameters
GLuint uniClipPlane;
GLuint uniMousePos;
GLuint uniCamPos;
GLuint uniEnvXform;
GLuint uniCausticsXform;
GLuint uniLightDir;
GLuint uniLightViewXform;
GLuint uniLightDirDisp;

glm::vec2 mousePos;

mt19937 rng;
uniform_int_distribution<> noise;
siv::PerlinNoise perlin;			// For random terrain generation

GLuint vao;				// Vertex array object
GLuint vbuf;			// Vertex buffer
GLuint ibuf;			// Index buffer
GLsizei vcount;			// Number of vertices

GLuint waterVtsX, waterVtsY;
GLuint waterVao;
GLuint waterVbuf;
GLuint waterIbuf;
GLuint waterSurfVao;
GLuint waterSurfVbuf;
GLuint waterSurfIbuf;
GLsizei waterVcount;
GLsizei waterSurfVcount;
vector<vert> waterVerts;
vector<vert> waterSurfVerts;
vector<GLuint> waterIds;
vector<GLuint> waterSurfIds;

GLuint wallVao;
GLuint wallVbuf;
GLuint wallIbuf;
GLsizei wallVcount;
vector<vert> wallVerts;
vector<GLuint> wallIds;

GLuint skyVao;
GLuint skyVbuf;
GLuint skyIbuf;
GLsizei skyVcount;
vector<vert> skyVerts;
vector<GLuint> skyIds;

GLuint terrVtsX, terrVtsY;
GLuint terrVao;
GLuint terrVbuf;
GLuint terrIbuf;
GLsizei terrVcount;
vector<vert> terrVerts;
vector<GLuint> terrIds;

bool enableTerrain;

// CAUSTICS VARIABLES
glm::vec3 lightPos;

Mesh* mesh;				// Mesh loaded from .obj file

// Camera state
glm::vec3 camCoords;		// Spherical coordinates (theta, phi, radius) of the camera
bool camRot;				// Whether the camera is currently rotating
glm::vec2 camOrigin;		// Original camera coordinates upon clicking
glm::vec2 mouseOrigin;		// Original mouse coordinates upon clicking

// Constants
const int MENU_EXIT = 0;			// Exit application
const int MENU_TERR = 1;			// Toggle terrain
const int MENU_RESEED = 2;			// Reseed random perlin terrain

const int MAT_WATER_SURF = 1;
const int MAT_WATER = 2;
const int MAT_WALL = 3;
const int MAT_SKYBOX = 4;
const int MAT_TERR = 5;

// Initialization functions
void initState();
void initGLUT(int* argc, char** argv);
void initOpenGL();
void initGeometry();
void initTextures();

void initWaterMesh();
void initWallsMesh();
void initWallTexture();
void initTerrTexture();
void initSkybox();
void initTerrain();

// Callback functions
void display();
void reshape(GLint width, GLint height);
void keyRelease(unsigned char key, int x, int y);
void mouseBtn(int button, int state, int x, int y);
void mouseMove(int x, int y);
void idle();
void menu(int cmd);
void cleanup();

// Other functions
void generateIslands();
GLuint loadSkybox(vector<std::string> faces);

int main(int argc, char** argv) {
	try {
		// Initialize
		initState();
		initGLUT(&argc, argv);
		initOpenGL();
		initGeometry();
		initTextures();

	} catch (const exception& e) {
		// Handle any errors
		cerr << "Fatal error: " << e.what() << endl;
		cleanup();
		return -1;
	}

	// Execute main loop
	glutMainLoop();

	return 0;
}

void initState() {
	// Initialize global state
	width = 0;
	height = 0;
	texWidth = 512;
	texHeight = 512;

	prevTexture = 0;
	currTexture = 0;
	islandsTexture = 0;
	wallTexture = 0;
	terrTexData = 0;
	refractionTexture = 0;
	reflectionTexture = 0;
	skyboxTexture = 0;
	environmentMap = 0;
	causticsMap = 0;

	gpgpuShader = 0;
	dispShader = 0;
	envShader = 0;
	causticsShader = 0;
	debugShader = 0;
	fbo = 0;

	uniXform = 0;
	uniClipPlane = 0;
	uniMousePos = 0;
	uniCamPos = 0;
	uniEnvXform = 0;
	uniCausticsXform = 0;
	uniLightDir = 0;
	uniLightViewXform = 0;
	uniLightDirDisp = 0;

	vao = 0;
	vbuf = 0;
	ibuf = 0;
	vcount = 0;

	mousePos = glm::vec2(-2.0f, -2.0f);

	random_device rd;
	rng = mt19937(rd());
	noise = uniform_int_distribution<>(0, 256);

	waterVtsX = 128;
	waterVtsY = 128;
	waterVao = 0;
	waterVbuf = 0;
	waterIbuf = 0;
	waterVcount = 0;
	waterSurfVcount = 0;

	wallVao = 0;
	wallVbuf = 0;
	wallIbuf = 0;
	wallVcount = 0;

	skyVao = 0;
	skyVbuf = 0;
	skyIbuf = 0;
	skyVcount = 0;

	terrVtsX = 128;
	terrVtsY = 128;
	terrVao = 0;
	terrVbuf = 0;
	terrIbuf = 0;
	terrVcount = 0;

	enableTerrain = false;

	lightPos = glm::vec3(1.0f, 2.0f, 1.0f);

	mesh = NULL;

	camCoords = glm::vec3(30.0f, 15.0f, 3.0f);
	camRot = false;
}

void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	width = 800; height = 600;
	glutInit(argc, argv);
	glutInitWindowSize(width, height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("GPU it!");

	// Create a menu
	int menuTerrain = glutCreateMenu(menu);
	glutAddMenuEntry("Toggle terrain", MENU_TERR);
	glutAddMenuEntry("Reseed", MENU_RESEED);

	glutCreateMenu(menu);
	glutAddSubMenu("Terrain", menuTerrain);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyRelease);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);
}

void initOpenGL() {
	// Set clear color and depth
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	// Enable depth testing
	glEnable(GL_DEPTH_TEST);
	// Allow unpacking non-aligned pixel data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Enable back face culling for walls and skybox rendering
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

	// Enable transparent rendering for water body
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compile and link display shader
	vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "glsl/sh_v_disp.glsl"));
	shaders.push_back(compileShader(GL_GEOMETRY_SHADER, "glsl/sh_g_disp.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "glsl/sh_f_disp.glsl"));
	dispShader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();

	// Compile and link GPGPU shader
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "glsl/sh_v_gpgpu.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "glsl/sh_f_gpgpu.glsl"));
	gpgpuShader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();

	// Compile and link environment mapping shader
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "glsl/sh_v_env.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "glsl/sh_f_env.glsl"));
	envShader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();

	// Compile and link caustics shader
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "glsl/sh_v_caustics.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "glsl/sh_f_caustics.glsl"));
	causticsShader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();

	// Compile and link debug shader
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "glsl/sh_v_debug.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "glsl/sh_f_debug.glsl"));
	debugShader = linkProgram(shaders);
	// Release shader sources
	for (auto s = shaders.begin(); s != shaders.end(); ++s)
		glDeleteShader(*s);
	shaders.clear();

	// Locate uniforms
	uniXform = glGetUniformLocation(dispShader, "xform");
	uniClipPlane = glGetUniformLocation(dispShader, "clipPlane");
	uniCamPos = glGetUniformLocation(dispShader, "camPos");
	uniLightViewXform = glGetUniformLocation(dispShader, "lightViewXform");
	uniMousePos = glGetUniformLocation(gpgpuShader, "mousePos");
	uniEnvXform = glGetUniformLocation(envShader, "xform");
	uniCausticsXform = glGetUniformLocation(causticsShader, "xform");
	uniLightDir = glGetUniformLocation(causticsShader, "lightDir");
	uniLightDirDisp = glGetUniformLocation(dispShader, "lightDir");

	// Bind texture image units
	GLuint uniTex = glGetUniformLocation(gpgpuShader, "prevTex");
	glUseProgram(gpgpuShader);
	glUniform1i(uniTex, 0);
	uniTex = glGetUniformLocation(gpgpuShader, "currTex");
	glUniform1i(uniTex, 1);
	uniTex = glGetUniformLocation(gpgpuShader, "islandsTex");
	glUniform1i(uniTex, 2);

	uniTex = glGetUniformLocation(dispShader, "waterTex");
	glUseProgram(dispShader);
	glUniform1i(uniTex, 0);
	uniTex = glGetUniformLocation(dispShader, "islandsTex");
	glUniform1i(uniTex, 1);
	uniTex = glGetUniformLocation(dispShader, "wallTex");
	glUniform1i(uniTex, 2);
	uniTex = glGetUniformLocation(dispShader, "terrTex");
	glUniform1i(uniTex, 3);
	uniTex = glGetUniformLocation(dispShader, "refractionTex");
	glUniform1i(uniTex, 4);
	uniTex = glGetUniformLocation(dispShader, "reflectionTex");
	glUniform1i(uniTex, 5);
	uniTex = glGetUniformLocation(dispShader, "skybox");
	glUniform1i(uniTex, 6);
	uniTex = glGetUniformLocation(dispShader, "causticsTex");
	glUniform1i(uniTex, 7);
	glUseProgram(0);

	uniTex = glGetUniformLocation(envShader, "islandsTex");
	glUseProgram(envShader);
	glUniform1i(uniTex, 0);

	uniTex = glGetUniformLocation(causticsShader, "waterTex");
	glUseProgram(causticsShader);
	glUniform1i(uniTex, 0);
	uniTex = glGetUniformLocation(causticsShader, "envTex");
	glUniform1i(uniTex, 1);

	uniTex = glGetUniformLocation(debugShader, "tex");
	glUseProgram(debugShader);
	glUniform1i(uniTex, 0);

	assert(glGetError() == GL_NO_ERROR);
}

void initGeometry() {
	// Vertex format
	struct vert {
		glm::vec2 pos;
		glm::vec2 tc;
	};
	// Create a surface (quad) to draw the texture onto
	vector<vert> verts = {
		{ glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
		{ glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
		{ glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
		{ glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f) },
	};
	// Vertex indices for triangles
	vector<GLuint> ids = {
		0, 1, 2,	// Triangle 1
		2, 3, 0		// Triangle 2
	};
	vcount = int(ids.size());

	// Create vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create vertex buffer
	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(vert), verts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vert), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), (GLvoid*)sizeof(glm::vec2));
	// Create index buffer
	glGenBuffers(1, &ibuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ids.size() * sizeof(GLuint), ids.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	initWaterMesh();
	initWallsMesh();
	initSkybox();
	initTerrain();
}

void initWaterMesh() {
	// Surface
	for (unsigned j = 0; j < waterVtsY; j++) {
		for (unsigned i = 0; i < waterVtsX; i++) {
			vert v = {
				glm::vec3((float)i / (float)(waterVtsX - 1) * 2.0f - 1.0f,
				0.0f, (float)j / (float)(waterVtsY - 1) * 2.0f - 1.0f),
				glm::vec2((float)i / (float)(waterVtsX - 1),
				(float)j / (float)(waterVtsY - 1)), MAT_WATER_SURF
			};
			waterVerts.push_back(v);
			waterSurfVerts.push_back(v);
		}
	}
	// Bottom
	for (unsigned j = 0; j < waterVtsY; j++) {
		for (unsigned i = 0; i < waterVtsX; i++) {
			vert v = {
				glm::vec3((float)i / (float)(waterVtsX - 1) * 2.0f - 1.0f,
				-1.0f, (float)j / (float)(waterVtsY - 1) * 2.0f - 1.0f),
				glm::vec2((float)i / (float)(waterVtsX - 1),
				(float)j / (float)(waterVtsY - 1)), MAT_WATER
			};
			waterVerts.push_back(v);
		}
	}

	// Vertex indices for triangles
	// Surface
	for (unsigned i = 0; i < waterVtsX * waterVtsY - waterVtsX; i++) {
		if (i % waterVtsY == (waterVtsY - 1))	continue;
		waterIds.push_back(i);
		waterSurfIds.push_back(i);
		waterIds.push_back(i + waterVtsX);
		waterSurfIds.push_back(i + waterVtsX);
		waterIds.push_back(i + 1);
		waterSurfIds.push_back(i + 1);

		waterIds.push_back(i + 1);
		waterSurfIds.push_back(i + 1);
		waterIds.push_back(i + waterVtsX);
		waterSurfIds.push_back(i + waterVtsX);
		waterIds.push_back(i + waterVtsX + 1);
		waterSurfIds.push_back(i + waterVtsX + 1);
	}
	// Bottom
	for (unsigned i = 0; i < waterVtsX * waterVtsY - waterVtsX; i++) {
		if (i % waterVtsY == (waterVtsY - 1))	continue;
		waterIds.push_back(waterVtsX * waterVtsY + i);
		waterIds.push_back(waterVtsX * waterVtsY + i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i + waterVtsX);

		waterIds.push_back(waterVtsX * waterVtsY + i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i + waterVtsX + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i + waterVtsX);
	}
	// Back
	for (unsigned i = 0; i < waterVtsY - 1; i++) {
		waterIds.push_back(i);
		waterIds.push_back(i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i);

		waterIds.push_back(i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i);
	}
	// Right
	for (unsigned i = waterVtsY - 1; i < waterVtsX * waterVtsY - waterVtsX; i += waterVtsX) {
		waterIds.push_back(i);
		waterIds.push_back(i + waterVtsX);
		waterIds.push_back(waterVtsX * waterVtsY + i);

		waterIds.push_back(i + waterVtsX);
		waterIds.push_back(waterVtsX * waterVtsY + i + waterVtsX);
		waterIds.push_back(waterVtsX * waterVtsY + i);
	}
	// Front
	for (unsigned i = waterVtsX * waterVtsY - waterVtsX; i < waterVtsX * waterVtsY - 1; i++) {
		waterIds.push_back(i);
		waterIds.push_back(waterVtsX * waterVtsY + i);
		waterIds.push_back(i + 1);

		waterIds.push_back(i + 1);
		waterIds.push_back(waterVtsX * waterVtsY + i);
		waterIds.push_back(waterVtsX * waterVtsY + i + 1);
	}
	// Left
	for (unsigned i = 0; i < waterVtsX * waterVtsY - waterVtsX; i += waterVtsX) {
		waterIds.push_back(i);
		waterIds.push_back(waterVtsX * waterVtsY + i);
		waterIds.push_back(i + waterVtsX);

		waterIds.push_back(i + waterVtsX);
		waterIds.push_back(waterVtsX * waterVtsY + i);
		waterIds.push_back(waterVtsX * waterVtsY + i + waterVtsX);
	}

	waterVcount = int(waterIds.size());
	waterSurfVcount = int(waterSurfIds.size());

	// Create vertex array object
	glGenVertexArrays(1, &waterVao);
	glBindVertexArray(waterVao);

	// Create vertex buffer
	glGenBuffers(1, &waterVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, waterVbuf);
	glBufferData(GL_ARRAY_BUFFER, waterVerts.size() * sizeof(vert), waterVerts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	GLubyte* offset = nullptr;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec2);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	// Create index buffer
	glGenBuffers(1, &waterIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIds.size() * sizeof(GLuint), waterIds.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create vertex array object
	glGenVertexArrays(1, &waterSurfVao);
	glBindVertexArray(waterSurfVao);

	// Create vertex buffer
	glGenBuffers(1, &waterSurfVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, waterSurfVbuf);
	glBufferData(GL_ARRAY_BUFFER, waterSurfVerts.size() * sizeof(vert), waterSurfVerts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	offset = nullptr;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec2);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	// Create index buffer
	glGenBuffers(1, &waterSurfIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterSurfIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterSurfIds.size() * sizeof(GLuint), waterSurfIds.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initWallsMesh() {
	wallVerts = {
		{ glm::vec3(-1.001f,  0.3f,   -1.001f), glm::vec2(0.0f, 0.0f), MAT_WALL },
		{ glm::vec3( 1.001f,  0.3f,   -1.001f), glm::vec2(1.0f, 0.0f), MAT_WALL },
		{ glm::vec3(-1.001f, -1.001f, -1.001f), glm::vec2(0.0f, 0.8f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f, -1.001f), glm::vec2(1.0f, 0.8f), MAT_WALL },

		{ glm::vec3( 1.001f,  0.3f,   -1.001f), glm::vec2(0.0f, 0.0f), MAT_WALL },
		{ glm::vec3( 1.001f,  0.3f,    1.001f), glm::vec2(1.0f, 0.0f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f, -1.001f), glm::vec2(0.0f, 0.8f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f,  1.001f), glm::vec2(1.0f, 0.8f), MAT_WALL },

		{ glm::vec3( 1.001f,  0.3f,    1.001f), glm::vec2(0.0f, 0.0f), MAT_WALL },
		{ glm::vec3(-1.001f,  0.3f,    1.001f), glm::vec2(1.0f, 0.0f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f,  1.001f), glm::vec2(0.0f, 0.8f), MAT_WALL },
		{ glm::vec3(-1.001f, -1.001f,  1.001f), glm::vec2(1.0f, 0.8f), MAT_WALL },

		{ glm::vec3(-1.001f,  0.3f,    1.001f), glm::vec2(0.0f, 0.0f), MAT_WALL },
		{ glm::vec3(-1.001f,  0.3f,   -1.001f), glm::vec2(1.0f, 0.0f), MAT_WALL },
		{ glm::vec3(-1.001f, -1.001f,  1.001f), glm::vec2(0.0f, 0.8f), MAT_WALL },
		{ glm::vec3(-1.001f, -1.001f, -1.001f), glm::vec2(1.0f, 0.8f), MAT_WALL },

		{ glm::vec3(-1.001f, -1.001f, -1.001f), glm::vec2(0.0f, 0.0f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f, -1.001f), glm::vec2(1.0f, 0.0f), MAT_WALL },
		{ glm::vec3(-1.001f, -1.001f,  1.001f), glm::vec2(0.0f, 1.0f), MAT_WALL },
		{ glm::vec3( 1.001f, -1.001f,  1.001f), glm::vec2(1.0f, 1.0f), MAT_WALL }
	};
	// Vertex indices for triangles
	wallIds = {
		 0,  2,  1,  1,  2,  3,
		 4,  6,  7,  4,  7,  5,
		 8, 10, 11,  9,  8, 11,
		12, 14, 13, 13, 14, 15,
		16, 18, 19, 16, 19, 17
	};
	wallVcount = int(wallIds.size());

	// Create vertex array object
	glGenVertexArrays(1, &wallVao);
	glBindVertexArray(wallVao);

	// Create vertex buffer
	glGenBuffers(1, &wallVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, wallVbuf);
	glBufferData(GL_ARRAY_BUFFER, wallVerts.size() * sizeof(vert), wallVerts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	GLubyte* offset = nullptr;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec2);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	// Create index buffer
	glGenBuffers(1, &wallIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, wallIds.size() * sizeof(GLuint), wallIds.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initSkybox() {
	skyVerts = {
		{ glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f,  10.0f, -10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec2(0.0f, 0.8f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f, -10.0f), glm::vec2(1.0f, 0.8f), MAT_SKYBOX },

		{ glm::vec3( 10.0f,  10.0f, -10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f,  10.0f,  10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f, -10.0f), glm::vec2(0.0f, 0.8f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f,  10.0f), glm::vec2(1.0f, 0.8f), MAT_SKYBOX },

		{ glm::vec3( 10.0f,  10.0f,  10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f,  10.0f), glm::vec2(0.0f, 0.8f), MAT_SKYBOX },
		{ glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec2(1.0f, 0.8f), MAT_SKYBOX },

		{ glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec2(0.0f, 0.8f), MAT_SKYBOX },
		{ glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec2(1.0f, 0.8f), MAT_SKYBOX },

		{ glm::vec3(-10.0f, -10.0f, -10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f, -10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f, -10.0f,  10.0f), glm::vec2(0.0f, 1.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f, -10.0f,  10.0f), glm::vec2(1.0f, 1.0f), MAT_SKYBOX },

		{ glm::vec3(-10.0f,  10.0f, -10.0f), glm::vec2(0.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f,  10.0f, -10.0f), glm::vec2(1.0f, 0.0f), MAT_SKYBOX },
		{ glm::vec3(-10.0f,  10.0f,  10.0f), glm::vec2(0.0f, 1.0f), MAT_SKYBOX },
		{ glm::vec3( 10.0f,  10.0f,  10.0f), glm::vec2(1.0f, 1.0f), MAT_SKYBOX }
	};
	// Vertex indices for triangles
	skyIds = {
		 0,  2,  1,  1,  2,  3,
		 4,  6,  7,  4,  7,  5,
		 8, 10, 11,  9,  8, 11,
		12, 14, 13, 13, 14, 15,
		16, 18, 19, 16, 19, 17,
		20, 21, 23, 20, 23, 22
	};
	skyVcount = int(skyIds.size());

	// Create vertex array object
	glGenVertexArrays(1, &skyVao);
	glBindVertexArray(skyVao);

	// Create vertex buffer
	glGenBuffers(1, &skyVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, skyVbuf);
	glBufferData(GL_ARRAY_BUFFER, skyVerts.size() * sizeof(vert), skyVerts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	GLubyte* offset = nullptr;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec2);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	// Create index buffer
	glGenBuffers(1, &skyIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, skyIds.size() * sizeof(GLuint), skyIds.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initTerrain() {
	for (unsigned j = 0; j < terrVtsY; j++) {
		for (unsigned i = 0; i < terrVtsX; i++) {
			vert v = {
				glm::vec3((float)i / (float)(terrVtsX - 1) * 2.0f - 1.0f,
				-0.5f, (float)j / (float)(terrVtsY - 1) * 2.0f - 1.0f),
				glm::vec2((float)i / (float)(terrVtsX - 1),
				(float)j / (float)(terrVtsY - 1)), MAT_TERR
			};
			terrVerts.push_back(v);
		}
	}

	// Vertex indices for triangles
	for (unsigned i = 0; i < terrVtsX * terrVtsY - terrVtsX; i++) {
		if (i % terrVtsY == (terrVtsY - 1))	continue;
		terrIds.push_back(i);
		terrIds.push_back(i + terrVtsX);
		terrIds.push_back(i + 1);

		terrIds.push_back(i + 1);
		terrIds.push_back(i + terrVtsX);
		terrIds.push_back(i + terrVtsX + 1);
	}

	terrVcount = int(terrIds.size());

	// Create vertex array object
	glGenVertexArrays(1, &terrVao);
	glBindVertexArray(terrVao);

	// Create vertex buffer
	glGenBuffers(1, &terrVbuf);
	glBindBuffer(GL_ARRAY_BUFFER, terrVbuf);
	glBufferData(GL_ARRAY_BUFFER, terrVerts.size() * sizeof(vert), terrVerts.data(), GL_DYNAMIC_DRAW);
	// Specify vertex attributes
	GLubyte* offset = nullptr;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	offset += sizeof(glm::vec2);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vert), offset);
	// Create index buffer
	glGenBuffers(1, &terrIbuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrIbuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrIds.size() * sizeof(GLuint), terrIds.data(), GL_DYNAMIC_DRAW);

	// Cleanup state
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initTextures() {
	// Create texture data
	initTexData = vector<glm::u8vec3>(texWidth * texHeight, glm::u8vec3(0, 0, 0));
	islandsTexData = vector<glm::u8vec3>(texWidth * texHeight, glm::u8vec3(255, 255, 255));
	refractionTexData = vector<glm::u8vec3>(texWidth * texHeight, glm::u8vec3(0, 0, 0));
	reflectionTexData = vector<glm::u8vec3>(texWidth * texHeight, glm::u8vec3(0, 0, 0));
	envMapData = vector<glm::u8vec4>(texWidth * texHeight, glm::u8vec4(0, 0, 0, 0));
	causticsMapData = vector<glm::u8vec4>(texWidth * texHeight, glm::u8vec4(0, 0, 0, 0));

	// Create texture objects
	glGenTextures(1, &prevTexture);
	glBindTexture(GL_TEXTURE_2D, prevTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, texWidth, texHeight, 0, GL_RGB, GL_BYTE, initTexData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &currTexture);
	glBindTexture(GL_TEXTURE_2D, currTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, texWidth, texHeight, 0, GL_RGB, GL_BYTE, initTexData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &refractionTexture);
	glBindTexture(GL_TEXTURE_2D, refractionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, refractionTexData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &reflectionTexture);
	glBindTexture(GL_TEXTURE_2D, reflectionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, reflectionTexData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &environmentMap);
	glBindTexture(GL_TEXTURE_2D, environmentMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, texWidth, texHeight, 0, GL_RGBA, GL_BYTE, envMapData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &causticsMap);
	glBindTexture(GL_TEXTURE_2D, causticsMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM, texWidth, texHeight, 0, GL_RGBA, GL_BYTE, causticsMapData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	vector<std::string> faces{
		"textures/skyboxRight.png",  "textures/skyboxLeft.png",  "textures/skyboxTop.png",
		"textures/skyboxBottom.png", "textures/skyboxFront.png", "textures/skyboxBack.png"
	};
	skyboxTexture = loadSkybox(faces);

	glBindTexture(GL_TEXTURE_2D, 0);

	initWallTexture();
	initTerrTexture();
	generateIslands();

	// Create framebuffer object (draw to currTexture)
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currTexture, 0);

	assert(glGetError() == GL_NO_ERROR);
}

void initWallTexture() {
	int wallTexWidth, wallTexHeight;

	// Load texture
	int n_ch;
	wallTexData = stbi_load("textures/Mosaic-C.png", &wallTexWidth, &wallTexHeight, &n_ch, 3);
	if (!wallTexData)
		throw std::runtime_error("failed to load image");

	// Create texture object
	glGenTextures(1, &wallTexture);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wallTexWidth, wallTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, wallTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void initTerrTexture() {
	int terrTexWidth, terrTexHeight;

	// Load texture
	int n_ch;
	terrTexData = stbi_load("textures/sand.png", &terrTexWidth, &terrTexHeight, &n_ch, 3);
	if (!terrTexData)
		throw std::runtime_error("failed to load image");

	// Create texture object
	glGenTextures(1, &terrTexture);
	glBindTexture(GL_TEXTURE_2D, terrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, terrTexWidth, terrTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, terrTexData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void display() {
	try {
		// Pass 1: GPGPU output to texture =============================

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);		// Enable render-to-texture
		glViewport(0, 0, texWidth, texHeight);		// Reshape to texture size
		glUseProgram(gpgpuShader);

		// Mouse position for interaction
		// I have no idea about ray-casting mouse interaction
		// So I would just keep the mouse to texture coordinate interaction
		glUniform2fv(uniMousePos, 1, value_ptr(mousePos));

		// Use the previous texture output as input
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, prevTexture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, currTexture);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, islandsTexture);
		// Draw the quad to invoke the shader
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, vcount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);

		// Pass 1.1: Environment Mapping =============================

		// Load model on demand
		if (!mesh) mesh = new Mesh("models/bunny.obj");

		glDisable(GL_BLEND);
		glUseProgram(envShader);
		
		glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 100.0f);
		glm::mat4 view = glm::lookAt(lightPos, glm::vec3(0.0f),glm::vec3(0.0f, 0.1f, 0.0f));
		glm::mat4 xform = proj * view;
		glm::mat4 lightViewXform = xform;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, environmentMap, 0);

		// Send transformation matrix to shader
		glUniformMatrix4fv(uniEnvXform, 1, GL_FALSE, value_ptr(lightViewXform));

		// Clear the texture
		glClear(GL_COLOR_BUFFER_BIT);
		// Enable terrain texture
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, islandsTexture);
		// Draw the scene (double-sided walls and double-sided terrain)
		glDisable(GL_CULL_FACE);
		mesh->move(0.0f, -0.5f, 0.0f);
		mesh->draw();
		glBindVertexArray(wallVao);
		glDrawElements(GL_TRIANGLES, wallVcount, GL_UNSIGNED_INT, NULL);
		if (enableTerrain) {
			glBindVertexArray(terrVao);
			glDrawElements(GL_TRIANGLES, terrVcount, GL_UNSIGNED_INT, NULL);
		}
		glEnable(GL_CULL_FACE);

		// Pass 1.2: Caustics Mapping =============================

		glUseProgram(causticsShader);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, causticsMap, 0);

		glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f) - lightPos);
		glUniform3fv(uniLightDir, 1, value_ptr(lightDir));

		// Send transformation matrix to shader
		glUniformMatrix4fv(uniCausticsXform, 1, GL_FALSE, value_ptr(lightViewXform));

		// Clear the texture
		glClear(GL_COLOR_BUFFER_BIT);
		// Enable terrain texture
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, prevTexture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, environmentMap);
		// Draw the scene (water)
		glBindVertexArray(waterSurfVao);
		glDrawElements(GL_TRIANGLES, waterSurfVcount, GL_UNSIGNED_INT, NULL);

		glEnable(GL_BLEND);

		// Pass 2: Refraction ===============================

		glUseProgram(dispShader);

		float aspect = (float)width / (float)height;
		// Create perspective projection matrix
		proj = glm::perspective(45.0f, aspect, 0.1f, 100.0f);
		// Create view transformation matrix
		view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -camCoords.z));
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(camCoords.y), glm::vec3(1.0, 0.0, 0.0));
		rot = glm::rotate(rot, glm::radians(camCoords.x), glm::vec3(0.0, 1.0, 0.0));
		xform = proj * view * rot;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractionTexture, 0);

		// Send transformation matrix to shader
		glUniformMatrix4fv(uniXform, 1, GL_FALSE, value_ptr(xform));

		// Clear the texture
		glClear(GL_COLOR_BUFFER_BIT);
		// Draw the textures
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, islandsTexture);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		glActiveTexture(GL_TEXTURE0 + 3);
		glBindTexture(GL_TEXTURE_2D, terrTexture);
		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		glActiveTexture(GL_TEXTURE0 + 7);
		glBindTexture(GL_TEXTURE_2D, causticsMap);
		// Draw the scene (skybox, walls and terrain)
		glBindVertexArray(skyVao);
		glDrawElements(GL_TRIANGLES, skyVcount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(wallVao);
		glDrawElements(GL_TRIANGLES, wallVcount, GL_UNSIGNED_INT, NULL);
		if (enableTerrain) {
			glBindVertexArray(terrVao);
			glDrawElements(GL_TRIANGLES, terrVcount, GL_UNSIGNED_INT, NULL);
		}

		// Pass 3: Reflection ===============================

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

		// Flip the camera
		rot = glm::rotate(glm::mat4(1.0f), -glm::radians(camCoords.y), glm::vec3(1.0, 0.0, 0.0));
		rot = glm::rotate(rot, glm::radians(camCoords.x), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 xformRflect = proj * view * rot;

		// Send transformation matrix to shader
		glUniformMatrix4fv(uniXform, 1, GL_FALSE, value_ptr(xformRflect));

		// Clip water surface
		glm::vec4 clipPlaneReflect(0.0f, 1.0f, 0.0f, 0.0f);
		glUniform4fv(uniClipPlane, 1, value_ptr(clipPlaneReflect));

		// Clear the texture
		glClear(GL_COLOR_BUFFER_BIT);
		// Draw the scene (skybox, walls and double-sided clipped terrain above water surface)
		glBindVertexArray(skyVao);
		glDrawElements(GL_TRIANGLES, skyVcount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(wallVao);
		glDrawElements(GL_TRIANGLES, wallVcount, GL_UNSIGNED_INT, NULL);
		if (enableTerrain) {
			glDisable(GL_CULL_FACE);
			glEnable(GL_CLIP_DISTANCE0);
			glBindVertexArray(terrVao);
			glDrawElements(GL_TRIANGLES, terrVcount, GL_UNSIGNED_INT, NULL);
			glDisable(GL_CLIP_DISTANCE0);
			glEnable(GL_CULL_FACE);
		}

		// Swap prev and curr textures
		std::swap(prevTexture, currTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currTexture, 0);

		// Pass 4: Display ===============================

		glBindFramebuffer(GL_FRAMEBUFFER, 0);		// Restore default framebuffer (draw to window)
		glViewport(0, 0, width, height);			// Reshape to window size

		// Send transformation matrix to shader
		glUniformMatrix4fv(uniXform, 1, GL_FALSE, value_ptr(xform));
		glUniformMatrix4fv(uniLightViewXform, 1, GL_FALSE, value_ptr(lightViewXform));

		// Camera position for water fresnel calculation
		glUniformMatrix4fv(uniCamPos, 1, GL_FALSE, value_ptr(camCoords));

		glUniform3fv(uniLightDirDisp, 1, value_ptr(lightDir));

		// Clear the window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Draw the textures
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, prevTexture);
		glActiveTexture(GL_TEXTURE0 + 4);
		glBindTexture(GL_TEXTURE_2D, refractionTexture);
		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, reflectionTexture);
		// Draw the scene (walls, double-sided terrain and double-sided water)
		mesh->move(0.0f, -0.5f, 0.0f);
		mesh->draw();
		glBindVertexArray(wallVao);
		glDrawElements(GL_TRIANGLES, wallVcount, GL_UNSIGNED_INT, NULL);
		glDisable(GL_CULL_FACE);
		if (enableTerrain) {
			glBindVertexArray(terrVao);
			glDrawElements(GL_TRIANGLES, terrVcount, GL_UNSIGNED_INT, NULL);
		}
		glBindVertexArray(waterVao);
		glDrawElements(GL_TRIANGLES, waterVcount, GL_UNSIGNED_INT, NULL);
		glEnable(GL_CULL_FACE);

		/*/ DEBUG PASS =============================
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glUseProgram(debugShader);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, environmentMap);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, vcount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);*/

		// Revert state
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		// Display the back buffer
		glutSwapBuffers();

	} catch (const exception& e) {
		cerr << "Fatal error: " << e.what() << endl;
		glutLeaveMainLoop();
	}
}

void reshape(GLint width, GLint height) {
	::width = width;
	::height = height;
	glViewport(0, 0, width, height);
}

void keyRelease(unsigned char key, int x, int y) {
	switch (key) {
	case 27:	// Escape key
		menu(MENU_EXIT);
		break;
	}
}

// Convert a position in screen space into texture space
glm::ivec2 mouseToTexCoord(int x, int y) {
	glm::vec3 mousePos(x, y, 1.0f);

	// Convert screen coordinates into clip space
	glm::mat3 screenToClip(1.0f);
	screenToClip[0][0] = 2.0f / width;
	screenToClip[1][1] = -2.0f / height;	// Flip y coordinate
	screenToClip[2][0] = -1.0f;
	screenToClip[2][1] = 1.0f;

	// Invert the aspect ratio correction (from display())
	float winAspect = (float)width / (float)height;
	float texAspect = (float)texWidth / (float)texHeight;
	glm::mat3 invAspect(1.0f);
	invAspect[0][0] = glm::max(1.0f, winAspect / texAspect);
	invAspect[1][1] = glm::max(1.0f, texAspect / winAspect);

	// Convert to texture coordinates
	glm::mat3 quadToTex(1.0f);
	quadToTex[0][0] = texWidth / 2.0f;
	quadToTex[1][1] = texHeight / 2.0f;
	quadToTex[2][0] = texWidth / 2.0f;
	quadToTex[2][1] = texHeight / 2.0f;

	// Get texture coordinate that was clicked on
	glm::ivec2 texPos = glm::ivec2(glm::floor(quadToTex * invAspect * screenToClip * mousePos));
	return texPos;
}

void mouseBtn(int button, int state, int x, int y) {
	if (button == GLUT_LEFT && state == GLUT_DOWN) {
		glm::ivec2 iPos = mouseToTexCoord(x, y);
		mousePos.x = (float)iPos.x / (float)texWidth;
		mousePos.y = (float)iPos.y / (float)texWidth;

		// Activate rotation mode
		camRot = true;
		camOrigin = glm::vec2(camCoords);
		mouseOrigin = glm::vec2(x, y);
	}
	if (button == GLUT_LEFT && state == GLUT_UP) {
		mousePos.x = -2.0f;
		mousePos.y = -2.0f;

		// Deactivate rotation
		camRot = false;
	}
	if (button == 3)
		camCoords.z = glm::clamp(camCoords.z - 0.1f, 0.1f, 10.0f);
	if (button == 4)
		camCoords.z = glm::clamp(camCoords.z + 0.1f, 0.1f, 10.0f);
}

void mouseMove(int x, int y) {
	glm::ivec2 iPos = mouseToTexCoord(x, y);
	mousePos.x = (float)iPos.x / (float)texWidth;
	mousePos.y = (float)iPos.y / (float)texWidth;

	if (camRot) {
		// Convert mouse delta into degrees, add to rotation
		float rotScale = glm::min(width / 450.0f, height / 270.0f);
		glm::vec2 mouseDelta = glm::vec2(x, y) - mouseOrigin;
		glm::vec2 newAngle = camOrigin + mouseDelta / rotScale;
		newAngle.y = glm::clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.y < -180.0f) newAngle.y += 360.0f;
		if (glm::length(newAngle - glm::vec2(camCoords)) > FLT_EPSILON) {
			camCoords.x = newAngle.x;
			camCoords.y = newAngle.y;
		}
	}
}

void idle() {
	glutPostRedisplay();
	// I've tested the program on different devices.
	// Some devices run the program slow and have almost no water motions.
	// The GPGPU calculation of wave equation is heavily depend on frames per second.
	// To solve this problem I have to manually slow down the rendering a little bit
	// to unify the performance on different devices.
	Sleep(10);

	// P.S: I've got this program run on my ancient Macbook with CoreDuo intergrated GPU,
	// and there was compeletely no mouse interaction or any kind of water motions,
	// and also there was strange alpha flickering on the walls.
	// I've also tested it on a VMWare virtual machine, the water motion worked fine,
	// but I have got no refractions but just reflections on the water.
	// Very strange effects on different devices which I have completely no idea why.
}

void menu(int cmd) {
	switch (cmd) {
	case MENU_EXIT:
		glutLeaveMainLoop();
		break;

	case MENU_TERR:
		if (enableTerrain) enableTerrain = false;
		else enableTerrain = true;
		generateIslands();
		break;
	case MENU_RESEED:
		generateIslands();
		break;
	}
}

void cleanup() {
	// Release all resources
	if (prevTexture) { glDeleteTextures(1, &prevTexture); prevTexture = 0; }
	if (currTexture) { glDeleteTextures(1, &currTexture); currTexture = 0; }
	if (islandsTexture) { glDeleteTextures(1, &islandsTexture); islandsTexture = 0; }
	if (wallTexture) { glDeleteTextures(1, &wallTexture); wallTexture = 0; }
	if (terrTexture) { glDeleteTextures(1, &terrTexture); terrTexture = 0; }
	if (refractionTexture) { glDeleteTextures(1, &refractionTexture); refractionTexture = 0; }
	if (reflectionTexture) { glDeleteTextures(1, &reflectionTexture); reflectionTexture = 0; }
	if (skyboxTexture) { glDeleteTextures(1, &skyboxTexture); skyboxTexture = 0; }

	if (dispShader) { glDeleteProgram(dispShader); dispShader = 0; }
	if (gpgpuShader) { glDeleteProgram(gpgpuShader); gpgpuShader = 0; }
	if (envShader) { glDeleteProgram(envShader); envShader = 0; }
	if (causticsShader) { glDeleteProgram(causticsShader); causticsShader = 0; }
	if (debugShader) { glDeleteProgram(debugShader); debugShader = 0; }

	uniXform = 0;
	uniClipPlane = 0;
	uniMousePos = 0;
	uniCamPos = 0;
	uniEnvXform = 0;
	uniCausticsXform = 0;
	uniLightDir = 0;
	uniLightViewXform = 0;
	uniLightDirDisp = 0;

	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	if (ibuf) { glDeleteBuffers(1, &ibuf); ibuf = 0; }
	vcount = 0;
	if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }

	if (waterVao) { glDeleteVertexArrays(1, &waterVao); waterVao = 0; }
	if (waterVbuf) { glDeleteBuffers(1, &waterVbuf); waterVbuf = 0; }
	if (waterIbuf) { glDeleteBuffers(1, &waterIbuf); waterIbuf = 0; }
	waterVcount = 0;

	if (waterSurfVao) { glDeleteVertexArrays(1, &waterSurfVao); waterSurfVao = 0; }
	if (waterSurfVbuf) { glDeleteBuffers(1, &waterSurfVbuf); waterSurfVbuf = 0; }
	if (waterSurfIbuf) { glDeleteBuffers(1, &waterSurfIbuf); waterSurfIbuf = 0; }
	waterSurfVcount = 0;

	if (wallVao) { glDeleteVertexArrays(1, &wallVao); wallVao = 0; }
	if (wallVbuf) { glDeleteBuffers(1, &wallVbuf); wallVbuf = 0; }
	if (wallIbuf) { glDeleteBuffers(1, &wallIbuf); wallIbuf = 0; }
	wallVcount = 0;

	if (skyVao) { glDeleteVertexArrays(1, &skyVao); skyVao = 0; }
	if (skyVbuf) { glDeleteBuffers(1, &skyVbuf); skyVbuf = 0; }
	if (skyIbuf) { glDeleteBuffers(1, &skyIbuf); skyIbuf = 0; }
	skyVcount = 0;

	if (terrVao) { glDeleteVertexArrays(1, &terrVao); terrVao = 0; }
	if (terrVbuf) { glDeleteBuffers(1, &terrVbuf); terrVbuf = 0; }
	if (terrIbuf) { glDeleteBuffers(1, &terrIbuf); terrIbuf = 0; }
	terrVcount = 0;

	if (wallTexData) { stbi_image_free(wallTexData); wallTexData = NULL; }

	if (mesh) { delete mesh; mesh = NULL; }
}

void generateIslands() {
	perlin.reseed(noise(rng));

	islandsTexData.clear();
	islandsTexData.resize(texWidth * texHeight, glm::u8vec3(255, 255, 255));
	if (islandsTexture) { glDeleteTextures(1, &islandsTexture); islandsTexture = 0; }

	if (enableTerrain) {	// Perlin noise generation
		for (int j = 0; j < texHeight; j++) {
			for (int i = 0; i < texWidth; i++) {
				float x = (float)i / (float)texWidth * 2.0f - 1.0f;
				float y = (float)j / (float)texHeight * 2.0f - 1.0f;
				auto value = (glm::u8)(perlin.octaveNoise0_1(x, y, 2) * 255.);
				islandsTexData[j * texHeight + i] = glm::u8vec3(value);
			}
		}
	}

	glGenTextures(1, &islandsTexture);
	glBindTexture(GL_TEXTURE_2D, islandsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, islandsTexData.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint loadSkybox(vector<std::string> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 3);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
