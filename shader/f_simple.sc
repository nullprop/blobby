#include <bgfx_shader.sh>

// time, 0, 0, 0
uniform vec4 u_globals;

const int MAX_SDF = 16;
uniform vec4 u_positions[MAX_SDF];

const int MAX_STEPS = 100;
const float MAX_DIST = 100.0;
const float SURF_DIST = 0.01;
const float SMOOTHNESS = 1.0;
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

    for (int i = 0; i < MAX_SDF; i++)
    {
        field = smooth_union(
            field,
            sdf_sphere(
                p,
                u_positions[i].xyz,
                0.5
            ),
            SMOOTHNESS
        );
    }

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

vec3 pixel_direction()
{
    vec4 near = vec4(
        2.0 * (((gl_FragCoord.x - u_viewRect[0]) / u_viewRect[2]) - 0.5),
        2.0 * (((gl_FragCoord.y - u_viewRect[1]) / u_viewRect[3]) - 0.5),
        0.0,
        1.0
    );
    near = mul(u_invViewProj, near);
    vec4 far = near + u_invViewProj[2];
    near.xyz /= near.w;
    far.xyz /= far.w;
    return normalize(far.xyz - near.xyz);
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
