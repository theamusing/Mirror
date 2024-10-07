#ifndef REFLECTPLANE_H
#define REFLECTPLANE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <opengl/model.hpp>
#include <opengl/shader.hpp>
#include <opengl/camera.hpp>
#include <opengl/light.hpp>
#include <opengl/screenQuad.hpp>

#define MASK_VERTEX_SHADER_PATH "../resources/shaders/mirror_mask.vs"
#define MASK_FRAGMENT_SHADER_PATH "../resources/shaders/mirror_mask.fs"
#define REFLECT_VERTEX_SHADER_PATH "../resources/shaders/mirror_reflect.vs"
#define REFLECT_GEOMETRY_SHADER_PATH "../resources/shaders/mirror_reflect.gs"
#define REFLECT_FRAGMENT_SHADER_PATH "../resources/shaders/mirror_reflect.fs"

#define MASK_RESOLUTION_X 800
#define MASK_RESOLUTION_Y 600
#define REFLECT_RESOLUTION_X 800
#define REFLECT_RESOLUTION_Y 600
#define MAX_REFLECT_PLANE 10
#define PI 3.14159265359

class ReflectPlane
{
public:
    Model model;

    ReflectPlane(string const &path, glm::vec3 normal, bool flip = false) : model(path, flip), baseNormal(glm::normalize(normal)) {}
    ReflectPlane(string const &path, bool flip = false) : model(path, flip)
    {
        // calculate normal
        if (model.meshes.size() == 0 || model.meshes[0].vertices.size() == 0)
        {
            std::cout << "ERROR::REFLECTPLANE::NO_VERTICES" << std::endl;
            return;
        }
        baseNormal = glm::normalize(model.meshes[0].vertices[0].Normal);
    }

    glm::vec3 getNormal()
    {
        glm::mat4 modelMatrix = model.getModelMatrix();
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        return glm::normalize(normalMatrix * baseNormal);
    }

    void Draw(Shader &shader)
    {
        model.Draw(shader);
    }

private:
    glm::vec3 baseNormal;
};

class ReflectPlaneManager
{
public:
    ReflectPlaneManager() : maskShader(Shader(MASK_VERTEX_SHADER_PATH, MASK_FRAGMENT_SHADER_PATH)),
                            reflectShader(Shader(REFLECT_VERTEX_SHADER_PATH, REFLECT_FRAGMENT_SHADER_PATH, REFLECT_GEOMETRY_SHADER_PATH)),
                            // reflectShader(Shader("../resources/shaders/model_lighting.vs", "../resources/shaders/model_lighting.fs")),
                            debugShader(Shader("../resources/shaders/screen_quad.vs", "../resources/shaders/screen_quad.fs"))
    {
        // init framebuffer
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glGenTextures(1, &texMask);
        glBindTexture(GL_TEXTURE_2D, texMask);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, MASK_RESOLUTION_X, MASK_RESOLUTION_Y, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenTextures(1, &texReflect);
        glBindTexture(GL_TEXTURE_2D, texReflect);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFLECT_RESOLUTION_X, REFLECT_RESOLUTION_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLuint rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, REFLECT_RESOLUTION_X, REFLECT_RESOLUTION_Y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //init data buffer
        glGenBuffers(1, &planeDataBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, planeDataBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_REFLECT_PLANE * sizeof(PlaneData), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
    }

    void addReflectPlane(ReflectPlane reflectPlane)
    {
        if(reflectPlanes.size() >= MAX_REFLECT_PLANE)
        {
            return;
        }       
        reflectPlanes.push_back(reflectPlane);

        // update texture offset
        for(int i = 0; i < reflectPlane.model.meshes.size(); i++)
        {
            textureOffset = max(textureOffset, (GLuint)(reflectPlane.model.meshes[i].textures.size()));
        }
    }

    void removeReflectPlane(int index)
    {
        if (index < reflectPlanes.size())
        {
            reflectPlanes.erase(reflectPlanes.begin() + index);
        }
    }

    void clear()
    {
        reflectPlanes.clear();
        textureOffset = 0;
    }

    void generateReflection(Camera& camera, LightManager& lightManager, vector<Model>& models)
    {
        maskShader.use();
        maskShader.setCamera(camera);
        DrawMask();
        // DebugMask();
        reflectShader.use();
        reflectShader.setCamera(camera);
        lightManager.Apply(reflectShader);
        DrawReflect(models);
        DebugMask();
    }

    void Draw(Shader &shader)
    {
        for (int i = 0; i < reflectPlanes.size(); i++)
        {
            reflectPlanes[i].Draw(shader);
        }
    }

private:
    struct PlaneData
    {
        glm::vec4 position;
        glm::vec4 normal;
    };
    vector<ReflectPlane> reflectPlanes;
    Shader maskShader, reflectShader;
    Shader debugShader;
    GLuint framebuffer, planeDataBuffer;
    GLuint texMask, texReflect;
    // To avoid conflict with mesh textures, we bind texMask and texReflect at GL_TEXTURE0 + textureOffset 
    GLuint textureOffset = 0;
    // debug
    ScreenQuad debugQuad;

    void DrawMask()
    {
        // disable blend when rendering mask
        glDisable(GL_BLEND);

        // set framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearTexImage(texMask, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texMask, 0);
        
        // render mask
        for(int i = 0; i < reflectPlanes.size(); i++)
        {
            maskShader.setUint("maskId", i);
            reflectPlanes[i].Draw(maskShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glEnable(GL_BLEND);
    }

    void DrawReflect(vector<Model>& models)
    {
        // set framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);
        glClearTexImage(texReflect, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texReflect, 0);

        // generate reflect plane data
        vector<PlaneData> planeData;
        for (int i = 0; i < reflectPlanes.size(); i++)
        {
            PlaneData data;
            data.position = glm::vec4(reflectPlanes[i].model.position, 1.0f);
            data.normal = glm::vec4(reflectPlanes[i].getNormal(), 0.0f);
            planeData.push_back(data);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, planeDataBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, planeData.size() * sizeof(PlaneData), planeData.data());
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, planeDataBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // set uniforms    
        reflectShader.setUint("GL_Num_ReflectPlane", reflectPlanes.size());
        glActiveTexture(GL_TEXTURE0 + textureOffset);
        glBindTexture(GL_TEXTURE_2D, texMask);
        reflectShader.setInt("texture_mask", textureOffset);
        glActiveTexture(GL_TEXTURE0);

        // render reflection
        for(int i = 0; i < models.size(); i++)
        {
            models[i].Draw(reflectShader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void DebugMask()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        debugShader.use();
        debugQuad.setTexture(texReflect);
        debugQuad.Draw(debugShader);
    }
};

#endif
