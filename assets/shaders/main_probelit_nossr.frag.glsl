#version 310 es
#extension GL_EXT_texture_buffer : enable
#extension GL_OES_texture_buffer : enable
#extension GL_EXT_texture_cube_map_array : enable
#extension GL_ARB_shader_ballot : enable
#extension GL_ARB_shader_group_vote : enable
#extension GL_AMD_gpu_shader_half_float : enable
#extension GL_AMD_shader_ballot : enable
//#extension GL_EXT_control_flow_attributes : enable
#extension GL_GOOGLE_include_directive : enable

$ModifyWarning

#ifdef GL_ES
    precision mediump float;
    precision mediump sampler2DShadow;
#endif

#include "internal/_fs_common.glsl"

#define LIGHT_ATTEN_CUTOFF 0.004

layout(binding = REN_MAT_TEX0_SLOT) uniform sampler2D diffuse_texture;
layout(binding = REN_MAT_TEX1_SLOT) uniform sampler2D normals_texture;
layout(binding = REN_MAT_TEX2_SLOT) uniform sampler2D specular_texture;
layout(binding = REN_SHAD_TEX_SLOT) uniform sampler2DShadow shadow_texture;
layout(binding = REN_DECAL_TEX_SLOT) uniform sampler2D decals_texture;
layout(binding = REN_SSAO_TEX_SLOT) uniform sampler2D ao_texture;
layout(binding = REN_LIGHT_BUF_SLOT) uniform mediump samplerBuffer lights_buffer;
layout(binding = REN_DECAL_BUF_SLOT) uniform mediump samplerBuffer decals_buffer;
layout(binding = REN_CELLS_BUF_SLOT) uniform highp usamplerBuffer cells_buffer;
layout(binding = REN_ITEMS_BUF_SLOT) uniform highp usamplerBuffer items_buffer;

#if defined(VULKAN) || defined(GL_SPIRV)
layout (binding = 0, std140)
#else
layout (std140)
#endif
uniform SharedDataBlock {
    SharedData shrd_data;
};

#if defined(VULKAN) || defined(GL_SPIRV)
layout(location = 0) in highp vec3 aVertexPos_;
layout(location = 1) in mediump vec2 aVertexUVs_;
layout(location = 2) in mediump vec3 aVertexNormal_;
layout(location = 3) in mediump vec3 aVertexTangent_;
layout(location = 4) in highp vec3 aVertexShUVs_[4];
#else
in highp vec3 aVertexPos_;
in mediump vec2 aVertexUVs_;
in mediump vec3 aVertexNormal_;
in mediump vec3 aVertexTangent_;
in highp vec3 aVertexShUVs_[4];
#endif

layout(location = REN_OUT_COLOR_INDEX) out vec4 outColor;
layout(location = REN_OUT_NORM_INDEX) out vec4 outNormal;
layout(location = REN_OUT_SPEC_INDEX) out vec4 outSpecular;


void EvaluateLightsource(
        highp vec3 frag_pos, vec3 frag_norm, highp vec4 lpos_and_index, vec4 lcol_and_radius, vec4 ldir_and_spot, sampler2DShadow shadow_texture, out vec3 out_light) {
    highp vec3 L = lpos_and_index.xyz - frag_pos;
    highp float dist = length(L);
    L /= dist;
    
    highp float d = max(dist - lcol_and_radius.w, 0.0);
    highp float denom = d / lcol_and_radius.w + 1.0;
    highp float atten = 1.0 / (denom * denom);
    
    highp float brightness = max(lcol_and_radius.r, max(lcol_and_radius.g, lcol_and_radius.b));
    
    highp float factor = LIGHT_ATTEN_CUTOFF / brightness;
    atten = (atten - factor) / (1.0 - LIGHT_ATTEN_CUTOFF);
    atten = clamp(atten, 0.0, 1.0);
    
    float _dot1 = clamp(dot(L, frag_norm), 0.0, 1.0);
    float _dot2 = dot(L, ldir_and_spot.xyz);
    
    atten = _dot1 * atten;
    if (_dot2 > ldir_and_spot.w && (brightness * atten) > FLT_EPS) {
        int shadowreg_index = floatBitsToInt(lpos_and_index.w);
        if (shadowreg_index != -1) {
            vec4 reg_tr = shrd_data.uShadowMapRegions[shadowreg_index].transform;
            
            highp vec4 pp = shrd_data.uShadowMapRegions[shadowreg_index].clip_from_world * vec4(frag_pos, 1.0);
            pp /= pp.w;
            pp.xyz = pp.xyz * 0.5 + vec3(0.5);
            pp.xy = reg_tr.xy + pp.xy * reg_tr.zw;
            
            atten *= SampleShadowPCF5x5(shadow_texture, pp.xyz);
        }
        out_light += lcol_and_radius.rgb * atten * smoothstep(ldir_and_spot.w, ldir_and_spot.w + 0.2, _dot2);
    }
}

void main(void) {
    vec3 albedo_color = texture(diffuse_texture, aVertexUVs_).rgb;
    
    vec2 duv_dx = dFdx(aVertexUVs_), duv_dy = dFdy(aVertexUVs_);
    vec3 normal_color = texture(normals_texture, aVertexUVs_).wyz;
    vec4 specular_color = texture(specular_texture, aVertexUVs_);
    
    vec3 dp_dx = dFdx(aVertexPos_);
    vec3 dp_dy = dFdy(aVertexPos_);
    
    vec3 normal = normal_color * 2.0 - 1.0;
    normal = normalize(mat3(aVertexTangent_, cross(aVertexNormal_, aVertexTangent_), aVertexNormal_) * normal);
    
    int vgpr_cell_index = GetFragCellIndex(shrd_data.uClipInfo, shrd_data.uResAndFRes.xy);

    // Check if we can use simple path
#if defined(GL_ARB_shader_ballot) && defined(GL_ARB_shader_group_vote) && 0
    int sgpr_cell_index = readFirstInvocationARB(vgpr_cell_index);
    bool whole_wave_in_single_cell = allInvocationsARB(vgpr_cell_index == sgpr_cell_index);
#else
	const int sgpr_cell_index = -1;
    const bool whole_wave_in_single_cell = false;
#endif

    vec3 additional_light = vec3(0.0);

    if (whole_wave_in_single_cell) {
        highp uvec2 sgpr_cell_data = texelFetch(cells_buffer, sgpr_cell_index).xy;
        highp uint sgpr_item_offset = bitfieldExtract(sgpr_cell_data.x, 0, 24);
        uint sgpr_lcount = bitfieldExtract(sgpr_cell_data.x, 24, 8);
        uint sgpr_dcount = bitfieldExtract(sgpr_cell_data.y, 0, 8);
        uint sgpr_pcount = bitfieldExtract(sgpr_cell_data.y, 8, 8);

        for (uint i = 0; i < sgpr_lcount; i++) {
            highp uint item_data = texelFetch(items_buffer, int(sgpr_item_offset + i)).x;
            int li = int(bitfieldExtract(item_data, 0, 12));

            highp vec4 lpos_and_index = texelFetch(lights_buffer, li * 3 + 0);
            vec4 lcol_and_radius = texelFetch(lights_buffer, li * 3 + 1);
            vec4 ldir_and_spot = texelFetch(lights_buffer, li * 3 + 2);
            
            EvaluateLightsource(aVertexPos_, normal, lpos_and_index, lcol_and_radius, ldir_and_spot, shadow_texture, additional_light);
        }
    } else {
        highp uvec2 vgpr_cell_data = texelFetch(cells_buffer, vgpr_cell_index).xy;
        highp uint vgpr_item_offset = bitfieldExtract(vgpr_cell_data.x, 0, 24);
        uint vgpr_lcount = bitfieldExtract(vgpr_cell_data.x, 24, 8);
        uint vgpr_dcount = bitfieldExtract(vgpr_cell_data.y, 0, 8);
        uint vgpr_pcount = bitfieldExtract(vgpr_cell_data.y, 8, 8);

#if defined(GL_AMD_shader_ballot)
        uint vgpr_item_index = 0u;
        highp uint vgpr_item_data = texelFetch(items_buffer, int(vgpr_item_offset + vgpr_item_index)).x;
        int vgpr_li = int(bitfieldExtract(vgpr_item_data, 0, 12));

        while (vgpr_item_index < vgpr_lcount) {
            int sgpr_li = minInvocationsNonUniformAMD(vgpr_li);
            if (vgpr_li == sgpr_li) {
                highp vec4 lpos_and_radius = texelFetch(lights_buffer, sgpr_li * 3 + 0);
                highp vec4 lcol_and_index = texelFetch(lights_buffer, sgpr_li * 3 + 1);
                vec4 ldir_and_spot = texelFetch(lights_buffer, sgpr_li * 3 + 2);

                // early fetch next item to reduce latency
				if (++vgpr_item_index < vgpr_lcount) {
					vgpr_item_data = texelFetch(items_buffer, int(vgpr_item_offset + vgpr_item_index)).x;
					vgpr_li = int(bitfieldExtract(vgpr_item_data, 0, 12));
				}

				EvaluateLightsource(aVertexPos_, normal, lpos_and_radius, lcol_and_index, ldir_and_spot, shadow_texture, additional_light);
            }
        }
#else
        for (uint i = 0; i < vgpr_lcount; i++) {
            highp uint item_data = texelFetch(items_buffer, int(vgpr_item_offset + i)).x;
            int li = int(bitfieldExtract(item_data, 0, 12));

            highp vec4 lpos_and_radius = texelFetch(lights_buffer, li * 3 + 0);
            highp vec4 lcol_and_index = texelFetch(lights_buffer, li * 3 + 1);
            vec4 ldir_and_spot = texelFetch(lights_buffer, li * 3 + 2);
            
            EvaluateLightsource(aVertexPos_, normal, lpos_and_radius, lcol_and_index, ldir_and_spot, shadow_texture, additional_light);
        }
#endif
    }

    vec3 indirect_col = vec3(0.0);
    float total_fade = 0.0;
    
    /*for (uint i = offset_and_lcount.x; i < offset_and_lcount.x + dcount_and_pcount.y; i++) {
        highp uint item_data = texelFetch(items_buffer, int(i)).x;
        int pi = int(bitfieldExtract(item_data, 24, 8));
        
        float dist = distance(shrd_data.uProbes[pi].pos_and_radius.xyz, aVertexPos_);
        float fade = 1.0 - smoothstep(0.9, 1.0, dist / shrd_data.uProbes[pi].pos_and_radius.w);

        indirect_col += fade * EvalSHIrradiance_NonLinear(normal,
                                                          shrd_data.uProbes[pi].sh_coeffs[0],
                                                          shrd_data.uProbes[pi].sh_coeffs[1],
                                                          shrd_data.uProbes[pi].sh_coeffs[2]);
        total_fade += fade;
    }*/
    
    indirect_col /= max(total_fade, 1.0);
    indirect_col = max(indirect_col, vec3(0.0));
    
    float lambert = clamp(dot(normal, shrd_data.uSunDir.xyz), 0.0, 1.0);
    float visibility = 0.0;
    if (lambert > 0.00001) {
        //visibility = GetSunVisibility(lin_depth, shadow_texture, aVertexShUVs_);
    }
    
    vec2 ao_uvs = gl_FragCoord.xy / shrd_data.uResAndFRes.zw;
    float ambient_occlusion = textureLod(ao_texture, ao_uvs, 0.0).r;
    vec3 diffuse_color = albedo_color * (shrd_data.uSunCol.xyz * lambert * visibility +
                                         ambient_occlusion * ambient_occlusion * indirect_col +
                                         additional_light);
    
    vec3 view_ray_ws = normalize(shrd_data.uCamPosAndGamma.xyz - aVertexPos_);
    float N_dot_V = clamp(dot(normal, view_ray_ws), 0.0, 1.0);
    
    vec3 kD = 1.0 - FresnelSchlickRoughness(N_dot_V, specular_color.xyz, specular_color.a);
    
    outColor = vec4(diffuse_color * kD, 1.0);
    outNormal = vec4(normal * 0.5 + 0.5, 0.0);
    outSpecular = specular_color;

    /*if (!whole_wave_in_single_cell) {
        outColor.r += 0.02;
    } else {
       outColor.g += 0.02;
    }*/
}
