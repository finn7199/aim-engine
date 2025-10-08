#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main()
{
    WorldPos = aPos;
    // no translation part of the view matrix so the skybox doesn't move
    mat4 viewNoTranslation = mat4(mat3(uView));
    vec4 pos = uProjection * viewNoTranslation * vec4(aPos, 1.0);
    //make the skybox always appear at the furthest possible depth
    gl_Position = pos.xyww;
}