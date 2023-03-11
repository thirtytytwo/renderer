#ifndef __C_MODEL_H__
#define __C_MODEL_H__

#include "LycMath.h"
#include <iostream>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
struct vertex
{
    vec3_t positionOS;
    vec3_t normalOS;
    vec2_t texcoord;
};

struct texture
{
    unsigned int id;
    std::string type;
    aiString path;
};

class Mesh
{
public:
    //网格数据
    std::vector<vertex> _vertex;
    std::vector<unsigned int> _index;

    //贴图，材质数据
    std::vector<texture> _textures;
    Mesh(std::vector<vertex> &verts, std::vector<unsigned int> &index, std::vector<texture> &texs) : _vertex(verts), _index(index),_textures(texs){};
};

class Model
{
private:
    //网格和子网格
    std::vector<Mesh> mesh;
    
    void LoadMesh(const std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
public:
    Model(std::string path);
    ~Model();

    int Get_Mesh_Size();
    Mesh Get_Mesh(int index);
};
#endif