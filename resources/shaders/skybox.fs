#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube texture_skybox;

void main()
{    
    FragColor = texture(texture_skybox, TexCoords);
}