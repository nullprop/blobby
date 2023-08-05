#include <bgfx_shader.sh>
#include <common.sh>
#include <sdf.sh>

const float RIM_GRADIENT = 0.6;

// https://iquilezles.org/articles/palettes/
// http://dev.thi.ng/gradients/
vec3 palette(float t)
{
    vec3 a = vec3(0.500, 0.500, 0.000);
    vec3 b = vec3(0.500, 0.500, 0.000);
    vec3 c = vec3(0.100, 0.500, 0.000);
    vec3 d = vec3(0.000, 0.000, 0.000);

    return a + b*cos(6.28318*(c*t+d));
}

void main()
{
    vec4 eye_pos = mul(u_invView, vec4(1.0, 1.0, 1.0, 1.0));
    vec3 pixel_dir = pixel_direction();
    float d = ray_march(eye_pos.xyz, pixel_dir.xyz);

    float alpha = step(d, MAX_DIST);

    vec3 ray = eye_pos.xyz + pixel_dir.xyz * d;
    vec3 n = get_normal(ray);

    // color gradient
    float f = dot(-pixel_dir.xyz, n) * RIM_GRADIENT;

    vec3 col = palette(1.0 - f) * alpha;

    gl_FragColor = vec4(col, 1.0);
}
