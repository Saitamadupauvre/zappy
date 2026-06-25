#version 330

in vec3 fragNormal;
in vec3 fragWorldPos;

uniform vec4  colDiffuse;   // crystal tint color (set per-draw via material diffuse)
uniform float emissiveStrength; // how much the crystal self-illuminates
uniform float alpha;            // base transparency

out vec4 finalColor;

void main()
{
    vec3  n        = normalize(fragNormal);
    // Fresnel: edges glow more than the face-on center
    float fresnel  = pow(1.0 - abs(dot(n, normalize(-fragWorldPos))), 2.5);

    vec3  base     = colDiffuse.rgb;
    vec3  emissive = base * emissiveStrength;
    vec3  color    = base + emissive + base * fresnel * 0.6;

    // Fresnel also boosts alpha at the edges to give depth
    float a = clamp(alpha + fresnel * 0.35, 0.0, 1.0);

    finalColor = vec4(color, a);
}
