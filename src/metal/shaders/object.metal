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

struct lightdata {
	packed_float3 position;
	packed_float3 ambient;
	packed_float3 diffuse;
	packed_float3 specular;
};

struct vertdata {
	float3 pos [[attribute(0)]];
	float3 normal [[attribute(1)]];
};

struct fragdata {
	float4 pos [[position]];
	float3 normal;
	float3 fragpos;
};

vertex
fragdata vertObject(constant matrixdata *mats [[buffer(0)]], constant model *mdl
		[[buffer(1)]], vertdata vert [[stage_in]]) {
	float4 pos = mdl->model * float4(vert.pos, 1.0f);
	float4 endpos = mats->persp * mats->view * pos;

	float3 normal = normalize(mdl->normal * float3(vert.normal));

	return {endpos, normal, float3(pos)};
}

[[early_fragment_tests]]
fragment
float4 fragObject(fragdata frag [[stage_in]], constant lightdata *light
		[[buffer(0)]], constant packed_float3 *viewpos [[buffer(1)]]) {
	float3 lightdir = normalize(light->position - frag.fragpos);

	float3 viewdir = normalize(*viewpos - frag.fragpos);
	float3 reflectdir = reflect(-lightdir, frag.normal);

	float3 albedo = float3(0.2f, 1.0f, 0.3f);

	float3 ambient = light->ambient * albedo;

	float diff = max(dot(frag.normal, lightdir), 0.0f);
	float3 diffuse = diff * light->diffuse * albedo;

	float spec = pow(max(dot(viewdir, reflectdir), 0.0f), 16.0f);
	float3 specular = spec * light->specular;

	float3 result = ambient + diffuse + specular;
	return float4(result, 1.0f);
	return float4(0.0f, 1.0f, 0.0f, 1.0f);
}
