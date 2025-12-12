#version 330 core

out vec3 rayDir;

uniform mat4 invProjection;
uniform mat4 invView;

const vec2 verts[3] = vec2[3](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main()
{
    vec2 pos = verts[gl_VertexID];
    gl_Position = vec4(pos, 0.0, 1.0);

    // reconstruct view ray from fullscreen position
    vec4 clip = vec4(pos, 1.0, 1.0);
    vec4 view = invProjection * clip;
    view /= view.w;
    view.w = 0.0;

    vec3 worldDir = normalize((invView * view).xyz);
    rayDir = worldDir;
}
