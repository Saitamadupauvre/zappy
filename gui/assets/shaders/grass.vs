#version 330

// Per-vertex blade geometry (local space: +Y up, root at y=0).
in vec3 vertexPosition;
in vec2 vertexTexCoord;   // .y encodes blade height: 0 = root, 1 = tip
in vec3 vertexNormal;

// Per-instance attributes (set up via vertex-attribute divisor = 1).
in mat4 instanceTransform; // occupies attribute locations [loc .. loc+3]
in vec4 instanceColor;     // tile-darkened blade tint

uniform mat4 mvp;          // view * projection (model is the instance transform)
uniform float time;
uniform vec2  windDir;     // normalized wind direction in world XZ
uniform float windStrength;

out float fragHeight;      // 0 at root, 1 at tip
out vec4  fragInstanceColor;

void main()
{
    float height = vertexTexCoord.y;

    // Place the blade in the world via its instance transform.
    vec4 worldPos = instanceTransform * vec4(vertexPosition, 1.0);

    // Wind: phase varies across the field so blades don't sway in unison; the
    // bend grows with height^2 so the root stays anchored and the tip whips.
    float phase = time * 2.0 + dot(worldPos.xz, windDir) * 0.6;
    float sway  = sin(phase) * windStrength * height * height;
    worldPos.xz += windDir * sway;

    fragHeight        = height;
    fragInstanceColor = instanceColor;
    gl_Position = mvp * worldPos;
}
