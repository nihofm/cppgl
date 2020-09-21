#version 150
in vec4 pos_wc;
in vec2 tc;
out vec4 out_col;

uniform mat4 view;
uniform mat4 proj;
uniform sampler2D depth;
uniform sampler2D wpos;
uniform float time;

//--- BEGIN MAGIC
float random(in vec2 _st) { return fract(sin(dot(_st.xy, vec2(12.9898, 78.233)))* 43758.5453123); }
// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 _st) {
    vec2 i = floor(_st);
    vec2 f = fract(_st);
    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}
#define NUM_OCTAVES 7
float fbm(in vec2 _st) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}
float fog(in vec2 st, in float time) {
    vec2 q = vec2(fbm(st + 0.00*time),
            fbm(st + vec2(1.0)));
    vec2 r = vec2(fbm(st + 1.0*q + vec2(1.7,9.2)+ 0.15*time),
            fbm(st + 1.0*q + vec2(8.3,2.8)+ 0.126*time));
    float f = fbm(st+r);
    return sqrt(f*f*f + .6*f*f + .5*f);
}
//--- END MAGIC

void main() {
    // Get screen coordinates of the current fragment for the wpos lookup
    vec4 sc = proj * view * pos_wc;
    sc/=sc.w;
    vec3 fragment_pos_in_fog_wc = texture(wpos, sc.xy * 0.5 + 0.5).xyz;

    // compute noise/fog value
    float n = fog(tc*15 + vec2(time*0.0001), 0.001*time) * 1.5;

    // Distance between the fog plane and the point behind the fog plane
    float l = length(fragment_pos_in_fog_wc.xyz - pos_wc.xyz);

    // Check for invalid values in the wpos buffer
    if (dot(fragment_pos_in_fog_wc.xyz, fragment_pos_in_fog_wc.xyz) <= 0.0)
        l = 10.0;

    // Smooth fading close to geometry
    float alpha = smoothstep(0.0, 6.0, l);

    // Fading outside of the board
    vec2 tcn = tc * 2. - 1.;
    vec2 d = abs(tcn) - vec2(0.5);
    float m = max(d.x, d.y);
    float t = mix(m, length(max(d, 0.0)), step(0.0, m));
    float beta = 1. - smoothstep(0.0, 0.25, t);

    // Apply Beer's law
    float gamma = 1. - exp(-0.075 * l);

    out_col.rgb = vec3(1);
    out_col.a = alpha * beta * gamma * n;
}
