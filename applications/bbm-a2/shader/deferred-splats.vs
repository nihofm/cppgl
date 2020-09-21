#version 130
in vec3 in_pos;
in int in_lifetime;
flat out int time;

void main() {
    time = in_lifetime;
    gl_Position = in_lifetime > 0 ? vec4(in_pos, 1) : vec4(9999999);
}
