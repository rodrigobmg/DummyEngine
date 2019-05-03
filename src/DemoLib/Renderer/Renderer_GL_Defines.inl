
#define _AS_STR(x) #x
#define AS_STR(x) _AS_STR(x)

// Resolution of frustum item grid
#define REN_GRID_RES_X 16
#define REN_GRID_RES_Y 8
#define REN_GRID_RES_Z 24

// Attribute location
#define REN_VTX_POS_LOC 0
#define REN_VTX_NOR_LOC 1
#define REN_VTX_TAN_LOC 2
#define REN_VTX_UV1_LOC 3
#define REN_VTX_UV2_LOC 4

// Texture binding
#define REN_DIFF_TEX_SLOT 0
#define REN_NORM_TEX_SLOT 1
#define REN_SPEC_TEX_SLOT 2
#define REN_SHAD_TEX_SLOT 3
#define REN_LMAP_SH_SLOT 4
#define REN_DECAL_TEX_SLOT 8
#define REN_SSAO_TEX_SLOT 9
#define REN_LIGHT_BUF_SLOT 10
#define REN_DECAL_BUF_SLOT 11
#define REN_CELLS_BUF_SLOT 12
#define REN_ITEMS_BUF_SLOT  13
#define REN_INST_BUF_SLOT 14

#define REN_SSR_DEPTH_TEX_SLOT 0
#define REN_SSR_NORM_TEX_SLOT 1
#define REN_SSR_SPEC_TEX_SLOT 2
#define REN_SSR_PREV_TEX_SLOT 3
#define REN_SSR_ENV_TEX_SLOT 4

#define REN_U_M_MATRIX_LOC 0
#define REN_U_INSTANCES_LOC 1

#define REN_UB_SHARED_DATA_LOC 0

// Shader output location
#define REN_OUT_COLOR_INDEX 0
#define REN_OUT_NORM_INDEX 1
#define REN_OUT_SPEC_INDEX 2

// Shadow resolution
#define REN_SHAD_RES_PC 4096
#define REN_SHAD_RES_ANDROID 4096

// Shadow cascades definition
#define REN_SHAD_CASCADE0_DIST 8.0
#define REN_SHAD_CASCADE0_SAMPLES 16
#define REN_SHAD_CASCADE1_DIST 24.0
#define REN_SHAD_CASCADE1_SAMPLES 8
#define REN_SHAD_CASCADE2_DIST 56.0
#define REN_SHAD_CASCADE2_SAMPLES 4
#define REN_SHAD_CASCADE3_DIST 120.0
#define REN_SHAD_CASCADE3_SAMPLES 4

#define REN_SSAO_BUF_RES_DIV 2

#define REN_MAX_BATCH_SIZE 8

#define REN_MAX_INSTANCES_TOTAL 262144
#define REN_MAX_SHADOWMAPS_TOTAL 32
#define REN_MAX_PROBES_TOTAL 32