/*================================================================
Filename: Test_ShadowMap.h
Date: 2017.12.27
Created by AirGuanZ
================================================================*/
#ifndef __TEST_SHADOW_MAP_H__
#define __TEST_SHADOW_MAP_H__

#include <memory>

#include <d3d11.h>
#include <SimpleMath.h>

#include <OWEShader.h>
#include "TestApp.h"

namespace Test_ShadowMap
{
    using namespace DirectX::SimpleMath;
    using namespace OWE;

    class App : public TestApp
    {
        struct Vertex
        {
            Vector3 pos;   //POSITION
            Vector3 color; //COLOR
            Vector3 nor;   //NORMAL
        };

        ID3D11Buffer *vtxBuf_ = nullptr;
        ID3D11InputLayout *shadowMapInputLayout_ = nullptr;
        ID3D11InputLayout *finalInputLayout_ = nullptr;

        Shader<SS_VS, SS_PS> shadowMapShader_;
        Shader<SS_VS, SS_PS> finalShader_;

        std::unique_ptr<ShaderUniforms<SS_VS, SS_PS>> shadowMapUniforms_;
        std::unique_ptr<ShaderUniforms<SS_VS, SS_PS>> finalUniforms_;

        ID3D11Texture2D *shadowMap_ = nullptr;
        ID3D11RenderTargetView *shadowMapRTV_ = nullptr;
        ID3D11ShaderResourceView *shadowMapSRV_ = nullptr;

        void InitScene(void)
        {
            HRESULT hr;

            //============ Shader ============

            shadowMapShader_.InitStage<SS_VS>(D3D_, ReadFile("Data\\Test_ShadowMap\\shadowMapVS.hlsl"));
            shadowMapShader_.InitStage<SS_PS>(D3D_, ReadFile("Data\\Test_ShadowMap\\shadowMapPS.hlsl"));

            finalShader_.InitStage<SS_VS>(D3D_, ReadFile("Data\\Test_ShadowMap\\finalVS.hlsl"));
            finalShader_.InitStage<SS_PS>(D3D_, ReadFile("Data\\Test_ShadowMap\\finalPS.hlsl"));

            shadowMapUniforms_.reset(shadowMapShader_.CreateUniformManager());
            finalUniforms_.reset(finalShader_.CreateUniformManager());

            //============ Input Layout ============

            D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, MemOffset(&Vertex::pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, MemOffset(&Vertex::color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, MemOffset(&Vertex::nor), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            hr = D3D_->CreateInputLayout(
                inputLayoutDesc, 3,
                shadowMapShader_.GetShaderByteCodeWithInputSignature(),
                shadowMapShader_.GetShaderByteCodeSizeWithInputSignature(),
                &shadowMapInputLayout_);
            if(FAILED(hr))
                throw Error("Failed to create input layout for shadow mapping shader");

            hr = D3D_->CreateInputLayout(
                inputLayoutDesc, 3,
                finalShader_.GetShaderByteCodeWithInputSignature(),
                finalShader_.GetShaderByteCodeSizeWithInputSignature(),
                &finalInputLayout_);
            if(FAILED(hr))
                throw Error("Failed to create input layout for final shader");

            //============ Vertex Buffer ============

            std::vector<Vertex> vtxBufData =
            {
                //Ground
                { { -10.0f, 0.0f, +10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                { { -10.0f, 0.0f, -10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                { { +10.0f, 0.0f, -10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                { { -10.0f, 0.0f, +10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                { { +10.0f, 0.0f, -10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                { { +10.0f, 0.0f, +10.0f }, { 0.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
                //Cube, x+
                { { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                { { +1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                { { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                { { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 1.0f, 0.0f, 0.0f } },
                //Cube, x-
                { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                { { -1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                { { -1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                { { -1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                { { -1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { -1.0f, 0.0f, 0.0f } },
                //Cube, y+
                { { -1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                { { -1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                { { -1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                { { +1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 1.0f, 0.0f } },
                //Cube, y-
                { { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                { { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                { { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                { { -1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, -1.0f, 0.0f } },
                //Cube, z+
                { { -1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { -1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { +1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { -1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { +1.0f, +1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { +1.0f, -1.0f, +1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                //Cube, z-
                { { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { +1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { -1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { +1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { -1.0f, +1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } },
                { { -1.0f, -1.0f, -1.0f }, { 1.0f, 0.431f, 0.706f }, { 0.0f, 0.0f, +1.0f } }
            };

            D3D11_BUFFER_DESC vtxBufDesc;
            vtxBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vtxBufDesc.ByteWidth = vtxBufData.size() * sizeof(Vertex);
            vtxBufDesc.CPUAccessFlags = 0;
            vtxBufDesc.MiscFlags = 0;
            vtxBufDesc.StructureByteStride = 0;
            vtxBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
            D3D11_SUBRESOURCE_DATA vtxBufInitData = { vtxBufData.data(), 0, 0 };
            hr = D3D_->CreateBuffer(&vtxBufDesc, &vtxBufInitData, &vtxBuf_);
            if(FAILED(hr))
                throw Error("Failed to create vertex buffer");

            //============ Shadow Map ============

            D3D11_TEXTURE2D_DESC texDesc;
            
        }

        ~App(void)
        {
            using namespace OWEShaderAux;

            ReleaseCOMObjects(vtxBuf_, shadowMapInputLayout_, finalInputLayout_);
            ReleaseCOMObjects(shadowMap_, shadowMapRTV_, shadowMapSRV_);

            DestroyD3DContext();
        }

    public:

    };
}

#endif //__TEST_SHADOW_MAP_H__
