#version 330

in vec3 fragWorldPos;

uniform vec3  uCenter;
uniform float uElapsed;
uniform float uDuration;
uniform float uMaxRadius;
uniform vec4  uColor;

out vec4 finalColor;

void main()
{
    float dist = length(fragWorldPos - uCenter);

    float t          = clamp(uElapsed / uDuration, 0.0, 1.0);
    float waveRadius = t * uMaxRadius;
    float ringWidth  = uMaxRadius * 0.08;

    float ring = smoothstep(waveRadius - ringWidth, waveRadius, dist)
               - smoothstep(waveRadius, waveRadius + ringWidth * 0.6, dist);

    float alpha = ring * (1.0 - t) * uColor.a;
    if (alpha < 0.005) discard;

    finalColor = vec4(uColor.rgb, alpha);
}
