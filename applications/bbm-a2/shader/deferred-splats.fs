#version 130
in vec4 particle_pos;
flat in int particle_time;
out vec4 out_col;
uniform vec2 near_far;
uniform vec2 screenres;
uniform float radius;
uniform sampler2D gbuf_depth;
uniform sampler2D gbuf_diff;
uniform sampler2D gbuf_pos;
uniform sampler2D gbuf_norm;

float linear_depth(in float depth, in float near, in float far) { return (2.0 * near) / (far + near - depth * (far - near)); }

void main() {
    out_col = vec4(0,1,0,1);
#ifndef A2_5
    // geometry inside splat radius?
    vec2 tc = gl_FragCoord.xy / screenres;
    float gbuf_depth = texture(gbuf_depth, tc).r;
    if (gl_FragCoord.z >= gbuf_depth) { discard; return; }
    if (gbuf_depth == 1.0) { discard; return; }
    vec3 gbuf_wpos = texture(gbuf_pos, tc).xyz;
    vec3 wc_dist = particle_pos.xyz - gbuf_wpos;
    float len = length(wc_dist);
    if (len > radius) { discard; return; }
    float dist = clamp(len / radius, 0, 1);

    // diffuse lighting
    vec3 lightcolor = 0.5 * vec3(1., 115/255.f, 65/255.f);
    vec3 norm = texture(gbuf_norm, tc).xyz;
    vec3 to_light = normalize(wc_dist);
    float n_dot_l = max(0.f, dot(norm, to_light));
    float falloff = 1 - dist * dist;
    vec3 diff = texture(gbuf_diff, tc).rgb;
    out_col = vec4(diff * lightcolor * n_dot_l * falloff, 0);

    // soften
    float frag_depth = linear_depth(gl_FragCoord.z, near_far.x, near_far.y);
    float geom_depth = linear_depth(gbuf_depth, near_far.x, near_far.y);
    float fade_geom = clamp(abs(frag_depth - geom_depth) / 0.01f, 0, 1);
    float fade_time = clamp(particle_time / 500.f, 0, 1);
    out_col *= fade_geom * fade_time;
#endif
}
