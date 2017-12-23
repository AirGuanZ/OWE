/*================================================================
Filename: Test_NormalMap.h
Date: 2017.12.22
Created by AirGuanZ
================================================================*/
#ifndef __TEST_NORMAL_MAP_H__
#define __TEST_NORMAL_MAP_H__

#include <DDSTextureLoader.h>
#include <SimpleMath.h>

#include <OWEShader.h>
#include "TestApp.h"

namespace Test_NormalMap
{
    using namespace OWE;
    using namespace DirectX::SimpleMath;

    class App : public TestApp
    {
        struct Vertex
        {
            Vector3 pos;   //POSITION
            Vector2 uv;    //TEXCOORD
            Vector3 axisU; //AXIS_U
            Vector3 axisV; //AXIS_V
        };

        struct VSCB_Trans
        {
            Matrix world;
            Matrix WVP;
        };

        struct VSCB_PointLight
        {
            Vector3 pos;
            float pad0;
            Vector3 color;
            float pad1;
            Vector3 disFactor;
            float pad2;
        };

        Shader<SS_VS, SS_PS> shader_;
        ShaderUniforms<SS_VS, SS_PS> *uniforms_;

        ID3D11Resource *normalMap_ = nullptr;
        ID3D11ShaderResourceView *normalMapView_ = nullptr;

        ID3D11Resource *tex_ = nullptr;
        ID3D11ShaderResourceView *texView_ = nullptr;

        ID3D11Buffer *vtxBuf_ = nullptr;
        ID3D11InputLayout *inputLayout_ = nullptr;

        ID3D11SamplerState *sampler_;

        void InitScene(void)
        {
            HRESULT hr;

            //============ Shader ============

            shader_.InitStage<SS_VS>(D3D_, _ReadFile("Data\\Test_NormalMap\\test.vs"));
            shader_.InitStage<SS_PS>(D3D_, _ReadFile("Data\\Test_NormalMap\\test.ps"));

            uniforms_ = shader_.CreateUniformManager();

            //============ Input Layout ============

            D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                    _MemOffset(&Vertex::pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
                    _MemOffset(&Vertex::uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "AXIS_U", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                    _MemOffset(&Vertex::axisU), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "AXIS_V", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
                    _MemOffset(&Vertex::axisV), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            hr = D3D_->CreateInputLayout(
                inputLayoutDesc, 4,
                shader_.GetShaderByteCodeWithInputSignature(),
                shader_.GetShaderByteCodeSizeWithInputSignature(),
                &inputLayout_);
            if(FAILED(hr))
                throw Error("Failed to create input layout");

            //============ Vertex Buffer ============

            Vertex vtxBufData[] =
            {
                { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                { {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                { {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                { {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } }
            };

            D3D11_BUFFER_DESC vtxBufDesc;
            vtxBufDesc.ByteWidth = sizeof(vtxBufData);
            vtxBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
            vtxBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            vtxBufDesc.CPUAccessFlags = 0;
            vtxBufDesc.MiscFlags = 0;
            vtxBufDesc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA vtxBufInitData = { vtxBufData, 0, 0 };
            hr = D3D_->CreateBuffer(
                &vtxBufDesc, &vtxBufInitData, &vtxBuf_);
            if(FAILED(hr))
                throw Error("Failed to create vertex buffer");

            //============= Textures =============

            if(FAILED(DirectX::CreateDDSTextureFromFile(
                    D3D_, L"Data\\Test_NormalMap\\tex.dds", &tex_, &texView_)) ||
               FAILED(DirectX::CreateDDSTextureFromFile(
                    D3D_, L"Data\\Test_NormalMap\\normalMap.dds", &normalMap_, &normalMapView_)))
                throw Error("Failed to load textures from file");

            //============= Sampler State =============

            D3D11_SAMPLER_DESC samplerDesc;
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 1;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            samplerDesc.MinLOD = -FLT_MAX;
            samplerDesc.MaxLOD = FLT_MAX;
            ZeroMemory(samplerDesc.BorderColor, 4 * sizeof(FLOAT));

            hr = D3D_->CreateSamplerState(&samplerDesc, &sampler_);
            if(FAILED(hr))
                throw Error("Failed to create sampler state");
        }

        void DestroyScene(void)
        {
            using namespace _OWEShaderAux;

            ReleaseCOMObjects(vtxBuf_, inputLayout_);
            ReleaseCOMObjects(normalMap_, normalMapView_);
            ReleaseCOMObjects(tex_, texView_);
            ReleaseCOMObjects(sampler_);
            shader_.Destroy();
            SafeDeleteObjects(uniforms_);
        }

    public:
        void Run(void)
        {
            try
            {
                if(!InitD3DContext())
                    throw Error("Failed to initialize D3D context");
                InitScene();
            }
            catch(const Error &err)
            {
                DestroyScene();
                DestroyD3DContext();
                throw err;
            }
        }
    };
}

#endif //__TEST_NORMAL_MAP_H__
