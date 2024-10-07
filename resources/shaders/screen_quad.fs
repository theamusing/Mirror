#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D Texture;

void main()
{    
    vec3 color = texture(Texture, TexCoords).xyz;
    if(color.r == 0.0 && color.g == 0.0 && color.b == 0.0)
        discard;
    FragColor = vec4(color, 1.0);
}
