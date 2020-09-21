#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
out vec3 out_pos;
out vec3 out_norm;
uniform sampler2D diffuse;

void main() {
    vec4 diff = texture(diffuse, tc);
    out_col = diff;
    out_pos = pos_wc.xyz;
    out_norm = norm_wc;
}
