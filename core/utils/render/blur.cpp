#ifdef _WIN32
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#endif

#include <imgui.h>
#include "blur.h"

// shaders are build during compilation and header files are created

#include "d3d11/shaders/cpp/d3d11/pixel/blur_x.h"
#include "d3d11/shaders/cpp/d3d11/pixel/blur_y.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <imgui.h>
#include "blur.h"
#include "d3d11/shaders_dx11.hpp"
#include "../../sdk/sdk.h"
using Microsoft::WRL::ComPtr;

// Constant buffer structure
struct CBPerFrame
{
    float params[4]; // Adjust based on your shader constants
};

// Global variables
static int backbufferWidth = 0;
static int backbufferHeight = 0;

static ComPtr<ID3D11RenderTargetView> pRTVBackup = nullptr;
static ComPtr<ID3D11PixelShader> pBlurShaderX = nullptr;
static ComPtr<ID3D11PixelShader> pBlurShaderY = nullptr;
static ComPtr<ID3D11Texture2D> pBlurTexture = nullptr;
static ComPtr<ID3D11SamplerState> pSamplerState = nullptr;
static ComPtr<ID3D11Buffer> pConstantBuffer = nullptr; // Constant buffer
static ComPtr<ID3D11ShaderResourceView> pBlurTextureSRV = nullptr; // SRV for blur texture

// Function prototypes
static void BeginBlur(ID3D11DeviceContext* pContext);
static void FirstBlurPass(ID3D11DeviceContext* pContext);
static void SecondBlurPass(ID3D11DeviceContext* pContext);
static void EndBlur(ID3D11DeviceContext* pContext);

// Initialize resources
void blur::InitResources(ID3D11Device* pDevice)
{
    // Create constant buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(CBPerFrame);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;

    pDevice->CreateBuffer(&cbDesc, nullptr, &pConstantBuffer);

    // Create sampler state
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);

    // Create shaders
    pDevice->CreatePixelShader(shaders::pixel::blur_x, sizeof(shaders::pixel::blur_x), nullptr, &pBlurShaderX);
    pDevice->CreatePixelShader(shaders::pixel::blur_y, sizeof(shaders::pixel::blur_y), nullptr, &pBlurShaderY);
}

// Begin blur effect
static void BeginBlur(ID3D11DeviceContext* pContext)
{
    if (!pBlurShaderX || !pBlurShaderY)
    {
        // Shaders should have been initialized before calling BeginBlur
        return;
    }

    // Backup the current render target
    pContext->OMGetRenderTargets(1, pRTVBackup.GetAddressOf(), nullptr);

    // Get the dimensions of the current render target
    D3D11_TEXTURE2D_DESC desc;
    ComPtr<ID3D11Resource> rtRes;
    pRTVBackup->GetResource(rtRes.GetAddressOf());
    ComPtr<ID3D11Texture2D> rtTex;
    rtRes.As(&rtTex);
    rtTex->GetDesc(&desc);

    // Check if the render target size has changed
    if (backbufferWidth != desc.Width || backbufferHeight != desc.Height)
    {
        // Release existing resources
        if (pBlurTexture)
            pBlurTexture.Get()->Release();

        // Update dimensions
        backbufferWidth = desc.Width;
        backbufferHeight = desc.Height;

        // Create a new texture for blurring
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = desc.Width;
        texDesc.Height = desc.Height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Adjust format as needed
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;

        sdk::m_device->CreateTexture2D(&texDesc, nullptr, &pBlurTexture);
    }

    // Set blur texture as render target
    pContext->OMSetRenderTargets(1, pRTVBackup.GetAddressOf(), nullptr);
}

// First pass of the blur effect
static void FirstBlurPass(ID3D11DeviceContext* pContext)
{
    if (!pBlurShaderX)
        return;

    pContext->PSSetShader(pBlurShaderX.Get(), nullptr, 0);

    // Update constant buffer
    CBPerFrame cbData = {};
    cbData.params[0] = 1.0f / backbufferWidth; // Example parameter
    cbData.params[1] = 0.0f;
    cbData.params[2] = 0.0f;
    cbData.params[3] = 0.0f;

    pContext->UpdateSubresource(pConstantBuffer.Get(), 0, nullptr, &cbData, 0, 0);
    pContext->PSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

    // Bind blur texture as shader resource
    pContext->PSSetShaderResources(0, 1, pBlurTextureSRV.GetAddressOf());

    // Set sampler state
    pContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());
}

// Second pass of the blur effect
static void SecondBlurPass(ID3D11DeviceContext* pContext)
{
    if (!pBlurShaderY)
        return;

    pContext->PSSetShader(pBlurShaderY.Get(), nullptr, 0);

    // Update constant buffer
    CBPerFrame cbData = {};
    cbData.params[0] = 0.0f;
    cbData.params[1] = 1.0f / backbufferHeight; // Example parameter
    cbData.params[2] = 0.0f;
    cbData.params[3] = 0.0f;

    pContext->UpdateSubresource(pConstantBuffer.Get(), 0, nullptr, &cbData, 0, 0);
    pContext->PSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

    // Bind blur texture as shader resource
    pContext->PSSetShaderResources(0, 1, pBlurTextureSRV.GetAddressOf());

    // Set sampler state
    pContext->PSSetSamplers(0, 1, pSamplerState.GetAddressOf());
}

// End blur effect
static void EndBlur(ID3D11DeviceContext* pContext)
{
    // Restore original render target
    pContext->OMSetRenderTargets(1, pRTVBackup.GetAddressOf(), nullptr);
    pRTVBackup.Get()->Release();
    pRTVBackup.Reset();

    // Clear shader resources and constant buffers
    ID3D11ShaderResourceView* nullSRV = nullptr;
    pContext->PSSetShaderResources(0, 1, &nullSRV);

    ID3D11Buffer* nullBuffer = nullptr;
    pContext->PSSetConstantBuffers(0, 1, &nullBuffer);
}

// Draw function for background blur
void blur::initialize(ImDrawList* drawList, ID3D11DeviceContext* pContext)
{
    
    BeginBlur(pContext);

    // Perform multiple passes for stronger blur effect
    for (int i = 0; i < 8; ++i)
    {
        FirstBlurPass(pContext);
        drawList->AddImage(pBlurTexture.Get(), { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });


        // Draw using pBlurTexture
        SecondBlurPass(pContext);
        drawList->AddImage(pBlurTexture.Get(), { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f });

        // Draw using pBlurTexture
    }

    EndBlur(pContext);
    drawList->AddImageRounded(pBlurTexture.Get(), { 0.0f, 0.0f }, { backbufferWidth * 1.0f, backbufferHeight * 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(255, 255, 255, 255), 7.f);
    // Draw final image using pBlurTexture
}

// Cleanup resources
void CleanupResources()
{
    pRTVBackup.Reset();
    pBlurShaderX.Reset();
    pBlurShaderY.Reset();
    pBlurTexture.Reset();
    pSamplerState.Reset();
    pConstantBuffer.Reset();
    pBlurTextureSRV.Reset();
}