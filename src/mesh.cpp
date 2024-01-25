#include "mesh.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

// Helper functions
int indexOfNumberLetter(std::string& str, int offset);
int lastIndexOfNumberLetter(std::string& str);
std::vector<std::string> split(const std::string& s, char delim);

// Constructor - load mesh from file
Mesh::Mesh(std::string filename) {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	currentOffset.x = 0.0f;
	currentOffset.y = 0.0f;
	currentOffset.z = 0.0f;
	currentRotation.x = 0.0f;
	currentRotation.y = 0.0f;
	currentRotation.z = 0.0f;

	vao = 0;
	vbuf = 0;
	vcount = 0;
	load(filename);
}

// Draw the mesh
void Mesh::draw() {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vcount);
	glBindVertexArray(0);
}

// Load a wavefront OBJ file
void Mesh::load(std::string filename) {
	// Release resources
	release();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Mesh::load() - Could not open file " << filename;
		throw std::runtime_error(ss.str());
	}

	// Store vertex and normal data while reading
	//std::vector<glm::vec3> raw_vertices;
	//std::vector<glm::vec3> raw_normals;
	//std::vector<unsigned int> v_elements;
	//std::vector<unsigned int> n_elements;
	raw_vertices.clear();
	raw_normals.clear();
	m_vertices.clear();
	m_normals.clear();
	v_elements.clear();
	n_elements.clear();

	std::string line;
	while (getline(file, line)) {
		if (line.substr(0, 2) == "v ") {
			// Read position data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			glm::vec3 vert(stof(values[0]), stof(values[1]), stof(values[2]));
			raw_vertices.push_back(vert);

			// Update bounding box
			minBB = glm::min(minBB, vert);
			maxBB = glm::max(maxBB, vert);
		}
		else if (line.substr(0, 3) == "vn ") {
			// Read normal data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			raw_normals.push_back(glm::vec3(stof(values[0]), stof(values[1]), stof(values[2])));

		}
		else if (line.substr(0, 2) == "f ") {
			// Read face data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			for (int i = 0; i < values.size() - 2; i++) {
				// Split up vertex indices
				std::vector<std::string> v1 = split(values[0], '/');		// Triangle fan for ngons
				std::vector<std::string> v2 = split(values[i + 1], '/');
				std::vector<std::string> v3 = split(values[i + 2], '/');

				// Store position indices
				v_elements.push_back(stoul(v1[0]) - 1);
				v_elements.push_back(stoul(v2[0]) - 1);
				v_elements.push_back(stoul(v3[0]) - 1);

				// Check for normals
				if (v1.size() >= 3 && v1[2].length() > 0) {
					n_elements.push_back(stoul(v1[2]) - 1);
					n_elements.push_back(stoul(v2[2]) - 1);
					n_elements.push_back(stoul(v3[2]) - 1);
				}
			}
		}
	}
	file.close();

	// Create vertex array
	std::vector<Vtx> vertices(v_elements.size());
	for (int i = 0; i < v_elements.size(); i += 3) {
		// Store positions
		vertices[i + 0].pos = raw_vertices[v_elements[i + 0]];
		vertices[i + 1].pos = raw_vertices[v_elements[i + 1]];
		vertices[i + 2].pos = raw_vertices[v_elements[i + 2]];

		// Check for normals
		if (n_elements.size() > 0) {
			// Store normals
			vertices[i + 0].norm = raw_normals[n_elements[i + 0]];
			vertices[i + 1].norm = raw_normals[n_elements[i + 1]];
			vertices[i + 2].norm = raw_normals[n_elements[i + 2]];
		}
		else {
			// Calculate normal
			glm::vec3 normal = normalize(cross(vertices[i + 1].pos - vertices[i + 0].pos,
				vertices[i + 2].pos - vertices[i + 0].pos));
			vertices[i + 0].norm = normal;
			vertices[i + 1].norm = normal;
			vertices[i + 2].norm = normal;
		}
	}
	vcount = int(vertices.size());

	m_vertices = raw_vertices;
	m_normals = raw_normals;

	// Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vtx), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), (GLvoid*)sizeof(glm::vec3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::move(const float& offsetX, const float& offsetY, const float& offsetZ)
{
	glm::vec3 offset(offsetX, offsetY, offsetZ);
	currentOffset = offset;

	float roll = currentRotation.x;
	float pitch = currentRotation.y;
	float yaw = currentRotation.z;
	glm::mat3 rx(1.0f, 0.0f, 0.0f, 0.0f, cos(roll), -sin(roll), 0.0f, sin(roll), cos(roll));
	glm::mat3 ry(cos(pitch), 0.0f, sin(pitch), 0.0f, 1.0f, 0.0f, -sin(pitch), 0.0f, cos(pitch));
	glm::mat3 rz(cos(yaw), -sin(yaw), 0.0f, sin(yaw), cos(yaw), 0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < raw_vertices.size(); i++) {
		m_vertices[i] = rx * ry * rz * raw_vertices[i];
		m_vertices[i] = raw_vertices[i] + offset;
	}

	// Create vertex array
	std::vector<Vtx> vertices(v_elements.size());
	for (int i = 0; i < v_elements.size(); i += 3) {
		// Store positions
		vertices[i + 0].pos = m_vertices[v_elements[i + 0]];
		vertices[i + 1].pos = m_vertices[v_elements[i + 1]];
		vertices[i + 2].pos = m_vertices[v_elements[i + 2]];

		// Check for normals
		if (n_elements.size() > 0) {
			// Store normals
			vertices[i + 0].norm = m_normals[n_elements[i + 0]];
			vertices[i + 1].norm = m_normals[n_elements[i + 1]];
			vertices[i + 2].norm = m_normals[n_elements[i + 2]];
		}
		else {
			// Calculate normal
			glm::vec3 normal = normalize(cross(vertices[i + 1].pos - vertices[i + 0].pos,
				vertices[i + 2].pos - vertices[i + 0].pos));
			vertices[i + 0].norm = normal;
			vertices[i + 1].norm = normal;
			vertices[i + 2].norm = normal;
		}
	}
	vcount = int(vertices.size());

	// Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vtx), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), (GLvoid*)sizeof(glm::vec3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::rotate(const float& roll, const float& pitch, const float& yaw) {
	glm::vec3 rotation(roll, pitch, yaw);
	currentRotation = rotation;

	glm::mat3 rx(1.0f, 0.0f, 0.0f, 0.0f, cos(roll), -sin(roll), 0.0f, sin(roll), cos(roll));
	glm::mat3 ry(cos(pitch), 0.0f, sin(pitch), 0.0f, 1.0f, 0.0f, -sin(pitch), 0.0f, cos(pitch));
	glm::mat3 rz(cos(yaw), -sin(yaw), 0.0f, sin(yaw), cos(yaw), 0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < raw_vertices.size(); i++) {
		m_vertices[i] = rx * ry * rz * raw_vertices[i];
		m_vertices[i] += currentOffset;
	}

	// Create vertex array
	std::vector<Vtx> vertices(v_elements.size());
	for (int i = 0; i < v_elements.size(); i += 3) {
		// Store positions
		vertices[i + 0].pos = m_vertices[v_elements[i + 0]];
		vertices[i + 1].pos = m_vertices[v_elements[i + 1]];
		vertices[i + 2].pos = m_vertices[v_elements[i + 2]];

		// Check for normals
		if (n_elements.size() > 0) {
			// Store normals
			vertices[i + 0].norm = m_normals[n_elements[i + 0]];
			vertices[i + 1].norm = m_normals[n_elements[i + 1]];
			vertices[i + 2].norm = m_normals[n_elements[i + 2]];
		}
		else {
			// Calculate normal
			glm::vec3 normal = normalize(cross(vertices[i + 1].pos - vertices[i + 0].pos,
				vertices[i + 2].pos - vertices[i + 0].pos));
			vertices[i + 0].norm = normal;
			vertices[i + 1].norm = normal;
			vertices[i + 2].norm = normal;
		}
	}
	vcount = int(vertices.size());

	// Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vtx), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), (GLvoid*)sizeof(glm::vec3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::getVerticesPos(std::vector<glm::vec3>& verticesOut, std::vector<unsigned int>& elementsOut) {
	verticesOut.clear();
	verticesOut = m_vertices;
	elementsOut.clear();
	elementsOut = v_elements;
}

void Mesh::getVerticesNorm(std::vector<glm::vec3>& normalOut, std::vector<unsigned int>& elementsOut) {
	normalOut.clear();
	normalOut = m_normals;
	elementsOut.clear();
	elementsOut = n_elements;
}

// Release resources
void Mesh::release() {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	vcount = 0;

	raw_vertices.clear();
	raw_normals.clear();
	m_vertices.clear();
	m_normals.clear();
	v_elements.clear();
	n_elements.clear();
}

int indexOfNumberLetter(std::string& str, int offset) {
	for (int i = offset; i < str.length(); ++i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return int(str.length());
}
int lastIndexOfNumberLetter(std::string& str) {
	for (int i = int(str.length()) - 1; i >= 0; --i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return 0;
}
std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;

	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}

	return elems;
}