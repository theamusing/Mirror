#version 460 core
#extension GL_ARB_shading_language_include : require
#include "/include/light.glsl"

layout(location = 0) out vec4 FragColor;

struct PlaneData{
    vec4 position;
    vec4 normal;
};

layout(std430, binding = 2) buffer GL_PLANEDATA_BUFFER{
    PlaneData GL_PlaneData[];
};

in vec2 gTexCoords;
in vec3 gNormal;
in vec3 gWorldPos;
in float maskId;

uniform uint GL_Num_ReflectPlane;
uniform sampler2D texture_diffuse1;
// uniform sampler2D texture_specular1;
uniform sampler2D texture_mask;

uniform vec3 cameraPos;
uniform vec2 screenResolution;

void main()
{   
    vec2 screenCoords = gl_FragCoord.xy / screenResolution;
    float id = texture(texture_mask, screenCoords).r * 255.0 - 1.0;
    if(abs(id - maskId) > 1e-3)
        discard;
    int planeId = int(id + 0.5);
    planeId = clamp(planeId, 0, int(GL_Num_ReflectPlane)-1);
    vec3 viewPos = cameraPos - 2 * dot(cameraPos - GL_PlaneData[planeId].position.xyz, GL_PlaneData[planeId].normal.xyz) * GL_PlaneData[planeId].normal.xyz;

    vec3 norm = normalize(gNormal);
    vec3 kd = vec3(texture(texture_diffuse1, gTexCoords));
    vec3 ks = vec3(0.2);
    // vec3 ks = vec3(texture(texture_specular1, TexCoords));

    FragColor = vec4(calculateLight(gWorldPos, norm, normalize(viewPos-gWorldPos), kd, ks), 1.0);
}
