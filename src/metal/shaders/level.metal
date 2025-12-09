#include <metal_matrix>

using namespace metal;

struct matrixdata {
	float4x4 view;
	float4x4 persp;
};

struct fragdata {
	float4 pos [[position]];
	float4 color;
};

vertex
fragdata vertLevel(uint vertexID [[vertex_id]], constant matrixdata *mats
		[[buffer(0)]], float3 position [[attribute(0)]] [[stage_in]]) {
	float4 pos = float4(position, 1.0f);
	float4 endpos = mats->persp * mats->view * pos;
	return {endpos, pos};
}

[[early_fragment_tests]]
fragment
float4 fragLevel(fragdata frag [[stage_in]]) {
	return frag.color;
}
