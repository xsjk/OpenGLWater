#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"

class Mesh {
public:
	Mesh(std::string filename);
	~Mesh() { release(); }

	// Return the bounding box of this object
	std::pair<glm::vec3, glm::vec3> boundingBox() const
	{
		return std::make_pair(minBB, maxBB);
	}

	void load(std::string filename);
	void draw();

	void move(const float&, const float&, const float&);
	void rotate(const float&, const float&, const float&);

	// Mesh vertex format
	struct Vtx {
		glm::vec3 pos;		// Position
		glm::vec3 norm;		// Normal
	};

	void getVerticesPos(std::vector<glm::vec3>&, std::vector<unsigned int>&);
	void getVerticesNorm(std::vector<glm::vec3>&, std::vector<unsigned int>&);

protected:
	void release();		// Release OpenGL resources

	// Bounding box
	glm::vec3 minBB;
	glm::vec3 maxBB;

	// OpenGL resources
	GLuint vao;		// Vertex array object
	GLuint vbuf;	// Vertex buffer
	GLsizei vcount;	// Number of vertices

	glm::vec3 currentOffset;
	glm::vec3 currentRotation;
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec3> raw_normals;
	std::vector<glm::vec3> m_vertices;
	std::vector<glm::vec3> m_normals;
	std::vector<unsigned int> v_elements;
	std::vector<unsigned int> n_elements;

private:
	// Disallow copy and move
	Mesh(const Mesh& other);
	Mesh(Mesh&& other);
	Mesh& operator=(const Mesh& other);
	Mesh& operator=(Mesh&& other);
};

#endif