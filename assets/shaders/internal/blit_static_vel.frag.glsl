#version 310 es

#ifdef GL_ES
    precision mediump float;
#endif

/*
UNIFORM_BLOCKS
    SharedDataBlock : $ubSharedDataLoc
*/

#include "_fs_common.glsl"

layout (std140) uniform SharedDataBlock {
    SharedData shrd_data;
};

layout(binding = 0) uniform mediump sampler2D depth_texture;

in vec2 aVertexUVs_;

out vec2 outVelocity;

void main() {
    ivec2 pix_uvs = ivec2(aVertexUVs_);

    float depth = texelFetch(depth_texture, pix_uvs, 0).r;

    vec4 point_cs = vec4(aVertexUVs_.xy / shrd_data.uResAndFRes.xy, depth, 1.0);
    point_cs.xyz = 2.0 * point_cs.xyz - vec3(1.0);

    vec4 point_ws = shrd_data.uInvViewProjMatrix * point_cs;
    point_ws /= point_ws.w;

    vec4 point_prev_cs = shrd_data.uViewProjPrevMatrix * point_ws;
    point_prev_cs /= point_prev_cs.w;

    outVelocity = 0.5 * (point_cs.xy + shrd_data.uTaaInfo.xy - point_prev_cs.xy);
}
