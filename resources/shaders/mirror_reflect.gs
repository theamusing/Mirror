#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 30) out;

struct PlaneData{
    vec4 position;
    vec4 normal;
};

layout(std430, binding = 2) buffer GL_PLANEDATA_BUFFER{
    PlaneData GL_PlaneData[];
};

in vec2 TexCoords[];
in vec3 Normal[];
in vec3 WorldPos[];

out vec2 gTexCoords;
out vec3 gNormal;
out vec3 gWorldPos;
out vec2 screenCoords;
out float maskId;

uniform uint GL_Num_ReflectPlane;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 v0 = WorldPos[0];
    vec3 v1 = WorldPos[1];
    vec3 v2 = WorldPos[2];

    mat4 transform = projection * view;

    for(int i = 0; i < GL_Num_ReflectPlane; i++)
    {
        vec3 pos = GL_PlaneData[i].position.xyz;
        vec3 normal = GL_PlaneData[i].normal.xyz;
        float d0 = dot(v0 - pos, normal);
        float d1 = dot(v1 - pos, normal);
        float d2 = dot(v2 - pos, normal);
        if(d0 <= 0 && d1 <= 0 && d2 <= 0)
            continue;

        // point 1       
        vec3 p0 = v0 - 2 * normal * d0;
        gl_Position = transform * vec4(p0, 1.0);
        gTexCoords = TexCoords[0];
        gNormal = Normal[0];
        gWorldPos = WorldPos[0];
        screenCoords = (gl_Position.xy / gl_Position.w + vec2(1.0)) / 2;
        maskId = i;
        EmitVertex();

        // point 2
        vec3 p1 = v1 - 2 * normal * d1;
        gl_Position = transform * vec4(p1, 1.0);
        gTexCoords = TexCoords[1];
        gNormal = Normal[1];
        gWorldPos = WorldPos[1];
        screenCoords = (gl_Position.xy / gl_Position.w + vec2(1.0)) / 2;
        maskId = i;
        EmitVertex();

        // point 3
        vec3 p2 = v2 - 2 * normal * d2;
        gl_Position = transform * vec4(p2, 1.0);
        gTexCoords = TexCoords[2];
        gNormal = Normal[2];
        gWorldPos = WorldPos[2];
        screenCoords = (gl_Position.xy / gl_Position.w + vec2(1.0)) / 2;
        maskId = i;
        EmitVertex();
        EndPrimitive();
    }
}