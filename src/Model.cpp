#include "Model.h"

Model::Model(std::string path)
{
    LoadMesh(path);
}

void Model::LoadMesh(const std::string path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path,aiProcess_Triangulate|aiProcess_GenNormals);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    aiNode* node = scene->mRootNode;
    ProcessNode(node, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->mesh.push_back(ProcessMesh(mesh, scene));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<vertex> verts;
    std::vector<unsigned int> index;
    std::vector<texture> textures;

    //顶点数据汇入
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertex vert;
        vert.positionOS.x = mesh->mVertices[i].x;
        vert.positionOS.y = mesh->mVertices[i].y;
        vert.positionOS.z = mesh->mVertices[i].z;

        vert.normalOS.x = mesh->mNormals[i].x;
        vert.normalOS.y = mesh->mNormals[i].y;
        vert.normalOS.z = mesh->mNormals[i].z;

        //在assimp中，最多可以储存八组uv坐标，我们这边只取第一组
        vec2_t uv = vec2_t(0,0);
        if(mesh->mTextureCoords[0])
        {
            uv.x = mesh->mTextureCoords[0]->x;
            uv.y = mesh->mTextureCoords[0]->y;
        }
        vert.texcoord = uv;

        verts.push_back(vert);
    }

    //面数据汇入
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            index.push_back(face.mIndices[j]);
        }
    }

    //贴图材质数据汇入

    //返回
    return Mesh(verts, index, textures);
}



