#pragma once
#ifndef LOAD_WITH_ASSIMP_
#define LOAD_WITH_ASSIMP_
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

typedef unsigned int _L_U_INT;

class A_model;
class Mesh;

struct _L_Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct _L_Texture {
	_L_U_INT id;
	string type;
	aiString path;
};

class Mesh {
	friend class A_model;
private:
	vector<_L_Vertex>	vertices;
	vector<_L_Texture>	textures;
	vector<_L_U_INT>	indices;

	_L_U_INT VAO, VBO, EBO;

	void setupMesh();
public:
	Mesh() {}
	Mesh::Mesh(vector<_L_Vertex> _ve,
		vector<_L_Texture>_te,
		vector<_L_U_INT>_in){
		this->vertices = _ve;
		this->textures = _te;
		this->indices = _in;

		setupMesh();
	}
	~Mesh() {
		this->indices.clear();
		this->textures.clear();
		this->vertices.clear();
	}

};

struct S3DColorName {
	int r, g, b, a;
	char name[512];
};

class A_model {
private:
	vector<Mesh> meshes;
	vector<_L_Texture>_loaded;
	string directory;

	void processNode(aiNode*, const aiScene*);
	Mesh processMesh(aiMesh*, const aiScene*);
	vector<_L_Texture> loadMaterialTextures(aiMaterial*, aiTextureType, string);
	_L_U_INT TextureFromFile(const char*, string);
	void CheckMaxAndMin(glm::vec3 newVec3);
	glm::vec3 v3Max, v3Min;

public:
	int nMeshNum = 0;
	GLuint * mVAOs;
	GLuint * mElementCount;
	vector<glm::vec3> vMax, vMin, vCenter;

	static void CheckMaxAndMin(glm::vec3 newVec3,glm::vec3 &dest_min,  glm::vec3 &dest_max);
	static vector<string> get_obj_from_path64(string);
	static map<int, S3DColorName> LoadColor_s(const char*);
	void load_model(string);

	A_model() {}
	A_model(const char* _path) {
		load_model(_path);
	}
	~A_model() {
		delete[] mVAOs;
		delete[] mElementCount;
		vMax.clear();
		vMin.clear();
		vCenter.clear();
		meshes.clear();
		_loaded.clear();
	}
};

#endif // !LOAD_WITH_ASSIMP_