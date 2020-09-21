#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec2 uv_offset;
uniform sampler2D diffuse;
uniform sampler2D normalmap;
uniform sampler2D overlay;

vec3 align(in vec3 axis, in vec3 v) {
    float s = sign(axis.z + 0.001f);
    vec3 w = vec3(v.x, v.y, v.z * s);
    vec3 h = vec3(axis.x, axis.y, axis.z + s);
    float k = dot(w, h) / (1.f + abs(axis.z));
    return k * h - w;
}

void main() {
    vec3 diff = texture(diffuse, tc).rgb + texture(overlay, 0.25 * tc + uv_offset).rgb;
    vec3 N = align(norm_wc, 2 * texture(normalmap, tc).xyz - 1);
    float n_dot_l = max(0, dot(N, -light_dir));
    out_col = vec4(diff * (ambient_col + light_col * n_dot_l), 1);
}
