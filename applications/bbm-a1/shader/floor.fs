#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform sampler2D diffuse;
uniform sampler2D normalmap;

vec3 align(in vec3 axis, in vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.f + abs(axis.z));
    return k * h - w;
}

void main() {

    vec2 TC = fract(tc);
    vec3 diff = texture(diffuse, TC).rgb;
    vec3 N = align(norm_wc, 2 * texture(normalmap, TC).xyz - 1);
    float n_dot_l = max(0, dot(N, -light_dir));
    out_col = vec4(diff * (ambient_col + light_col * n_dot_l), 1);
}
