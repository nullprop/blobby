// time, num sdfs, 0, 0
uniform vec4 u_globals;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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
