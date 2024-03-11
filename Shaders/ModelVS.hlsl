/*===================================================================================
*    Date : 2023/05/23(âŒ)
*        Author		: Gakuto.S
*        File		: SpriteVS.hlsl
*        Detail		: 
===================================================================================*/
struct VS_INPUT
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float2 texcoord0    : TEXCOORD0;
    float2 texcoord1    : TEXCOORD1;
    float4 joint        : JOINT;
    float4 weight       : WEIGHT;
    float4 color        : COLOR;
};

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

cbuffer MATRICES : register(b1)
{
    matrix  local;
    matrix  jointMatrix[128];
    float   jointCount;
    float3  dummy;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
    matrix viewProjection = mul(view, projection); // view projection matrix
    
    float4 worldPosition;
    float4 worldNormal;
    
    if (jointCount > 0.0)
    {
        matrix skinMatrix =
            input.weight.x * jointMatrix[int(input.joint.x)] +
            input.weight.y * jointMatrix[int(input.joint.y)] +
            input.weight.z * jointMatrix[int(input.joint.z)] +
            input.weight.w * jointMatrix[int(input.joint.w)];

        matrix nodeMatrix   = mul(skinMatrix, local);
        nodeMatrix          = mul(nodeMatrix, world);
        float4 position     = float4(input.position, 1.0f);
        worldPosition       = mul(position, nodeMatrix);
        
        float3x3 loc = float3x3(local._11_21_31, local._12_22_32, local._13_23_33);
        float3x3 wor = float3x3(world._11_21_31, world._12_22_32, world._13_23_33);
        float3x3 ski = float3x3(skinMatrix._11_21_31, skinMatrix._12_22_32, skinMatrix._13_23_33);
        
        float3x3 mat = mul(mul(ski, loc), wor);
        mat = transpose(mat);
        worldNormal.xyz = mul(input.normal.xyz, mat);
        worldNormal.w = 1.0;
    }
    else
    {
        matrix nodeMatrix = mul(local, world);
        float4 position = float4(input.position, 1.0f);
        worldPosition = mul(position, nodeMatrix);
        
        float3x3 loc = float3x3(local._11_21_31, local._12_22_32, local._13_23_33);
        float3x3 wor = float3x3(world._11_21_31, world._12_22_32, world._13_23_33);
        
        float3x3 mat = mul(wor, loc);
        mat = transpose(mat);
        worldNormal.xyz = mul(input.normal.xyz, mat);
        worldNormal.w = 1.0;
    }
    
    // ç≈èIìIÇ»åãâ 
    output.position = mul(worldPosition, viewProjection);
    
    output.worldPosition    = worldPosition.xyz;
    output.normal           = normalize(worldNormal);
    output.texcoord0        = input.texcoord0;
    output.texcoord1        = input.texcoord1;
    output.color            = input.color;
    
    return output;
}