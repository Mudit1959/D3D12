// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    
    float3 worldPos : POSITION;
    
    float3 tangent : TANGENT;
};

//Random value
float random(float2 s)
{
    return frac(sin(dot(s, float2(12.9898, 78.233))) * 43758.5453123);
}



// -- LIGHTING -- //
#define MAX_LIGHTS 128
#define MAX_SPECULAR_EXPONENT 256.0f
#define MIN_ROUGHNESS 0.0000001f
#define PI 3.1415926535897932384626433832795
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
    int Type; // Which kind of light? 0, 1, 2, 3, 4, 5 (see above)
    float3 Direction; // Directional and Spot lights need a direction

    float Range; // Point and Spot lights have a max range for attenuation
    float3 Position; // Point and Spot lights have a position in space

    float Intensity; // All lights need an intensity
    float3 Color; // All lights need a color

    float SpotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float SpotOuterAngle; // Outer cone angle (radians) – Outside this, no light!
    float3 Padding; // Purposefully padding to hit the 16-byte boundary
};

// -- PBR -- 

float3 DiffuseLambertPBR(float3 surfaceNormal, float3 toLightNormalized, float3 surfaceColor)
{
    return saturate(dot(surfaceNormal, toLightNormalized)) * surfaceColor;
}

// Trowbridge-Reitz is used to calculate the percentage of microfacets that could reflect light towards the camera
float TrowbridgeReitz(float3 surfaceNormal, float3 halfVector, float roughness) // D()
{
    roughness = roughness * roughness;
    float nonZeroR = max(roughness * roughness, MIN_ROUGHNESS);
    float NdotMSq = pow(saturate(dot(surfaceNormal, halfVector)), 2);
    
    float den = (NdotMSq * (nonZeroR - 1)) + 1;
    float denSq = den * den;
    
    return nonZeroR / (PI * denSq);
}

// Determines the reflectiveness of the material based on viewing angle and the glossiness of the material
float3 Fresnel(float3 toCamera, float3 halfVector, float3 f0) // F()
{
    float VdotH = saturate(dot(toCamera, halfVector));
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}


// Fresnel-Smith is used to determine the percentage of light not reflected by the surface itself
float G_Schlick(float3 surfaceNormal, float3 toCamera, float roughness) // G()
{
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(surfaceNormal, toCamera));
    // Note: Numerator is now 1 to cancel Cook-Torrance denominator!
    return 1 / (NdotV * (1 - k) + k);
}

// f0 - the color of the specular light - if it is a metal use the texture color
// toLight - l
// toCamera - v
// halfVector - h = (v+l)/2
// surfaceNormal - n
// roughnessSquared - alpha
float3 CookTorranceBRDF(float3 toLight, float3 toCamera, float3 halfVector, float3 surfaceNormal, float roughness, float3 f0)
{
    float3 D = TrowbridgeReitz(surfaceNormal, halfVector, roughness);
    float G = G_Schlick(surfaceNormal, toCamera, roughness);
    float3 F = Fresnel(toCamera, halfVector, f0);
    
    float3 specularResult = (D * F * G) / 4;
    return specularResult * saturate(dot(surfaceNormal, toLight));

}

// F - Fresnel Result
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}