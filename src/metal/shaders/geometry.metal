#include <metal_matrix>

using namespace metal;

struct matrixdata {
	float4x4 view;
	float4x4 persp;
};

struct model {
	float4x4 model;
	float3x3 normal;
};

struct vertdata {
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
};

struct fragdata {
	float4 position [[position]];
	half3 normal;
};

struct outdata {
	half4 albedo_specular [[color(0)]];
	half4 normal_shadow [[color(1)]];
};

vertex
fragdata vertGeometry(constant matrixdata *mats [[buffer(0)]], constant
		model *m [[buffer(1)]], vertdata vert [[stage_in]]) {
	model mdl = *m;

	float4 pos = mdl.model * float4(vert.position, 1.0f);
	float4 endpos = mats->persp * mats->view * pos;

	half3 normal = normalize(half3(mdl.normal * vert.normal));

	return {endpos, normal};
}

[[early_fragment_tests]]
fragment
outdata fragGeometry(fragdata frag [[stage_in]]) {
	outdata out;
	out.albedo_specular = half4(0.2h, 0.8h, 0.1h, 1.0h);
	out.normal_shadow = half4(frag.normal, 1.0h);

	return out;
}
