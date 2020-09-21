#version 130
in vec2 tc;
out vec4 out_col;
uniform sampler2D depth;
uniform sampler2D col;
void main() {
    gl_FragDepth = texture(depth, tc).r;
    out_col = texture(col, tc);
}
