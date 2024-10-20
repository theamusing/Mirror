#ifndef REFLECTPLANE_GLSL
#define REFLECTPLANE_GLSL

#define MAX_REFLECT_NUM 3

struct GL_PlaneData {
    vec4 position;
    vec4 normal;
    vec4 color;
    float reflectRate;
    float blurLevel;
};

layout(std430, binding = 2) buffer GL_REFLECTPLANE_BUFFER
{
    GL_PlaneData GL_ReflectPlane[];
};

uniform uint GL_Num_ReflectPlane;

#endif /* REFLECTPLANE_GLSL */