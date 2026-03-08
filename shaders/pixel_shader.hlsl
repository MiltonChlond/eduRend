
Texture2D texDiffuse : register(t0);

cbuffer LightCameraBuffer : register(b2)
{
    float4 lightPos;
    float4 camPos;
};

cbuffer MaterialBuffer : register(b1)
{
    float4 ambientColor;
    float4 diffuseColor;
    float4 specularColor;
};

SamplerState samp : register(s0);

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float2 TexCoord : TEX;
    float3 WorldPos : WORLDPOS;
};

//-----------------------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------------------

float4 PS_main(PSIn input) : SV_Target
{
	// Debug shading #1: map and return normal as a color, i.e. from [-1,1]->[0,1] per component
	// The 4:th component is opacity and should be = 1
    
    //float3 N = normalize(input.Normal);
    //float3 L = normalize(lightPos.xyz - input.WorldPos);

    //float NdotL = saturate(dot(N, L));

    //return float4(NdotL.xxx, 1.0);
    
    float3 N = normalize(input.Normal);
    float3 L = normalize(lightPos.xyz - input.WorldPos);
    float3 V = normalize(camPos.xyz - input.WorldPos);
    float3 R = reflect(-L, N);

    float3 texColor = texDiffuse.Sample(samp, input.TexCoord).rgb;
    float3 ambient = ambientColor.rgb * texColor;

    float diff = saturate(dot(N, L));
    float3 diffuse = texColor * diff;

    float shininess = specularColor.w;
    float spec = pow(saturate(dot(V, R)), shininess);
    float3 specular = specularColor.rgb * spec;

    float3 finalColor = ambient + diffuse + specular;

    return float4(finalColor, 1.0);
    
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
//	return float4(input.TexCoord, 0, 1);
}