#include "ShaderInclude.hlsli"

cbuffer PSConstants : register(b0)
{
    unsigned int albedoIndex;
    unsigned int normalIndex;
    unsigned int roughnessIndex;
    unsigned int metalnessIndex;

    float2 UVScale;
    float2 UVOffset;

    float3 cameraWorldPos;
    int lights;

    Light light[MAX_LIGHTS];
}

SamplerState BasicSampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    Texture2D AlbedoTexture = ResourceDescriptorHeap[albedoIndex];
    
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    return AlbedoTexture.Sample(BasicSampler, input.uv);
}