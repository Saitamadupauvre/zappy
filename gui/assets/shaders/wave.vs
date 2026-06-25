#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragWorldPos;

void main()
{
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    fragWorldPos  = worldPos.xyz;
    gl_Position   = mvp * vec4(vertexPosition, 1.0);
}
