#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"
#include "/include/reflectPlane.glsl"

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;

uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;
uniform samplerCube texture_skybox;
uniform sampler2D texture_reflect;

uniform vec3 cameraPos;
uniform vec2 screenResolution;

uniform uint planeId;

float fresnelSchlick(float cosTheta, float refIndex);

void main()
{    
    vec3 norm = GL_ReflectPlane[planeId].normal.xyz;
    vec3 kd = vec3(texture(texture_diffuse1, TexCoords));
    vec3 ks = vec3(0.2);
    // vec3 ks = vec3(texture(texture_specular1, TexCoords));

    vec2 screenCoords = gl_FragCoord.xy / screenResolution;

    float blurLevel = GL_ReflectPlane[planeId].blurLevel;
    vec4 reflectData = textureLod(texture_reflect, screenCoords, blurLevel);
    vec3 reflectColor = GL_ReflectPlane[planeId].color.xyz;
    vec3 skyColor = textureLod(texture_skybox, reflect(WorldPos - cameraPos, norm), blurLevel).xyz;

    reflectColor *= mix(reflectData.xyz, skyColor, 1 - reflectData.a);

    // reflectColor *=  fresnelSchlick(abs(dot(norm, normalize(cameraPos - WorldPos))), 0.2);

    vec3 baseColor = calculateLight(WorldPos, norm, normalize(cameraPos - WorldPos), kd, ks);

    float reflectRate = GL_ReflectPlane[planeId].reflectRate;
    FragColor = vec4(mix(reflectColor, baseColor, 1 - reflectRate), 1.0);
}

float fresnelSchlick(float cosTheta, float refIndex)
{
    float r0 = (1.0 - refIndex) / (1.0 + refIndex);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}
