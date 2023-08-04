$input v_position

#include <bgfx_shader.sh>

// time, 0, 0, 0
uniform vec4 u_globals;

const int MAX_STEPS = 100;
const float MAX_DIST = 100.0;
const float SURF_DIST = 0.01;

const int SHAPES = 6;
const float MAX_SPREAD_H = 0.1;
const float MAX_HEIGHT = 2.0;
const float RISE_SPEED = 0.4;
const float MIN_SPEED = 0.15;
const float LAVA_SMOOTHNESS = 0.5;

const float HEIGHT_GRADIENT = 0.2;
const float RIM_GRADIENT = 0.6;

// https://iquilezles.org/articles/palettes/
// http://dev.thi.ng/gradients/
vec3 palette(float t) {
    vec3 a = vec3(0.500, 0.500, 0.000);
    vec3 b = vec3(0.500, 0.500, 0.000);
    vec3 c = vec3(0.100, 0.500, 0.000);
    vec3 d = vec3(0.000, 0.000, 0.000);

    return a + b*cos(6.28318*(c*t+d));
}

float sdf_sphere(vec3 p, vec3 sp, float r)
{
    return length(sp - p) - r;
}

float smooth_union(float d1, float d2, float k)
{
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float get_dist(vec3 p)
{
    float field = MAX_DIST;

    // random blobs
    for (int i = 0; i < SHAPES; i++)
    {
        float seed1 = rand(vec2((i+1)*1.15));
        float seed2 = rand(vec2((i+1)*1.25));
        float seed3 = rand(vec2((i+1)*1.35));
        field = smooth_union(
            field,
            sdf_sphere(
                p,
                vec3(
                    (seed1 - 0.5) * MAX_SPREAD_H,
                    MAX_HEIGHT * sin(u_globals[0] * max(seed2, MIN_SPEED) * RISE_SPEED) - 0.5,
                    (seed3 - 0.5) * MAX_SPREAD_H
                ),
                0.2 * (1.0 - pow(seed2, 4))
            ),
            LAVA_SMOOTHNESS
        );
    }

    // big blob at the bottom
    field = smooth_union(
        field,
        sdf_sphere(
            p,
            vec3(
                0.0,
                -MAX_HEIGHT,
                0.0
            ),
            0.8
        ),
        LAVA_SMOOTHNESS
    );

    return field;
}

vec3 get_normal(vec3 p)
{
    float d = get_dist(p);
    vec2 e = vec2(0.01, 0.0);
    vec3 norm = d - vec3(
        get_dist(p - e.xyy),
        get_dist(p - e.yxy),
        get_dist(p - e.yyx)
    );
    return normalize(norm);
}

float ray_march(vec3 origin, vec3 direction)
{
    float d = 0.0;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        vec3 p = origin + direction * d;
        float sd = get_dist(p);
        d += sd;
        if (d > MAX_DIST || sd < SURF_DIST)
        {
            break;
        }
    }
    return d;
}

void main() {
    vec4 cam_pos = mul(u_invView, vec4(1.0, 1.0, 1.0, 1.0));
    // vec4 cam_dir = u_invViewProj * vec4(0, 0, 1.0, 1.0);
    // cam_dir.xyz /= camDirection.w;
    vec3 cam_dir = normalize(v_position - cam_pos.xyz);
    float d = ray_march(cam_pos.xyz, cam_dir.xyz);

    float alpha = step(d, MAX_DIST);

    vec3 ray = cam_pos.xyz + cam_dir.xyz * d;
    vec3 n = get_normal(ray);

    // color gradient
    float h = 1.0 - (ray.y * MAX_HEIGHT * HEIGHT_GRADIENT);
    float f = dot(-cam_dir.xyz, n) * RIM_GRADIENT;
    float g = f * h;

    vec3 col = palette(1.0 - g) * alpha;

    gl_FragColor = vec4(col, 1.0);
}
