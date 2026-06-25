#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 mvp;
uniform float thickness;

void main()
{
    vec3 pos = vertexPosition + (vertexNormal * thickness);
    gl_Position = mvp * vec4(pos, 1.0);
}