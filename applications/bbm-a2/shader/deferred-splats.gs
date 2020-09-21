#version 150
layout(points) in;
layout(triangle_strip, max_vertices=4) out;
flat in int time[1];
out vec4 particle_pos;
flat out int particle_time;
uniform float radius;
uniform mat4 view;
uniform mat4 proj;

void main() {
    particle_pos = gl_in[0].gl_Position;
    particle_time = time[0];
    vec4 pos_ec = view * particle_pos;
    vec4 offset = vec4(radius * normalize(-pos_ec.xyz), 0);
    gl_Position = proj * (pos_ec + offset + vec4(radius, radius, 0, 0));
    EmitVertex();
    gl_Position = proj * (pos_ec + offset + vec4(-radius, radius, 0, 0));
    EmitVertex();
    gl_Position = proj * (pos_ec + offset + vec4(radius, -radius, 0, 0));
    EmitVertex();
    gl_Position = proj * (pos_ec + offset + vec4(-radius, -radius, 0, 0));
    EmitVertex();
    EndPrimitive();
}
