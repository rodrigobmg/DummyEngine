R"(#version 310 es
#extension GL_ARB_texture_multisample : enable
#ifdef GL_ES
    precision mediump float;
#endif

)" __ADDITIONAL_DEFINES_STR__ R"(

#define REN_OIT_DISABLED            )" AS_STR(REN_OIT_DISABLED) R"(
#define REN_OIT_MOMENT_BASED        )" AS_STR(REN_OIT_MOMENT_BASED) R"(
#define REN_OIT_WEIGHTED_BLENDED    )" AS_STR(REN_OIT_WEIGHTED_BLENDED) R"(

#define REN_OIT_MOMENT_RENORMALIZE  )" AS_STR(REN_OIT_MOMENT_RENORMALIZE) R"(

#define REN_OIT_MODE )" AS_STR(REN_OIT_MODE) R"(

layout(binding = )" AS_STR(REN_BASE0_TEX_SLOT) R"() uniform mediump
#if defined(MSAA_4)
    sampler2DMS
#else
    sampler2D
#endif
    s_accum_texture;

layout(binding = )" AS_STR(REN_BASE1_TEX_SLOT) R"() uniform mediump sampler2D s_additional_texture;

in vec2 aVertexUVs_;

out vec4 outColor;

void main() {
    ivec2 icoord = ivec2(aVertexUVs_);

#if defined(MSAA_4)
    vec4 accum = 0.25 * (texelFetch(s_accum_texture, icoord, 0) +
                         texelFetch(s_accum_texture, icoord, 1) +
                         texelFetch(s_accum_texture, icoord, 2) +
                         texelFetch(s_accum_texture, icoord, 3));
#else
    vec4 accum = texelFetch(s_accum_texture, icoord, 0);
#endif

#if (REN_OIT_MODE == REN_OIT_MOMENT_BASED)
    float b0 = texelFetch(s_additional_texture, icoord, 0).x;

#if REN_OIT_MOMENT_RENORMALIZE
    accum.xyz /= clamp(accum.w, 1e-5, 5e4);
#endif

    float k = exp(-b0);
    outColor = vec4(accum.xyz, k);
#elif (REN_OIT_MODE == REN_OIT_WEIGHTED_BLENDED)
    float revealage = texelFetch(s_additional_texture, icoord, 0).x;

    outColor = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), revealage);
#endif
}
)"
