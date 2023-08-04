$input a_position
$output v_position

#include <bgfx_shader.sh>

void main() {
    vec4 vert = mul(u_model[0], vec4(a_position, 1.0));
    v_position = vert.xyz;
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
