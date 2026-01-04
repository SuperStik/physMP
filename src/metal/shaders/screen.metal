struct fragdata {
	float4 position [[position]];
};

vertex
fragdata vertScreen(float2 position [[attribute(0)]] [[stage_in]]) {
	return {float4(position, 0.0f, 1.0f)};
}

[[early_fragment_tests]]
fragment
half4 fragScreen(fragdata frag [[stage_in]]) {
	return half4(0.5h, 0.5h, 1.0h, 1.0h);
}
