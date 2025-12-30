#include <metal_matrix>

using namespace metal;

struct matrixdata {
	float4x4 view;
	float4x4 persp;
};

struct model {
	float4x4 model;
	float3x3 normal;
	packed_float3 viewpos;
};

struct lightdata {
	packed_float3 position;
	packed_half3 ambient;
	packed_half3 diffuse;
	packed_half3 specular;
};

struct vertdata {
	float3 pos [[attribute(0)]];
	float3 normal [[attribute(1)]];
};

struct fragdata {
	float4 pos [[position]];
	float3 fragpos;
	float3 viewpos;
	half3 normal;
};

vertex
fragdata vertObject(constant matrixdata *mats [[buffer(0)]], constant model *m
		[[buffer(1)]], vertdata vert [[stage_in]]) {
	model mdl = *m;

	float4 pos = mdl.model * float4(vert.pos, 1.0f);
	float4 endpos = mats->persp * mats->view * pos;

	half3 normal = normalize(half3(mdl.normal * vert.normal));

	return {endpos, pos.xyz, mdl.viewpos, normal};
}

[[early_fragment_tests]]
fragment
half4 fragObject(fragdata frag [[stage_in]], constant lightdata *light
		[[buffer(0)]], constant packed_float3 *viewpos [[buffer(1)]]) {
	half3 lightdir = normalize(half3(light->position - frag.fragpos));
	half3 viewdir = half3(normalize(frag.viewpos - frag.fragpos));
	half3 reflectdir = reflect(-lightdir, frag.normal);

	half3 albedo = half3(0.2h, 1.0h, 0.3h);

	half3 ambient = light->ambient * albedo;

	half diff = max(dot(frag.normal, lightdir), 0.0h);
	half3 diffuse = diff * light->diffuse * albedo;

	half spec = pow(max(dot(viewdir, reflectdir), 0.0h), 16.0h);
	half3 specular = spec * light->specular;

	half3 result = ambient + diffuse + specular;
	return half4(result, 1.0h);
}
