#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube uSkybox;

void main()
{
    FragColor = texture(uSkybox, WorldPos);
}