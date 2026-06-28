#version 330

in vec3 vertexPosition;

uniform mat4 matView;
uniform mat4 matProjection;

out vec3 fragDir;

void main()
{
    fragDir = vertexPosition;
    // Strip translation from view matrix so cube follows camera
    vec4 pos = matProjection * mat4(mat3(matView)) * vec4(vertexPosition, 1.0);
    // xyww trick: after perspective divide z/w = 1.0 (max depth) → renders behind all geometry
    gl_Position = pos.xyww;
}
