#include <metal_matrix>

using namespace metal;

struct matrixdata {
	float4x4 view;
	float4x4 persp;
};

struct vertdata {
	packed_float3 pos;
};

struct fragdata {
	float4 pos [[position]];
};

vertex
fragdata vertObject(uint vertexID [[vertex_id]], constant matrixdata *mats
		[[buffer(0)]], constant vertdata *verts [[buffer(15)]], constant
		float4x4 *model [[buffer(2)]]) {
	float4 pos = float4(verts[vertexID].pos, 1.0f);
	float4 endpos = mats->persp * mats->view * *model * pos;
	return {endpos};
}

fragment
half4 fragObject(fragdata frag [[stage_in]]) {
	return half4(0.2h, 1.0h, 0.3h, 1.0h);
}
