#version 130
flat in int time;
out vec4 out_col;
uniform vec2 screenres;
uniform vec2 near_far;
uniform sampler2D depth_tex;

float linear_depth(in float depth, in float near, in float far) { return (2.0 * near) / (far + near - depth * (far - near)); }

void main() {
    float fade_pos = clamp(1 - 2 * distance(gl_PointCoord, vec2(0.5)), 0, 1);
    float fade_time = clamp(time / 500.f, 0, 1);
    out_col = pow(fade_pos * fade_time, 2) * vec4(1, 115 / 255.f, 65 / 255.f, 1);
    float frag_depth = linear_depth(gl_FragCoord.z, near_far.x, near_far.y);
    float geom_depth = linear_depth(texture(depth_tex, gl_FragCoord.xy / screenres).r, near_far.x, near_far.y);
    float fade_depth = clamp(abs(frag_depth - geom_depth) / 0.01f, 0, 1);
    out_col *= vec4(fade_depth * fade_depth);
}
