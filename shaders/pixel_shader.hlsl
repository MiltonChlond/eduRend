
Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
TextureCube skyBox : register(t2);

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
    int IsSkyBox;
    float3 padding;
};

SamplerState samp : register(s0);

struct PSIn
{
	float4 Pos  : SV_Position;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Binormal : BINORMAL;
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
    
    //float3 N = normalize(input.Normal);
    
    if(IsSkyBox == 1)
    {
        float3 vectorDirection = normalize(input.WorldPos - camPos.xyz);
        return float4(skyBox.Sample(samp, vectorDirection).rgb, 1.0f);
    }
    
    float4 normalTex = texNormal.Sample(samp, input.TexCoord);
    float3 NM = normalTex.rgb * 2.0f - 1.0f;
    
    float3 T = normalize(input.Tangent);
    float3 B = normalize(input.Binormal);
    float3 N = normalize(input.Normal);
    float3x3 TBN = float3x3(T, B, N);
    
    N = normalize(mul(NM, TBN));
    //N = normalize(input.Normal);
    //N = normalize(N);
    if(normalTex.a < 1)
    {
        N = input.Normal;
    }
    
    float3 L = normalize(lightPos.xyz - input.WorldPos);
    float3 V = normalize(camPos.xyz - input.WorldPos);
    float3 R = reflect(-L, N); 
    
    float3 dir = normalize(reflect(-V, N));
    float3 refColor = skyBox.Sample(samp, dir).rgb;
    
    //return float4(refColor, 1.0f);
    
    float3 texColor = texDiffuse.Sample(samp, input.TexCoord).rgb;
    float3 ambient = ambientColor.rgb * texColor;

    float diff = saturate(dot(N, L));
    float3 diffuse = texColor * diff;

    float shininess = specularColor.w;
    float spec = pow(saturate(dot(V, R)), shininess);
    float3 specular = specularColor.rgb * spec;

    float3 finalColor = ambient + diffuse + specular + (refColor * 0.1f);

    return float4(finalColor, 1.0);
    
	// Debug shading #2: map and return texture coordinates as a color (blue = 0)
//	return float4(input.TexCoord, 0, 1);
}