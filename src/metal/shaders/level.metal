struct vertdata {
	float2 pos;
};

struct fragdata {
	float4 pos [[position]];
};

vertex
fragdata vertLevel(uint vertexID [[vertex_id]], constant vertdata *verts [[buffer(0)]]) {
	return {float4(verts[vertexID].pos, 0.0f, 1.0f)};
}

fragment
float4 fragLevel(constant fragdata *frag) {
	return frag->pos;
}
