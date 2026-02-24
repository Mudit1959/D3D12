#include "ShaderInclude.hlsli"

//Texture2D AllTextures[ ] : register(t0, space0);

cbuffer PSConstants : register(b0)
{
    unsigned int albedoIndex;
    unsigned int normalIndex;
    unsigned int roughnessIndex;
    unsigned int metalnessIndex;

    float2 UVScale;
    float2 UVOffset;

    float3 cameraWorldPos;
    int numLights;

    Light lights[MAX_LIGHTS];
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
    Texture2D Albedo = ResourceDescriptorHeap[albedoIndex];
    
    /*
    Texture2D Albedo = AllTextures[albedoIndex];
    Texture2D NormalMap = AllTextures[NormalMap];
    Texture2D RoughnessMap = AllTextures[roughnessIndex];
    Texture2D MetalnessMap = AllTextures[metalnessIndex];
    
    // -- RENORMALIZING AND SCALING NORMALS AND UV --
    input.uv = input.uv * scale + offset;
    input.normal = normalize(input.normal);
    
    float3 total = float3(0.0f, 0.0f, 0.0f);
    
    // -- GAMMA CORRECT SURFACE(ALBEDO) COLOR --
    float3 surfaceColor = Albedo.Sample(BasicSampler, input.uv).rgb;
    surfaceColor = pow(surfaceColor, 2.2); // Remove gamma correction from texture
    
    // -- SAMPLING ROUGHNESS AND METALNESS(f0) --
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    float3 f0 = lerp(0.04, surfaceColor.rgb, metalness);
    //return float4(metalness, 0.0f, 0.0f, 1.0f);
    
    
    // -- SAMPLE NORMAL MAP, CHANGE NORMALS TO ACCOUNT FOR SURFACE UNEVENENESS -- 
    float3 finalNormal;
    finalNormal = normalize(NormalMap.Sample(BasicSampler, input.uv).xyz * 2.0f - 1.0f); // First unpack the normal map's normal
    
    float3 T, B, N;
    N = input.normal;
    T = normalize(input.tangent - dot(input.tangent, N) * N);
    B = cross(T, N);
    
    float3x3 TBN = float3x3(T, B, N);
    finalNormal = mul(finalNormal, TBN);
    input.normal = finalNormal; // Reassigned incoming data to hold new normal
    
    // -- OTHER VARIABLES
    float3 toCamera, halfVector, toLight, add;
    toCamera = normalize(cameraWorldPos - input.worldPos);
    
    for (int i = 0; i < 5; i++)
    {
        
        switch (lights[i].Type)
        {
            
            case LIGHT_TYPE_DIRECTIONAL: //0
                toLight = normalize(-lights[i].Direction);
                halfVector = (toLight + toCamera) / 2;
                
                add = DiffuseEnergyConserve(DiffuseLambertPBR(input.normal, toLight, surfaceColor),
                                                Fresnel(toCamera, halfVector, f0),
                                                metalness);
                add += CookTorranceBRDF(toLight, toCamera, halfVector, input.normal, roughness, f0);
            
                add *= lights[i].Color * lights[i].Intensity;
                
                total += add;
                break;
            
            case LIGHT_TYPE_POINT: //1
                // Point lights emit in all directions, so we will depend on the range and position of the light
                toLight = normalize(lights[i].Position - input.worldPos);
                halfVector = normalize(toLight + toCamera) / 2;
                
                add = DiffuseEnergyConserve(DiffuseLambertPBR(input.normal, toLight, surfaceColor),
                                                Fresnel(toCamera, halfVector, f0),
                                                metalness);
                add += CookTorranceBRDF(toLight, toCamera, halfVector, input.normal, roughness, f0);
            
                add *= Attenuate(lights[i], input.worldPos);
                add *= lights[i].Color * lights[i].Intensity;
            
                total += add;
            
                break;
            
            case LIGHT_TYPE_SPOT: //2
                // Spot lights emit light in a conical manner, so we will depend on range, position, and angles!
                toLight = normalize(lights[i].Position - input.worldPos);
                halfVector = normalize(toLight + toCamera) / 2;
            
                add = DiffuseEnergyConserve(DiffuseLambertPBR(input.normal, toLight, surfaceColor),
                                               Fresnel(toCamera, halfVector, f0),
                                                metalness);
                add += CookTorranceBRDF(toLight, toCamera, halfVector, input.normal, roughness, f0);
            
                
            
                float surfaceCos = saturate(dot(-toLight, lights[i].Direction));
                float cosOuter = cos(lights[i].SpotOuterAngle);
                float cosInner = cos(lights[i].SpotInnerAngle);
                float fallOff = cosOuter - cosInner;
            
                float spotTerm = saturate((cosOuter - surfaceCos) / fallOff);
                add *= spotTerm;
                
                add *= Attenuate(lights[i], input.worldPos);
                add *= lights[i].Color * lights[i].Intensity;
                
                total += add;
                
                break;
            
        }
    }
    
    total = pow(total, 1.0f / 2.2f); // Gamma correct final color
    return float4(total, 1.0f);
    */
    
    
    
    return Albedo.Sample(BasicSampler, input.uv);
}