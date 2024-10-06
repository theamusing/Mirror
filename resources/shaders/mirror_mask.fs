#version 460 core

layout(location = 0) out float FragColor;

uniform uint maskId;

void main()
{    
    FragColor = 1;//float(maskId) / 255.0;
}