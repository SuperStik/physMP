struct vertdata {
	float2 pos;
};

struct fragdata {
	float4 pos [[position]];
	float4 color;
};

vertex
fragdata vertLevel(uint vertexID [[vertex_id]], constant vertdata *verts
		[[buffer(0)]]) {
	float4 pos = float4(verts[vertexID].pos, 0.0f, 1.0f);
	return {pos, pos};
}

fragment
float4 fragLevel(fragdata frag [[stage_in]]) {
	return frag.color;
}
