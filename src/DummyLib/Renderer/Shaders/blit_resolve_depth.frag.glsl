R"(#version 310 es
#extension GL_ARB_texture_multisample : enable

#ifdef GL_ES
    precision mediump float;
#endif

layout(binding = )" AS_STR(REN_BASE0_TEX_SLOT) R"() uniform highp sampler2DMS depth_texture;

in vec2 aVertexUVs_;

void main() {
    highp ivec2 coord = ivec2(aVertexUVs_);

    highp float d1 = texelFetch(depth_texture, coord, 0).r;
    highp float d2 = texelFetch(depth_texture, coord, 1).r;
    highp float d3 = texelFetch(depth_texture, coord, 2).r;
    highp float d4 = texelFetch(depth_texture, coord, 3).r;

    highp float max_depth = max(max(d1, d2), max(d3, d4));
    gl_FragDepth = max_depth;
}
)"