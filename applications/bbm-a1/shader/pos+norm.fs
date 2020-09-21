#version 130
in vec4 pos_wc;
in vec3 norm_wc;
out vec4 out_col;
uniform vec3 color;
uniform vec3 light_dir;
uniform vec3 light_col;
void main() {
    out_col = vec4(0.,0.,0.,1.);
    float n_dot_l = max(0, dot(norm_wc, -light_dir));
    out_col += vec4(color * light_col * n_dot_l, 0.);
}
