#version 460 core

layout(location = 0) out float FragColor;

uniform uint maskId;

void main()
{    
    FragColor = (float(maskId) + 1.0) / 255.0;
}