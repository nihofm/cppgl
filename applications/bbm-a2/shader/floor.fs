#version 150
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
out vec3 out_pos;
out vec3 out_norm;
uniform sampler2D diffuse;
uniform sampler2D normalmap;

vec3 align(vec3 axis, vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.f + abs(axis.z));
    return k * h - w;
}

void main() {
    vec2 TC = fract(tc);
    vec4 diff = texture(diffuse, TC);
    vec3 N = align(norm_wc, 2 * texture(normalmap, TC).xyz - 1);
    out_col = diff;
    out_pos = pos_wc.xyz;
    out_norm = N;
}
