#version 330

in vec3 fragDir;
uniform float time;
out vec4 finalColor;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// 2-octave noise — cheap but enough for clouds/geography
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1,0)), u.x),
               mix(hash(i + vec2(0,1)), hash(i + vec2(1,1)), u.x), u.y);
}

float fbm2(vec2 p) {
    return 0.6 * noise(p) + 0.4 * noise(p * 2.1);
}

bool hitSphere(vec3 camera, vec3 dir, out vec2 pos) {
    float b = dot(camera, dir);
    if (b > 0.0) return false;
    float h = b * b - (dot(camera, camera) - 1.0);
    if (h <= 0.0) return false;
    vec3 p = camera + (-b - sqrt(h)) * dir;
    pos = p.xz + time * vec2(0.004, 0.016);
    return true;
}

vec3 getStars(vec3 dir) {
    // Single layer, cheap hash
    vec3 p = dir * 55.0;
    vec3 i = floor(p); vec3 f = fract(p);
    float r = hash(i.xy + i.z * 37.0);
    if (r < 0.97) return vec3(0.0);
    float tw = 0.5 + 0.5 * sin(time * 2.0 + r * 63.0);
    return vec3(smoothstep(0.12, 0.0, length(f - 0.5)) * tw * r * 4.0);
}

void main() {
    vec3 dir    = normalize(fragDir);
    vec3 camera = vec3(0.0, 1.2, 0.7);

    // ── EARTH ────────────────────────────────────────────────────────────────
    vec3 color;
    vec2 pos;
    if (hitSphere(camera, dir, pos)) {
        float geo    = fbm2(5.0 * pos);
        vec3  land   = mix(vec3(0.05, 0.40, 0.05), vec3(0.20, 0.55, 0.10),
                           smoothstep(0.4, 0.65, geo));
        vec3  water  = vec3(0.0, 0.06, 0.18);
        vec3  ground = mix(water, land, smoothstep(0.48, 0.52, geo));
        float clouds = smoothstep(0.55, 0.70, fbm2(8.0 * pos + 0.3));
        color = mix(ground, vec3(0.85), clouds);
    } else {
        // ── SPACE + STARS ─────────────────────────────────────────────────
        color = vec3(0.0, 0.0, 0.015) + getStars(dir);
    }

    // ── ATMOSPHERE LIMB ───────────────────────────────────────────────────
    float b       = dot(camera, dir);
    vec3  alt     = camera - dir * b;
    float horizon = sqrt(dot(alt, alt));
    float atmMask = step(b, 0.0);
    float glow    = smoothstep(1.02, 0.98, horizon) * atmMask;
    color = mix(color, vec3(0.15, 0.35, 0.6), glow * 0.7);

    finalColor = vec4(color, 1.0);
}
