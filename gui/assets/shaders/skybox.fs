#version 330

in vec3 fragDir;
uniform float time;
out vec4 finalColor;

#define PI  3.141592654
#define TWIRLY 2.5

// ── PARAMÈTRES DE CONTRÔLE GÉNÉRAUX ─────────────────────────────────────────
const float STARS_INTENSITY  = 4.0;   
const float COSMIC_SATURATION = 0.95; 

// ── PARAMÈTRES DE L'UNIQUE GALAXIE SATELLITE (EN BAS) ───────────────────────
const vec3  GALAXY_CENTER   = vec3(0.0, -0.45, -1.0); 
const float GALAXY_SCALE    = 0.32;                  
const vec2  GALAXY_ANISOTROPY = vec2(1.0, 0.38);     

// ── PARAMÈTRES DE L'UNIQUE PLANÈTE (DERRIÈRE, HAUT) ─────────────────────────
const vec3  PLANET_CENTER   = vec3(0.0, 0.45, 1.0); 
const float PLANET_RADIUS   = 0.12; 

// ── CONFIGURATION STAR NEST RESTE OPTIMISÉE (CONSERVE LES FPS) ──────────────
#define ITERATIONS 11        
#define FORMUPARAM 0.53
#define VOLSTEPS 7          
#define STEPSIZE 0.22        

#define SPEED       0.0005  
#define BRIGHTNESS  0.0015  
#define DARKMATTER  0.050   
#define DISTFADING  0.550   

// Approximation rapide pour le fond
float fast_tanh(float x) { return x / (1.0 + abs(x)); }

float hash( float n ) { return fract(cos(n)*41415.92653); }

vec3 hash3(vec3 p) {
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.xxy + p.yzz) * p.zyx);
}

// ── FONCTIONS DE LA GALAXIE ──────────────────────────────────────────────────
void rot(inout vec2 p, float a) {
    float c = cos(a), s = sin(a);
    p = vec2(c*p.x + s*p.y, -s*p.x + c*p.y);
}

vec2 toPolar(vec2 p) { return vec2(length(p), atan(p.y, p.x)); }
vec2 toRect(vec2 p)  { return p.x*vec2(cos(p.y), sin(p.y)); }

vec2 mod2(inout vec2 p, vec2 size) {
    vec2 c = floor((p + size*0.5)/size);
    p = mod(p + size*0.5, size) - size*0.5;
    return c;
}

float noise1(vec2 p) {
    p *= fast_tanh(0.1*length(p));
    float tm = time * 0.04; 
    float a = cos(p.x), b = cos(p.y);
    float c = cos(p.x*1.87 + tm), d = cos(p.y*1.22 + tm);
    return a*b*c*d;
}

float rand(vec2 co){
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float getGalaxyNoise(vec2 p, float a, float z) {
    vec2 pp = toPolar(p);
    pp.y += pp.x*TWIRLY + a;
    return noise1(toRect(pp) * z);
}

float getGalaxyHeight(vec2 p) {
    float ang = atan(p.y, p.x);
    float l = length(p);
    float sp = mix(1.0, pow(0.75 + 0.25*sin(2.0*(ang + l*TWIRLY)), 3.0), fast_tanh(6.0*l));
    float s = 0.0, a = 1.0, f = 15.0, d = 0.0;
    
    for (int i = 0; i < 4; ++i) { 
        s += a * getGalaxyNoise(p, (time * 0.04) * (0.025 * float(i)), f);
        a *= 0.67;
        f *= 1.414;
        d += a;
    }
    s *= sp;
    float value = -0.25 + s/d;
    float sabsVal = mix((1.0)*value*value+0.25, abs(value), step(0.0, abs(value)-0.5));
    return sabsVal * exp(-5.5*l*l);
}

vec3 getGalaxyStars(vec2 p) {
    vec2 pp = toPolar(p);
    pp.x /= (1.0+length(pp.x))*0.5;
    p = toRect(pp);

    float sz = 0.012; 
    vec3 s = vec3(10000.0);
    for (int i = 0; i < 2; ++i) {  
        rot(p, 0.5);
        vec2 ip = p;
        vec2 n = mod2(ip, vec2(sz));
        float r = rand(n);
        if (r > 0.4) { 
            vec2 o = -1.0 + 2.0*vec2(r, fract(r*1000.0));
            s.x = min(s.x, length(ip-0.25*sz*o));
            s.yz = n*0.1;
        }
    }
    return s;
}

// ── TEXTURE DE JUPITER RESTAURÉE (10 PAS POUR UN MAX DE DÉTAILS) ─────────────
vec3 makeJupiter(vec2 uv) {
    float timeScale = 0.5; 
    vec2 zoom = vec2(20.0, 5.5);
    vec2 offset = vec2(2.0, 1.0);

    vec2 point = uv * zoom + offset;
    
    for(int i = 1; i < 10; i++) {
        float float_i = float(i); 
        point.x += 0.2 * sin(float_i * point.y + time * timeScale);
        point.y += 0.3 * cos(float_i * point.x + time * 0.2);
    }
        
    float r = cos(point.x + point.y + 2.0) * 0.5 + 0.5;
    float g = sin(point.x + point.y + 2.2) * 0.5 + 0.5;
    float b = (sin(point.x + point.y + 1.0) + cos(point.x + point.y + 1.5)) * 0.5 + 0.5;
    
    return vec3(r, g, b) + vec3(0.4);
}

// ── ÉTOILES DU FOND COMBINÉES ────────────────────────────────────────────────
vec3 getLayeredStars(vec3 dir) {
    vec3 colAccum = vec3(0.0);
    vec3 p = dir * 75.0; vec3 i = floor(p); vec3 f = fract(p); vec3 r3 = hash3(i);
    if (r3.x >= 0.975) {
        float tw = mix(0.4, 1.0, 0.5 + 0.5 * sin(time * 1.5 + r3.y * 100.0));
        colAccum += mix(vec3(1.0), vec3(0.9, 0.95, 1.0), r3.z) * (smoothstep(0.08, 0.0, length(f - (0.5 + 0.35 * sin(r3 * 6.2831)))) * 3.0) * tw * r3.y * 1.8;
    }
    p = dir * 35.0; i = floor(p); f = fract(p); r3 = hash3(i);
    if (r3.x >= 0.990) {
        float tw = mix(0.4, 1.0, 0.5 + 0.5 * sin(time * 2.5 + r3.y * 100.0));
        colAccum += mix(vec3(1.0), vec3(0.9, 0.95, 1.0), r3.z) * (smoothstep(0.08, 0.0, length(f - (0.5 + 0.35 * sin(r3 * 6.2831)))) * 3.0) * tw * r3.y * 2.5;
    }
    return colAccum * STARS_INTENSITY;
}

// ── MAIN ────────────────────────────────────────────────────────────────────
void main() {
    vec3 dir = normalize(fragDir); 
    vec3 finalOutColor = vec3(0.0);
    
    // RENDU DE LA PLANÈTE (PRIORITAIRE ET DE HAUTE QUALITÉ)
    vec3 forwardPl = normalize(PLANET_CENTER);
    if (dot(dir, forwardPl) > 0.85) { 
        vec3 rightPl = normalize(cross(vec3(0.0, 1.0, 0.0), forwardPl));
        vec3 upPl = cross(forwardPl, rightPl);

        vec2 planetScreenUV = vec2(dot(dir, rightPl), dot(dir, upPl));
        float dis = length(planetScreenUV);

        if (dis < PLANET_RADIUS) {
            vec2 planetCoord = planetScreenUV / PLANET_RADIUS;
            
            float sphereDis = 1.0 - pow(max(0.0, 1.0 - length(planetCoord)), 0.6);
            planetCoord = normalize(planetCoord) * sphereDis;
            planetCoord = (planetCoord + 1.0) * 0.5;

            float light = pow(max(0.0, planetCoord.x), 2.0 * (cos(time * 0.1 + 1.0) + 1.5));
            vec3 surfaceColor = makeJupiter(vec2(planetCoord.y, planetCoord.x)) * light; 
            
            float fresnelIntensity = pow(dis / PLANET_RADIUS, 3.0);
            vec3 fresnelPlanet = mix(surfaceColor, vec3(0.7, 0.6, 0.5), fresnelIntensity * pow(max(0.0, planetCoord.x), 2.0));
            
            finalColor = vec4(fresnelPlanet * planetCoord.x * 1.1, 1.0);
            return; 
        } 
        else {
            float distFromSurface = dis - PLANET_RADIUS;
            float atmosGlow = exp(-distFromSurface * 22.0) * 0.18;
            finalOutColor += vec3(0.7, 0.6, 0.5) * atmosGlow * smoothstep(-0.05, 0.15, planetScreenUV.x + 0.05);
        }
    }

    // 2. FOND COSMIQUE INTERSTÉLLAIRE
    float starNestTime = time * SPEED;
    vec3 from = vec3(1.0, 0.5, 0.5) + vec3(starNestTime * 2.0, starNestTime, -2.0);
    
    float s = 0.1, fade = 1.0;
    vec3 cosmicBackground = vec3(0.0);
    
    for (int r = 0; r < VOLSTEPS; r++) {
        vec3 p = from + s * dir * 0.5; 
        p = abs(vec3(0.85) - mod(p, vec3(1.7))); 
        
        float pa, a = pa = 0.0;
        for (int i = 0; i < ITERATIONS; i++) { 
            p = abs(p) / dot(p, p) - FORMUPARAM; 
            a += abs(length(p) - pa);
            pa = length(p);
        }
        
        float dm = max(0.0, DARKMATTER - a * a * 0.001);
        a = max(0.0, a - 8.5); 
        a *= a * a; 
        
        if (r > 3) fade *= 1.0 - dm;
        
        cosmicBackground += vec3(s, s * s, s * s * s * s) * a * BRIGHTNESS * fade;
        fade *= DISTFADING;
        s += STEPSIZE;
    }
    
    finalOutColor += mix(vec3(length(cosmicBackground)), cosmicBackground, COSMIC_SATURATION) * 0.012;

    // 3. RENDU DE LA GALAXIE SATELLITE
    vec3 forwardGal = normalize(GALAXY_CENTER);
    if (dot(dir, forwardGal) > 0.0) { 
        vec3 rightGal = normalize(cross(vec3(0.0, 1.0, 0.0), forwardGal));
        vec3 upGal = cross(forwardGal, rightGal);
        
        vec2 uvGalaxy = vec2(dot(dir, rightGal), dot(dir, upGal)) / GALAXY_SCALE;
        uvGalaxy.y /= GALAXY_ANISOTROPY.y; // Correction de l'orthographe ici !
        
        float distToGalaxy = length(uvGalaxy);

        if (distToGalaxy < 1.6) {
            rot(uvGalaxy, 0.03 * time); 

            float h = getGalaxyHeight(uvGalaxy);
            float fakeNormalY = smoothstep(1.0, 0.0, h * 0.5); 

            vec3 galColor = vec3(0.22, 0.22, 0.45) * h; 
            galColor += vec3(0.08) * pow(max(0.0, 1.0 - distToGalaxy), 4.0); 
            galColor += pow(vec3(0.25) * h, fakeNormalY * 1.75 * (mix(vec3(0.5, 1.0, 1.5), vec3(1.5, 1.0, 0.5), 1.25 * fast_tanh(distToGalaxy))));

            vec3 gStars = getGalaxyStars(uvGalaxy);
            if (gStars.x < 100.0) { 
                float sr = rand(gStars.yz);
                float slowTwinkle = mix(0.2, 1.0, 0.5 + 0.5 * sin(time * 0.8 + sr * 20.0));
                float si = pow(max(0.0, fast_tanh(h) * sr), 0.25) * 0.0006; 
                
                vec3 starScatter = sr * 3.5 * exp(-2.5 * distToGalaxy * distToGalaxy) * tanh(pow(si / max(0.0001, gStars.x), 2.2)) * mix(vec3(0.5, 0.75, 1.0), vec3(1.0, 0.75, 0.5), sr * 0.6);
                galColor += clamp(starScatter * slowTwinkle, 0.0, 1.0) * smoothstep(0.0, 0.35, 1.0 - fakeNormalY);
            }

            finalOutColor = mix(finalOutColor, galColor * 0.45, smoothstep(1.6, 0.4, distToGalaxy) * smoothstep(0.0, 0.03, h));
        }
    }

    // 4. ÉTOILES DE PREMIER PLAN
    finalOutColor += getLayeredStars(dir);
    
    // 5. CHAINE DE TONEMAPPING & COLOR GRADING
    finalOutColor *= 1.3; 
    finalOutColor = pow(finalOutColor, vec3(1.5));
    finalOutColor = finalOutColor / (1.0 + finalOutColor);
    finalOutColor = pow(finalOutColor, vec3(1.0 / 1.5));
    
    finalOutColor = mix(finalOutColor, finalOutColor * finalOutColor * (3.0 - 2.0 * finalOutColor), vec3(1.0));
    finalOutColor = pow(finalOutColor, vec3(1.3, 1.20, 1.0));    
    finalColor = vec4(pow(clamp(finalOutColor * 1.01, vec3(0.0), vec3(1.0)), vec3(0.7 / 2.2)), 1.0);
}