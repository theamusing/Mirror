#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture;

void main()
{    
    FragColor = vec4(texture(Texture, TexCoords).xyz, 1.0);
}
