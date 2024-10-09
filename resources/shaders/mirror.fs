#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

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

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 kd = vec3(texture(texture_diffuse1, TexCoords));
    vec3 ks = vec3(0.2);
    // vec3 ks = vec3(texture(texture_specular1, TexCoords));

    vec2 screenCoords = gl_FragCoord.xy / screenResolution;

    vec4 reflectData = textureLod(texture_reflect, screenCoords, 0);
    vec3 reflectColor;
    if(reflectData.a > 0.5) // mirror
    {
        reflectColor = reflectData.xyz;
    }
    else // skybox
    {
        reflectColor = textureLod(texture_skybox, reflect(WorldPos - cameraPos, norm), 0).xyz;
    }
    vec3 baseColor = calculateLight(WorldPos, norm, normalize(cameraPos-WorldPos), kd, ks);

    FragColor = vec4(reflectColor * 0.8 + baseColor * 0.2, 1.0);
}
