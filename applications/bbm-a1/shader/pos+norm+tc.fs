#version 130
in vec4 pos_wc;
in vec3 norm_wc;
in vec2 tc;
out vec4 out_col;
uniform vec3 ambient_col;
uniform vec3 light_dir;
uniform vec3 light_col;
uniform sampler2D diffuse;

void main() {
    vec3 diff = texture(diffuse, tc).rgb;
    float n_dot_l = max(0, dot(norm_wc, -light_dir));
    out_col = vec4(diff * (ambient_col + light_col * n_dot_l), 1);
}
