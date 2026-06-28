// Forked from the amazing shader by mhnewman: https://www.shadertoy.com/view/XlXGD7

#version 330

in vec3 fragDir;
uniform float time;
out vec4 finalColor;

const mat2 m = mat2(1.616, 1.212, -1.212, 1.616);

float hash12(vec2 p) {
	p = fract(p * vec2(5.3983, 5.4427));
    p += dot(p.yx, p.xy + vec2(21.5351, 14.3137));
	return fract(p.x * p.y * 95.4337);
}

vec2 hash21(float p) {
	vec2 p2 = fract(p * vec2(5.3983, 5.4427));
    p2 += dot(p2.yx, p2.xy + vec2(21.5351, 14.3137));
	return fract(vec2(p2.x * p2.y * 95.4337, p2.x * p2.y * 97.597));
}

vec3 hash3(vec3 p) {
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.xxy + p.yzz) * p.zyx);
}

float noise(in vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(hash12(i + vec2(0.0, 0.0)),
                   hash12(i + vec2(1.0, 0.0)), u.x),
               mix(hash12(i + vec2(0.0, 1.0)),
                   hash12(i + vec2(1.0, 1.0)), u.x), u.y);
}

float hash12_3(vec2 p) {
	float f = hash12(p);
    return f * f * f;
}

float noise_3(in vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
	vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(mix(hash12_3(i + vec2(0.0, 0.0)),
                   hash12_3(i + vec2(1.0, 0.0)), u.x),
               mix(hash12_3(i + vec2(0.0, 1.0)),
                   hash12_3(i + vec2(1.0, 1.0)), u.x), u.y);
}

float fbm(vec2 p) {
    float f = 0.0;
    f += 0.5    * noise(p); p = m * p;
    f += 0.25   * noise(p); p = m * p;
    f += 0.125  * noise(p); p = m * p;
    f += 0.0625 * noise(p); p = m * p;
    f += 0.03125  * noise(p); p = m * p;
    f += 0.015625 * noise(p);
    return f / 0.984375;
}

// Sphere intersection: camera orbits above unit sphere, ray may hit it
bool getPosition(in vec3 camera, in vec3 dir, out vec2 pos) {
	float b = dot(camera, dir);
	float c = dot(camera, camera) - 1.0;
	float h = b * b - c;
	if (h <= 0.0) return false;
    float t = -b - sqrt(h);
    if (t <= 0.0) return false; // intersection is behind the ray
    vec3 p = camera + t * dir;
    pos = p.xz + time * vec2(0.005, 0.02);
	return true;
}

vec3 getStars(vec3 dir) {
    vec3 col = vec3(0.0);
    vec3 p = dir * 75.0;
    vec3 i = floor(p); vec3 f = fract(p); vec3 r3 = hash3(i);
    if (r3.x >= 0.975) {
        float tw = mix(0.4, 1.0, 0.5 + 0.5 * sin(time * 1.5 + r3.y * 100.0));
        col += mix(vec3(1.0), vec3(0.9, 0.95, 1.0), r3.z)
             * smoothstep(0.08, 0.0, length(f - 0.5)) * 3.0 * tw * r3.y * 1.8;
    }
    p = dir * 35.0;
    i = floor(p); f = fract(p); r3 = hash3(i);
    if (r3.x >= 0.990) {
        float tw = mix(0.4, 1.0, 0.5 + 0.5 * sin(time * 2.5 + r3.y * 100.0));
        col += mix(vec3(1.0), vec3(0.9, 0.95, 1.0), r3.z)
             * smoothstep(0.08, 0.0, length(f - 0.5)) * 3.0 * tw * r3.y * 2.5;
    }
    return col * 3.5;
}

void main() {
    vec3 dir = normalize(fragDir);

    vec3 camera = vec3(0.0, 1.2, 0.7);

    // ── EARTH (round sphere) ─────────────────────────────────────────────────
    vec3 earth = vec3(0.0);
    bool hitEarth = false;
    vec2 position;
    if (getPosition(camera, dir, position)) {
        hitEarth = true;
        float geography = fbm(6.0 * position);

        float coast      = 0.2 * pow(geography + 0.5, 50.0);
        float population = smoothstep(0.2, 0.6, fbm(2.0 * position) + coast);
        vec2 p = 40.0 * position;
        population *= (noise_3(p) + coast); p = m * p;
        population *= (noise_3(p) + coast); p = m * p;
        population *= (noise_3(p) + coast); p = m * p;
        population *= (noise_3(p) + coast); p = m * p;
        population *= (noise_3(p) + coast);
        population = smoothstep(0.0, 0.02, population);

        vec3 land  = vec3(0.1 + 2.0 * population, 0.07 + 1.3 * population, population);
        vec3 water = vec3(0.0, 0.05, 0.1);
        earth = mix(land, water, smoothstep(0.49, 0.5, geography));

        vec2 wind    = vec2(fbm(30.0 * position), fbm(60.0 * position));
        float weather = fbm(20.0 * (position + 0.03 * wind)) * (0.6 + 0.4 * noise(10.0 * position));
        float clouds  = 0.8 * smoothstep(0.35, 0.45, weather) * smoothstep(-0.25, 1.0, fbm(wind));
        earth = mix(earth, vec3(0.5, 0.5, 0.5), clouds);

        // lightning
        vec2 rndDir2   = hash21(time);
        vec3 strikeDir = normalize(vec3(rndDir2.x * 2.0 - 1.0, -1.0, rndDir2.y * 2.0 - 1.0));
        vec2 strike;
        if (getPosition(camera, strikeDir, strike)) {
            vec2 diff = position - strike;
            float lightning = clamp(1.0 - 1500.0 * dot(diff, diff), 0.0, 1.0);
            lightning *= smoothstep(0.65, 0.75, weather);
            earth += lightning;
        }
    }

    // ── ATMOSPHERE GLOW AROUND LIMB ──────────────────────────────────────────
    float b_atm     = dot(camera, dir);
    vec3 altitude   = camera - dir * b_atm;
    float horizon   = sqrt(dot(altitude, altitude));

    // Only show atmosphere when looking toward the earth (b < 0)
    float atmMask   = step(b_atm, 0.0);

    vec3 atmosphere = vec3(0.2, 0.25, 0.3);
    atmosphere = mix(atmosphere, vec3(0.05, 0.1,  0.3),  smoothstep(0.992, 1.004, horizon));
    atmosphere = mix(atmosphere, vec3(0.1,  0.0,  0.0),  smoothstep(1.0,   1.004, horizon));
    atmosphere = mix(atmosphere, vec3(0.2,  0.17, 0.1),  smoothstep(1.008, 1.015, horizon));
    atmosphere = mix(atmosphere, vec3(0.0,  0.0,  0.0),  smoothstep(1.015, 1.02,  horizon));

    // ── SPACE + STARS (background when not hitting earth) ───────────────────
    vec3 space = vec3(0.0, 0.0, 0.02) + getStars(dir);

    // Compose: stars behind atmosphere behind earth
    float horizonBlend = clamp(pow(horizon, 20.0), 0.0, 1.0) * atmMask;
    vec3 color = hitEarth ? earth : space;
    color = mix(color, atmosphere, horizonBlend);

    finalColor = vec4(color, 1.0);
}
