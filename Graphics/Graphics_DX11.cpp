/*===================================================================================
*    Date : 2023/06/22(木)
*        Author		: Gakuto.S
*        File		: Graphics_DX11.h
*        Detail		:
===================================================================================*/
#include "Graphics_DX11.h"
#include "../Application.h"

void GraphicsDX11::Init(WindowWin* window)
{
    CreateDeviceAndSwapChain(window);
    CreateRenderTarget();
    CreateDepthBuffer();
    SetViewport();
}

void GraphicsDX11::Uninit()
{
    SAFE_RELEASE(m_depthStencilState);
    SAFE_RELEASE(m_depthStencilView);
    SAFE_RELEASE(m_renderTargetView);
    SAFE_RELEASE(m_swapchain);
    SAFE_RELEASE(m_context);
    SAFE_RELEASE(m_device);
}

void GraphicsDX11::Clear()
{
    float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_context->ClearRenderTargetView(m_renderTargetView, color);
    m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void GraphicsDX11::Present()
{
    m_swapchain->Present(0, 0);
}

/// <summary>
/// 頂点バッファの生成
/// </summary>
ID3D11Buffer* GraphicsDX11::CreateVertexBuffer(unsigned int perVertexSize, unsigned int vertexNum, void* data)
{
    HRESULT ret{};
    ID3D11Buffer* buffer{};
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth  = perVertexSize * vertexNum;
    desc.BindFlags  = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    desc.Usage      = D3D11_USAGE::D3D11_USAGE_DEFAULT;

    if (!data)
    {// データが動的に書き込まれる場合
        desc.Usage          = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
    }

    if (data)
    {// データがある場合は書きこんでしまう
        D3D11_SUBRESOURCE_DATA subResource{};
        subResource.pSysMem = data;
        ret = m_device->CreateBuffer(&desc, &subResource, &buffer);
    }
    else
    {// データがない場合
        ret = m_device->CreateBuffer(&desc, nullptr, &buffer);
    }
    
    if(FAILED(ret))
        return nullptr;

    return buffer;
}

/// <summary>
/// インデックスバッファの生成
/// </summary>
ID3D11Buffer* GraphicsDX11::CreateIndexBuffer(unsigned int perIndexSize, unsigned int indexNum, void* data)
{
    HRESULT ret{};
    ID3D11Buffer* buffer{};
    DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R32_UINT;

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth      = perIndexSize * indexNum;
    desc.Usage          = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags      = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA subResource{};
    subResource.pSysMem = data;
    ret = m_device->CreateBuffer(&desc, &subResource, &buffer);
    if(FAILED(ret))
        return nullptr;

    // インデックス1つ当たりのサイズ
    if(perIndexSize == sizeof(short))
        format = DXGI_FORMAT::DXGI_FORMAT_R16_UINT;

    return buffer;
}

/// <summary>
/// ブレンド設定の生成
/// </summary>
ID3D11BlendState* GraphicsDX11::CreateBlendState(const D3D11_BLEND_DESC& bDesc)
{
    ID3D11BlendState* blendState = nullptr;
    m_device->CreateBlendState(&bDesc, &blendState);
    assert(blendState);

    return blendState;
}

/// <summary>
/// ラスタライザ設定の生成
/// </summary>
ID3D11RasterizerState* GraphicsDX11::CreateRasterizerState(const D3D11_RASTERIZER_DESC& rDesc)
{
    ID3D11RasterizerState* rasterizerState = nullptr;
    m_device->CreateRasterizerState(&rDesc, &rasterizerState);
    assert(rasterizerState);

    return rasterizerState;
}

/// <summary>
/// 頂点シェーダの生成
/// </summary>
ID3D11VertexShader* GraphicsDX11::CreateVertexShader(const std::string& source)
{
    ID3D11VertexShader* buffer;
    m_device->CreateVertexShader(source.data(), source.size(), nullptr, &buffer);
    assert(buffer);

    return buffer;
}

/// <summary>
/// 入力レイアウトの生成
/// </summary>
ID3D11InputLayout* GraphicsDX11::CreateInputLayout(const std::string& source, const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputLayout)
{
    ID3D11InputLayout* buffer;
    m_device->CreateInputLayout(inputLayout.data(), UINT(inputLayout.size()), source.data(), source.size(), &buffer);
    assert(buffer);

    return buffer;
}

/// <summary>
/// ピクセルシェーダの生成
/// </summary>
ID3D11PixelShader* GraphicsDX11::CreatePixelShader(const std::string& source)
{
    ID3D11PixelShader* buffer;
    m_device->CreatePixelShader(source.data(), source.size(), nullptr, &buffer);
    assert(buffer);

    return buffer;
}

/// <summary>
/// 定数バッファの生成
/// </summary>
ID3D11Buffer* GraphicsDX11::CreateShaderResource(unsigned int size, unsigned int stride)
{
    HRESULT ret{};
    ID3D11Buffer* buffer{};
    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth              = size;
    desc.Usage                  = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
    desc.BindFlags              = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
    desc.StructureByteStride    = stride;
    desc.CPUAccessFlags         = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
    ret = m_device->CreateBuffer(&desc, nullptr, &buffer);
    assert(buffer);

    return buffer;
}

/// <summary>
/// サンプラーの生成
/// </summary>
ID3D11SamplerState* GraphicsDX11::CreateSamplerState(const D3D11_SAMPLER_DESC& sDesc)
{
    // サンプラーステートの生成
    ID3D11SamplerState* samplerState = nullptr;
    m_device->CreateSamplerState(&sDesc, &samplerState);
    assert(samplerState);

    return samplerState;
}

/// <summary>
/// テクスチャの生成
/// </summary>
ID3D11ShaderResourceView* GraphicsDX11::CreateTextureFromBuffer(const unsigned char* buffer, unsigned int width, unsigned int height)
{
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = width;
    desc.Height             = height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource{};
    subResource.pSysMem     = buffer;
    subResource.SysMemPitch = width * common::LoadTexture::k_RGBA;

    HRESULT ret{};
    ID3D11Texture2D* textureBuffer{};
    ret = m_device->CreateTexture2D(&desc, &subResource, &textureBuffer);
    if (FAILED(ret))
        return nullptr;

    ID3D11ShaderResourceView* resourceView{};
    ret = m_device->CreateShaderResourceView(textureBuffer, nullptr, &resourceView);
    if (FAILED(ret))
    {
        SAFE_RELEASE(textureBuffer);
        return nullptr;
    }

    SAFE_RELEASE(textureBuffer);
    assert(resourceView);

    return resourceView;
}

/// <summary>
/// テクスチャの生成
/// </summary>
ID3D11ShaderResourceView* GraphicsDX11::CreateTextureFromFile(const char* fileName)
{
    common::LoadTexture texture(fileName);

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width              = texture.GetWidth();
    desc.Height             = texture.GetHeight();
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage              = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    desc.BindFlags          = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource{};
    subResource.pSysMem     = texture.GetImage();
    subResource.SysMemPitch = texture.GetWidth() * common::LoadTexture::k_RGBA;

    HRESULT ret{};
    ID3D11Texture2D* textureBuffer{};
    ret = m_device->CreateTexture2D(&desc, &subResource, &textureBuffer);
    if (FAILED(ret))
        return nullptr;

    ID3D11ShaderResourceView* resourceView{};
    ret = m_device->CreateShaderResourceView(textureBuffer, nullptr, &resourceView);
    if (FAILED(ret))
    {
        SAFE_RELEASE(textureBuffer);
        return nullptr;
    }

    SAFE_RELEASE(textureBuffer);
    assert(resourceView);

    return resourceView;
}

/// <summary>
/// 頂点バッファの設定
/// </summary>
void GraphicsDX11::SetVertexBuffer(ID3D11Buffer* buffer, unsigned int stride, unsigned int offset)
{
    m_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
}

/// <summary>
/// インデックスバッファの設定
/// </summary>
void GraphicsDX11::SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format, unsigned int offset)
{
    m_context->IASetIndexBuffer(buffer, format, offset);
}

/// <summary>
/// ブレンドステートの設定
/// </summary>
void GraphicsDX11::SetBlendState(ID3D11BlendState* blendState)
{
    float blendFactor[4]{};
    m_context->OMSetBlendState(blendState, blendFactor, UINT_MAX);
}

/// <summary>
/// ラスタライザステートの設定
/// </summary>
void GraphicsDX11::SetRasterizerState(ID3D11RasterizerState* rasterizerState)
{
    m_context->RSSetState(rasterizerState);
}

/// <summary>
/// 入力レイアウトの設定
/// </summary>
void GraphicsDX11::SetInputLayout(ID3D11InputLayout* inputLayout)
{
    m_context->IASetInputLayout(inputLayout);
}

/// <summary>
/// 頂点シェーダの設定
/// </summary>
void GraphicsDX11::SetVertexShader(ID3D11VertexShader* shader)
{
    m_context->VSSetShader(shader, nullptr, 0);
}

/// <summary>
/// 描画コール
/// </summary>
void GraphicsDX11::Draw(unsigned int vertexNum, unsigned int startVertexLocation)
{
    m_context->Draw(vertexNum, startVertexLocation);
}

/// <summary>
/// 描画コール
/// </summary>
void GraphicsDX11::DrawIndexed(unsigned int indexNum, unsigned int instanceNum, unsigned int firstIndex, unsigned int vertexoffset, unsigned int firstInstance)
{
    m_context->DrawIndexedInstanced(indexNum, instanceNum, firstIndex, vertexoffset, firstInstance);
}

/// <summary>
/// ピクセルシェーダの設定
/// </summary>
void GraphicsDX11::SetPixelShader(ID3D11PixelShader* shader)
{
    m_context->PSSetShader(shader, nullptr, 0);
}

/// <summary>
/// プリミティブトポロジーの設定
/// </summary>
void GraphicsDX11::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY primitiveTopology)
{
    m_context->IASetPrimitiveTopology(primitiveTopology);
}

/// <summary>
/// 定数バッファの設定
/// </summary>
void GraphicsDX11::SetShaderResource(ID3D11Buffer* resource, unsigned int slot, unsigned short visibility)
{
    // シェーダリソースの設定
    if ((visibility & SET_VS) != 0)
        m_context->VSSetConstantBuffers(slot, 1, &resource);
    if ((visibility & SET_HS) != 0)
        m_context->HSSetConstantBuffers(slot, 1, &resource);
    if ((visibility & SET_DS) != 0)
        m_context->DSSetConstantBuffers(slot, 1, &resource);
    if ((visibility & SET_GS) != 0)
        m_context->GSSetConstantBuffers(slot, 1, &resource);
    if ((visibility & SET_PS) != 0)
        m_context->PSSetConstantBuffers(slot, 1, &resource);
}

/// <summary>
/// テクスチャの設定
/// </summary>
void GraphicsDX11::SetTexture(ID3D11ShaderResourceView* resourceView, unsigned int slot, unsigned short visibility)
{
    // シェーダリソースの設定
    if ((visibility & SET_VS) != 0)
        m_context->VSSetShaderResources(slot, 1, &resourceView);
    if ((visibility & SET_HS) != 0)
        m_context->HSSetShaderResources(slot, 1, &resourceView);
    if ((visibility & SET_DS) != 0)
        m_context->DSSetShaderResources(slot, 1, &resourceView);
    if ((visibility & SET_GS) != 0)
        m_context->GSSetShaderResources(slot, 1, &resourceView);
    if ((visibility & SET_PS) != 0)
        m_context->PSSetShaderResources(slot, 1, &resourceView);
}

/// <summary>
/// サンプラーの設定
/// </summary>
void GraphicsDX11::SetSamplerState(ID3D11SamplerState* samplerState, unsigned int slot, unsigned short visibility)
{
    // サンプラーの設定
    if ((visibility & SET_VS) != 0)
        m_context->VSSetSamplers(slot, 1, &samplerState);
    if ((visibility & SET_HS) != 0)
        m_context->HSSetSamplers(slot, 1, &samplerState);
    if ((visibility & SET_DS) != 0)
        m_context->DSSetSamplers(slot, 1, &samplerState);
    if ((visibility & SET_GS) != 0)
        m_context->GSSetSamplers(slot, 1, &samplerState);
    if ((visibility & SET_PS) != 0)
        m_context->PSSetSamplers(slot, 1, &samplerState);
}

/// <summary>
/// マップ
/// </summary>
void GraphicsDX11::Map(ID3D11Buffer* buffer, void* data, size_t size)
{
    D3D11_MAPPED_SUBRESOURCE msr;
    HRESULT ret = m_context->Map(buffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, data, size);
    m_context->Unmap(buffer, 0);
}

/// <summary>
/// モデル行列の計算
/// </summary>
glm::mat4 GraphicsDX11::CalculateModelMatrix(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale)
{
    glm::mat4 trl, rot, scl;
    trl = glm::translate(glm::mat4(1.0f), position);
    rot = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    rot *= glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rot *= glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    scl = glm::scale(glm::mat4(1.0f), scale);

    return glm::transpose(trl * rot * scl);
}

/// <summary>
/// モデル行列の計算
/// </summary>
glm::mat4 GraphicsDX11::CalculateModelMatrix(const glm::vec3 position, const glm::quat rotation, const glm::vec3 scale)
{
    glm::mat4 trl, rot, scl;
    trl = glm::translate(glm::mat4(1.0f), position);
    rot = glm::mat4(rotation);
    scl = glm::scale(glm::mat4(1.0f), scale);

    return glm::transpose(trl * rot * scl);
}

/// <summary>
/// カメラ行列の取得
/// </summary>
glm::mat4 GraphicsDX11::GetCameraMatrix()
{
    glm::mat4 view;
    glm::mat4 trl, rot;

    trl  = glm::translate(glm::mat4(1.0f), m_mainCamera.position);
    rot = glm::mat4(glm::quat(glm::vec3(m_mainCamera.rotation)));
    view = trl * rot;
    view = glm::inverse(view);

    return glm::transpose(view);
}

/// <summary>
/// 正面方向を求める
/// </summary>
glm::vec3 GraphicsDX11::GetForward(const glm::mat4& mat)
{
    return glm::vec3(mat[2][0], mat[2][1], mat[2][2]);
}

/// <summary>
/// 右方向を求める
/// </summary>
glm::vec3 GraphicsDX11::GetRight(const glm::mat4& mat)
{
    return glm::vec3(mat[0][0], mat[0][1], mat[0][2]);
}

/// <summary>
/// 2D射影行列の取得
/// </summary>
glm::mat4 GraphicsDX11::GetProjection2DMatrix()
{
    glm::mat4 proj;
    proj = glm::orthoLH(
        0.0f, 
        float(m_width), 
        float(m_height), 
        0.0f, 
        0.0f, 
        1.0f
    );

    return glm::transpose(proj);
}

/// <summary>
/// 3D射影行列の取得
/// </summary>
glm::mat4 GraphicsDX11::GetProjection3DMatrix()
{
    glm::mat4 proj;
    proj = glm::perspectiveFovLH(
        1.0f,
        float(m_width),
        float(m_height),
        0.1f,
        256.0f
    );

    return glm::transpose(proj);
}

bool GraphicsDX11::CreateDeviceAndSwapChain(WindowWin* window)
{
	m_width     = window->GetWidth();
    m_height    = window->GetHeight();

	HRESULT ret = S_OK;
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount                        = 2;
    desc.BufferDesc.Width                   = m_width;
    desc.BufferDesc.Height                  = m_height;
    desc.BufferDesc.Format                  = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferDesc.RefreshRate.Numerator   = 60;
    desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow                       = window->GetHandle();
    desc.SampleDesc.Count                   = 1;
    desc.SampleDesc.Quality                 = 0;
    desc.SwapEffect                         = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
    desc.Windowed                           = true;

    // デバイスの生成処理
    ret = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &desc,
        &m_swapchain,
        &m_device,
        nullptr,
        &m_context
    );

    if (FAILED(ret))
        return false;

    return true;
}

bool GraphicsDX11::CreateRenderTarget()
{
    HRESULT ret;
    ID3D11Texture2D* renderTarget = nullptr;
    ret = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderTarget);
    if (FAILED(ret))
        return false;

    ret = m_device->CreateRenderTargetView(renderTarget, nullptr, &m_renderTargetView);

    return SUCCEEDED(ret);
}

bool GraphicsDX11::CreateDepthBuffer()
{
	// デプスステンシルバッファの作成
    ID3D11Texture2D* depthStencil = nullptr;
    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width               = m_width;
    textureDesc.Height              = m_height;
    textureDesc.MipLevels           = 1;
    textureDesc.ArraySize           = 1;
    textureDesc.Format              = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    textureDesc.SampleDesc.Count    = 1;
    textureDesc.SampleDesc.Quality  = 0;
    textureDesc.Usage               = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags           = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
    textureDesc.CPUAccessFlags      = 0;
    textureDesc.MiscFlags           = 0;
    m_device->CreateTexture2D(&textureDesc, nullptr, &depthStencil);
    if (!depthStencil)
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
    viewDesc.Format         = textureDesc.Format;
    viewDesc.ViewDimension  = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    viewDesc.Flags          = 0;
    m_device->CreateDepthStencilView(depthStencil, &viewDesc, &m_depthStencilView);
    if (!m_depthStencilView)
        return false;

    // レンダーターゲットに設定する
    m_context->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

    // 深度有効
    D3D11_DEPTH_STENCIL_DESC desc{};
    desc.DepthEnable    = true;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc      = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable  = false;
    m_device->CreateDepthStencilState(&desc, &m_depthStencilState);
    if (!m_depthStencilState)
        return false;

    // 深度を有効に設定
    m_context->OMSetDepthStencilState(m_depthStencilState, 0);

    return true;
}

void GraphicsDX11::SetViewport()
{
    D3D11_VIEWPORT view{};
    view.Width      = FLOAT(m_width);
    view.Height     = FLOAT(m_height);
    view.MaxDepth   = 1.0f;
    m_context->RSSetViewports(1, &view);
}
