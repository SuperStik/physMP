#include <metal_common>
#include <metal_texture>

using namespace metal;

struct fragdata {
	float4 position [[position]];
	float2 texcoords;
};

vertex
fragdata vertScreen(float2 position [[attribute(0)]] [[stage_in]]) {
	float2 texcoords = saturate(position);
	texcoords.y = 1.0f - texcoords.y;
	return {float4(position, 0.0f, 1.0f), texcoords};
}

[[early_fragment_tests]]
fragment
half4 fragScreen(fragdata frag [[stage_in]], texture2d_array<half> tex
	[[texture(0)]]) {
	return tex.sample(sampler(), frag.texcoords, 0);
}
