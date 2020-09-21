#version 130
in vec2 tc;
out vec4 out_col;
uniform sampler2D diffuse;
void main() {
    out_col = texture(diffuse, tc);
}
