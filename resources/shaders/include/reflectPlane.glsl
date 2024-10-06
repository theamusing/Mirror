#ifndef REFLECTPLANE_GLSL
#define REFLECTPLANE_GLSL

struct ReflectPlane {
    vec4 position;
    vec4 normal;
    uint reflectionDepth;
}

layout(std430, binding = 2) buffer GL_REFLECTPLANE_BUFFER
{
    ReflectPlane GL_ReflectPlane[];
}

uniform uint GL_Num_ReflectPlane;

#endif /* REFLECTPLANE_GLSL */