#version 330

in vec3 fragWorldPos;
in vec3 fragWorldNormal;

uniform vec3  uCenter;
uniform vec3  uSurfaceNormal;
uniform float uTime;
uniform float uRadius;
uniform vec4  uColor;

out vec4 finalColor;

#define PI 3.14159265359
#define TAU 6.28318530718

// Sharp, crisp ring line with slight anti-aliasing
float sharpRing(float d, float r, float thickness) {
    float halfThick = thickness * 0.5;
    // Using a tiny feather (0.003) for crisp anime linework without jagged edges
    return smoothstep(r - halfThick - 0.003, r - halfThick, d) - 
           smoothstep(r + halfThick, r + halfThick + 0.003, d);
}

// Sharp line drawer for geometric shapes
float line(float d, float thickness) {
    return smoothstep(thickness, thickness - 0.003, abs(d));
}

// Generates an N-sided regular polygon distance field
// Used to create crisp geometric stars and triangles instead of soft spokes
float ndfPolygon(vec2 p, float r, int sides) {
    float a = atan(p.x, p.y) + PI;
    float r_side = TAU / float(sides);
    return cos(floor(0.5 + a / r_side) * r_side - a) * length(p) - r;
}

// Energy flicker effect for that unstable magical power look
float energyFlicker(vec2 delta, float t) {
    float angle = atan(delta.y, delta.x);
    return sin(angle * 12.0 + t * 6.0) * cos(angle * 4.0 - t * 4.0) * 0.005;
}

void main()
{
    // Project delta onto the surface tangent plane so distance is measured
    // along the surface regardless of curvature (flat grid or torus).
    // Discard fragments whose surface normal faces away from the center tile's normal.
    // threshold 0.0 = discard back hemisphere; raise to cull more oblique faces.
    if (dot(fragWorldNormal, uSurfaceNormal) < 0.0) discard;

    vec3 delta3 = fragWorldPos - uCenter;
    // Project delta onto this fragment's own tangent plane for correct on-surface distance.
    vec3 tangentDelta = delta3 - dot(delta3, fragWorldNormal) * fragWorldNormal;
    float rawDist = length(tangentDelta);
    vec2 delta = tangentDelta.xz; // angular reference for atan-based patterns

    if (rawDist > uRadius * 1.1) discard;

    float t = uTime;

    float distort = energyFlicker(delta, t) * uRadius;
    float dist = rawDist + distort;

    // 1. CONCENTRIC RINGS (Crisp, layered borders)
    float r1 = sharpRing(dist, uRadius * 0.95, uRadius * 0.015); // Heavy outer ring
    float r2 = sharpRing(dist, uRadius * 0.90, uRadius * 0.005); // Thin inner-outer accent
    float r3 = sharpRing(dist, uRadius * 0.60, uRadius * 0.008); // Mid-layer ring
    float r4 = sharpRing(dist, uRadius * 0.55, uRadius * 0.004); // Mid-layer accent
    float r5 = sharpRing(dist, uRadius * 0.25, uRadius * 0.010); // Inner core ring
    float rings = r1 + r2 + r3 + r4 + r5;

    // 2. ROTATING GEOMETRIC RUNES (Anime Magic Crest)
    // Rotate coordinate spaces for overlapping triangles to form a hexagram (Star of David style)
    float rot1 = t * 0.4;
    mat2 matRot1 = mat2(cos(rot1), -sin(rot1), sin(rot1), cos(rot1));
    vec2 pRot1 = matRot1 * delta;

    float rot2 = -t * 0.25;
    mat2 matRot2 = mat2(cos(rot2), -sin(rot2), sin(rot2), cos(rot2));
    vec2 pRot2 = matRot2 * delta;

    // First triangle (pointing up)
    float tri1 = line(ndfPolygon(pRot1, uRadius * 0.43, 3), uRadius * 0.006);
    // Second triangle (pointing down, offset by 180 degrees via rotation matrix)
    mat2 mat180 = mat2(cos(PI), -sin(PI), sin(PI), cos(PI));
    float tri2 = line(ndfPolygon(mat180 * pRot1, uRadius * 0.43, 3), uRadius * 0.006);
    
    // Outer rotating square accent layer
    float square = line(ndfPolygon(pRot2, uRadius * 0.64, 4), uRadius * 0.004);
    
    float geometry = max(max(tri1, tri2), square);

    // 3. RUNE ACCENTS (Tick marks mimicking ancient text)
    float angle = atan(delta.y, delta.x) + t * 0.1;
    float ticks = 0.0;
    if (dist > uRadius * 0.65 && dist < uRadius * 0.85) {
        float sector = fract(angle / (TAU / 24.0)); // 24 divisions
        ticks = smoothstep(0.08, 0.0, abs(sector - 0.5)) * sharpRing(dist, uRadius * 0.75, uRadius * 0.15);
    }

    // 4. MAGICAL GLOW & BLOOM
    // Intense core energy glow + subtle overall bloom over the linework
    float centerGlow = smoothstep(uRadius * 0.3, 0.0, dist) * 0.4;
    float lineGlow = (rings + geometry + ticks) * 0.5; // Ambient glow surrounding lines

    // Combine masks
    float linework = rings + geometry + ticks;
    float totalAlpha = linework + centerGlow + lineGlow;
    totalAlpha = clamp(totalAlpha * uColor.a, 0.0, 1.0);

    if (totalAlpha < 0.005) discard;

    // 5. ANIME OVEREXPOSURE COLORING
    // Anime energy graphics look best when lines are so hot they turn white at their centers
    vec3 baseColor = uColor.rgb;
    
    // Power pulse that brightens the whole sigil rhythmically
    float pulse = 0.8 + 0.2 * sin(t * 5.0);
    baseColor *= pulse;

    // Mix in pure white bloom based on line thickness intensity
    // This gives that classic "glowing neon / energy beam" overexposed core
    vec3 finalRGB = mix(baseColor, vec3(1.0), smoothstep(0.4, 0.9, linework));
    
    // Add additive flash injection for the glowing regions
    finalRGB += baseColor * centerGlow * 1.5;

    finalColor = vec4(finalRGB, totalAlpha);
}