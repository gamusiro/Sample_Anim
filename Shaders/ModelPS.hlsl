/*===================================================================================
*    Date : 2023/05/23(火)
*        Author		: Gakuto.S
*        File		: SpritePS.hlsl
*        Detail		: 
===================================================================================*/
#define MANUAL_SRGB

struct PS_INPUT
{
    float4 position     : SV_Position;
    float3 worldPosition: POSITION;
    float4 normal       : NORMAL;
    float2 texcoord0    : TEXCOORD0;
    float2 texcoord1    : TEXCOORD1;
    float4 color        : COLOR;
};

cbuffer MATRICES : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 camPos;
};

///マテリアル構造体
cbuffer MATERIAL : register(b1)
{
    int     alphaMode;
    float   alphaCutoff;
    float   metallicFactor;
    float   roughnessFactor;
    float4  baseColorFactor;
    float4  emissiveFactor;
    int     baseColorIndex;
    int     metallicRoughnessIndex;
    int     normalIndex;
    int     occulusionIndex;
    int     emissiveIndex;
    int     doubleSided;    // 0 or 1
    int     baseColorTexCoord;
    int     metallicRoughnessTexCoord;
    int     normalTexCoord;
    int     occlusionTexCoord;
    int     emissiveTexCoord;
    float   dummy;
};

struct PBRInfo
{
    float   NdotL;
    float   NdotV;
    float   NdotH;
    float   LdotH;
    float   VdotH;
    float   perceptualRoughness;
    float   metalness;
    float3  reflectance0;
    float3  reflectance90;
    float   alphaRoughness;
    float3  diffuseColor;
    float3  specularColor;
};


// 通常のテクスチャ
Texture2D g_baseColorTexture            : register(t0);
Texture2D g_metallicRoughnessTexture    : register(t1);
Texture2D g_normalTexture               : register(t2);
Texture2D g_occlusionTexture            : register(t3);
Texture2D g_emissiveTexture             : register(t4);

// 拡張テクスチャ
Texture2D g_specularGlossinessTexture   : register(t5);
Texture2D g_diffuseTexture              : register(t6);

// 通常のサンプラー
SamplerState g_baseColorSampler         : register(s0);
SamplerState g_metallicRoughnessSampler : register(s1);
SamplerState g_normalSampler            : register(s2);
SamplerState g_occlusionSampler         : register(s3);
SamplerState g_emissiveSampler          : register(s4);

// 定数データ
static const float M_PI = 3.141592653589793;
static const float c_MinRoughness = 0.04;
static const float3 lightDirect = float3(0.0, 1.0, 0.0);
static const float3 lightColor = float3(1.0, 1.0, 1.0);

// パラメータだけど定数で置く
static const float exposure = 4.5;
static const float gamma = 2.2;


// アンチャーテッド2で使用されていたトーンマッピング
float3 Uncharted2Tonemap(float3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

// トーンマッピング
float4 tonemap(float4 color)
{
    float3 outcol = Uncharted2Tonemap(color.rgb * exposure);
    outcol = outcol * (1.0f / Uncharted2Tonemap(float3(11.2, 11.2, 11.2)));
    return float4(pow(outcol, float3(1.0f / gamma, 1.0 / gamma, 1.0 / gamma)), color.a);
}

// 線形カラーに変換
float4 SRGBtoLINEAR(float4 srgbIn)
{
#ifdef MANUAL_SRGB
#ifdef SRGB_FAST_APPROXIMATION
	float3 linOut = pow(srgbIn.xyz, float3(2.2, 2.2, 2.2));
#else //SRGB_FAST_APPROXIMATION
    float3 bLess = step(float3(0.04045, 0.04045, 0.04045), srgbIn.xyz);
    float3 linOut = lerp(srgbIn.xyz / float3(12.92, 12.92, 12.92),
                    pow((srgbIn.xyz + float3(0.055, 0.055, 0.055)) / float3(1.055, 1.055, 1.055),
                    float3(2.4, 2.4, 2.4)), bLess);
#endif //SRGB_FAST_APPROXIMATION
	return float4(linOut,srgbIn.w);
#else //MANUAL_SRGB
    return srgbIn;
#endif //MANUAL_SRGB
}

// 法線データの取得
float3 GetNormal(PS_INPUT input)
{
    float3 tangentNormal = g_normalTexture.Sample(g_normalSampler, normalTexCoord == 0 ? input.texcoord0 : input.texcoord1).rgb * 2.0 - 1.0;
    
    float3 q1   = ddx(input.worldPosition.xyz);
    float3 q2   = ddy(input.worldPosition.xyz);
    float2 st1  = ddx(input.texcoord0);
    float2 st2  = ddy(input.texcoord0);
    
    float3 N = normalize(input.normal);
    float3 T = normalize(q1 * st2.y - q2 * st1.y);
    float3 B = -normalize(cross(T, N));
    
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(tangentNormal, TBN));
}


float3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

// フレネル反射
float3 specularReflection(PBRInfo pbrInputs)
{
    return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}


float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

float4 main(PS_INPUT input) : SV_Target
{
    float4 outColor = float4(0.0, 0.0, 0.0, 1.0);
    
    float perceptualRoughness;
    float metallic;
    float3 diffuseColor;
    float4 baseColor;

    float3 f0 = float3(0.04, 0.04, 0.04);
    
    // アルファマスク処理
    if (alphaMode == 1.0f)
    {
        if (baseColorIndex > -1)
            baseColor = SRGBtoLINEAR(g_baseColorTexture.Sample(g_baseColorSampler, baseColorTexCoord == 0 ? input.texcoord0 : input.texcoord1)) * baseColorFactor;
        else
            baseColor = baseColorFactor;
        if (baseColor.a < alphaCutoff)
            discard;
    }
    
    // 物理ベースレンダリング
    perceptualRoughness = roughnessFactor;
    metallic            = metallicFactor;
    
    if (metallicRoughnessIndex > -1)
    {
        float4 mrSample = g_metallicRoughnessTexture.Sample(g_metallicRoughnessSampler, metallicRoughnessTexCoord == 0 ? input.texcoord0 : input.texcoord1);
        perceptualRoughness = mrSample.g * perceptualRoughness;
        metallic = mrSample.b * metallic;
    }
    else
    {
        perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
        metallic = clamp(metallic, 0.0, 1.0);
    }
    
    if (baseColorIndex > -1)
        baseColor = SRGBtoLINEAR(g_baseColorTexture.Sample(g_baseColorSampler, baseColorTexCoord == 0 ? input.texcoord0 : input.texcoord1)) * baseColorFactor;
    else
        baseColor = baseColorFactor;
    
    baseColor *= input.color;
    diffuseColor = baseColor.rgb * (float3(1.0, 1.0, 1.0) - f0);
    diffuseColor *= 1.0 - metallic;
    
    float   alphaRoughness = perceptualRoughness * perceptualRoughness;
    float3  specularColor = lerp(f0, baseColor.rgb, metallic);
    float   reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float   reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    float3  specularEnvironmentR0 = specularColor.rgb;
    float3  specularEnvironmentR90 = float3(1.0, 1.0, 1.0) * reflectance90;
    
    float3 n = (normalIndex > -1) ? GetNormal(input) : normalize(input.normal.xyz);
    float3 v = normalize(camPos.xyz - input.worldPosition); // Vector from surface point to camera
    float3 l = normalize(lightDirect);                      // Vector from surface point to light
    float3 h = normalize(l + v);                            // Half vector between both l and v
    float3 reflection = -normalize(reflect(v, n));
    
    
    float NdotL = clamp(dot(n, l), 0.001, 1.0);
    float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
    float NdotH = clamp(dot(n, h), 0.0, 1.0);
    float LdotH = clamp(dot(l, h), 0.0, 1.0);
    float VdotH = clamp(dot(v, h), 0.0, 1.0);
    
    
    PBRInfo pbrInputs;
    pbrInputs.NdotL                 = NdotL;
    pbrInputs.NdotV                 = NdotV;
    pbrInputs.NdotH                 = NdotH;
    pbrInputs.LdotH                 = LdotH;
    pbrInputs.VdotH                 = VdotH;
    pbrInputs.perceptualRoughness   = perceptualRoughness;
    pbrInputs.metalness             = metallic;
    pbrInputs.reflectance0          = specularEnvironmentR0;
    pbrInputs.reflectance90         = specularEnvironmentR90;
    pbrInputs.alphaRoughness        = alphaRoughness;
    pbrInputs.diffuseColor          = diffuseColor;
    pbrInputs.specularColor         = specularColor;
    
    
    float3  F = specularReflection(pbrInputs);
    float   G = geometricOcclusion(pbrInputs);
    float   D = microfacetDistribution(pbrInputs);
    
    float3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    float3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
    
    float3 color = NdotL * lightColor * (diffuseContrib + specContrib);
    color += (pbrInputs.diffuseColor * 0.7) + (pbrInputs.specularColor * 0.5);
    
    
    const float u_OcclusionStrength = 1.0f;
    if (occulusionIndex > -1)
    {
        float ao = g_occlusionTexture.Sample(g_occlusionSampler, occlusionTexCoord == 0 ? input.texcoord0 : input.texcoord1).r;
        color = lerp(color, color * ao, u_OcclusionStrength);
    }

    // エミッシブ(発光の加算)
    const float u_EmissiveFactor = 1.0f;
    if (emissiveIndex > -1)
    {
        float3 emissive = SRGBtoLINEAR(g_emissiveTexture.Sample(g_emissiveSampler, occlusionTexCoord == 0 ? input.texcoord0 : input.texcoord1)).rgb * u_EmissiveFactor;
        color += emissive;
    }
	
    outColor = float4(color, baseColor.a);
    
    return outColor;
}