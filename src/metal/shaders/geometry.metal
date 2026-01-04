struct vertdata {
	float3 position [[attribute(0)]];
};

struct fragdata {
	float4 position [[position]];
};

struct outdata {
	half4 albedo_specular [[color(0)]];
	half4 normal_shadow [[color(1)]];
};

vertex
fragdata vertGeometry(vertdata vert [[stage_in]]) {
	return {float4(vert.position, 1.0f)};
}

[[early_fragment_tests]]
fragment
outdata fragGeometry(fragdata frag [[stage_in]]) {
	return {
		half4(0.0h, 0.0h, 0.0h, 1.0h),
		half4(0.5h, 0.5h, 1.0h, 1.0h)
	};
}
