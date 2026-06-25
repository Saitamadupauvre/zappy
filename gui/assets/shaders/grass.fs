#version 330

in float fragHeight;        // 0 at root, 1 at tip
in vec4  fragInstanceColor; // per-blade tint (carries tile darkness)

out vec4 finalColor;

void main()
{
    // Blade colour IS the tile colour (carried per-instance), just shaded along
    // its height: darker at the root, a touch brighter at the tip.
    float shade = mix(0.55, 1.15, fragHeight);
    vec3 grass = clamp(fragInstanceColor.rgb * shade, 0.0, 1.0);

    finalColor = vec4(grass, 1.0);
}
