/*================================================================
Filename: Test_HeightMap.h
Date: 2017.12.19
Created by AirGuanZ
================================================================*/
#ifndef __TEST_HEIGHT_MAP_H__
#define __TEST_HEIGHT_MAP_H__

#include <fstream>
#include <string>

#include <d3d11.h>
#include <SimpleMath.h>
#include <SimShader.hpp>

#include "TestApp.h"

namespace Test_HeightMap
{
    using namespace DirectX::SimpleMath;
    using namespace SimShader;

    class App : public TestApp
    {
        struct Vertex
        {
            Vector2 pos;
            Vector2 uv;
        };

        struct VSCB
        {
            Matrix WVP;
        };

        ID3D11Buffer *vtxBuf_ = nullptr;
        ID3D11Buffer *idxBuf_ = nullptr;
        ID3D11InputLayout *inputLayout_ = nullptr;

        Shader<SS_VS, SS_PS> shader_;
        ShaderUniforms<SS_VS, SS_PS> *uniforms_ = nullptr;

        ID3D11Resource *heightMap_ = nullptr;
        ID3D11ShaderResourceView *heightMapView_ = nullptr;

        ID3D11Resource *tex_ = nullptr;
        ID3D11ShaderResourceView *texView_ = nullptr;

        ID3D11SamplerState *sampler_ = nullptr;

        template<typename ClassType, typename MemType>
        static size_t _MemOffset(MemType ClassType::* pMem)
        {
            return reinterpret_cast<size_t>(&(reinterpret_cast<ClassType*>(0)->*pMem));
        }

        static std::string _ReadFile(const std::string &filename)
        {
            std::ifstream fin(filename, std::ifstream::in);
            if(!fin)
                throw SimShader::Error(("Failed to open file: " + filename).c_str());
            return std::string(std::istreambuf_iterator<char>(fin),
                               std::istreambuf_iterator<char>());
        }
    public:

        void InitScene(void)
        {
            HRESULT hr;

            //============= Shader =============
            
            shader_.InitStage<SS_VS>(D3D_, _ReadFile("Data\\Test_HeightMap\\test.vs"));
            shader_.InitStage<SS_PS>(D3D_, _ReadFile("Data\\Test_HeightMap\\test.ps"));

            uniforms_ = shader_.CreateUniformManager();

            //============= Input Layout =============

            D3D11_INPUT_ELEMENT_DESC inputDesc[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, _MemOffset(&Vertex::pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, _MemOffset(&Vertex::uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };

            hr = D3D_->CreateInputLayout(inputDesc, 2,
                shader_.GetShaderByteCodeWithInputSignature(),
                shader_.GetShaderByteCodeSizeWithInputSignature(),
                &inputLayout_);
            if(FAILED(hr))
                throw SimShader::Error("Failed to create input layout");

            //============= Vertex & Index Buffer =============

            constexpr float VERTEX_GRID_SIZE = 1.0f;
            constexpr int VERTEX_GRID_COUNT = 100;

            std::vector<Vertex> vtxBufData(VERTEX_GRID_COUNT * VERTEX_GRID_COUNT);
            for(int yIdx = 0; yIdx != VERTEX_GRID_COUNT; ++yIdx)
            {
                for(int xIdx = 0; xIdx != VERTEX_GRID_COUNT; ++xIdx)
                {
                    vtxBufData[yIdx * VERTEX_GRID_COUNT + xIdx].pos =
                        Vector2(xIdx * VERTEX_GRID_SIZE, yIdx * VERTEX_GRID_SIZE);
                    vtxBufData[yIdx * VERTEX_GRID_COUNT + xIdx].uv =
                        Vector2(xIdx / (VERTEX_GRID_COUNT - 1.0f), yIdx / (VERTEX_GRID_COUNT - 1.0f));
                }
            }

            std::vector<UINT> idxBufData(6 * (VERTEX_GRID_COUNT - 1) * (VERTEX_GRID_COUNT - 1));
            int idxBufIdx = 0;
            for(int yIdx = 0; yIdx != VERTEX_GRID_COUNT - 1; ++yIdx)
            {
                for(int xIdx = 0; xIdx != VERTEX_GRID_COUNT - 1; ++xIdx)
                {
                    idxBufData[idxBufIdx++] = yIdx * VERTEX_GRID_COUNT + xIdx;
                    idxBufData[idxBufIdx++] = (yIdx + 1) * VERTEX_GRID_COUNT + xIdx;
                    idxBufData[idxBufIdx++] = (yIdx + 1) * VERTEX_GRID_COUNT + xIdx + 1;
                    idxBufData[idxBufIdx++] = yIdx * VERTEX_GRID_COUNT + xIdx;
                    idxBufData[idxBufIdx++] = (yIdx + 1) * VERTEX_GRID_COUNT + xIdx + 1;
                    idxBufData[idxBufIdx++] = yIdx * VERTEX_GRID_COUNT + xIdx + 1;
                }
            }

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

            D3D11_BUFFER_DESC idxBufDesc;
            idxBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            idxBufDesc.ByteWidth = idxBufData.size() * sizeof(decltype(idxBufData)::value_type);
            idxBufDesc.CPUAccessFlags = 0;
            idxBufDesc.MiscFlags = 0;
            idxBufDesc.StructureByteStride = 0;
            idxBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
            D3D11_SUBRESOURCE_DATA idxBufInitData = { idxBufData.data(), 0, 0 };
            hr = D3D_->CreateBuffer(&idxBufDesc, &idxBufInitData, &idxBuf_);
            if(FAILED(hr))
                throw Error("Failed to create index buffer");
        }

        void DestroyScene(void)
        {
            using namespace _SimShaderAux;

            ReleaseCOMObjects(vtxBuf_, idxBuf_, inputLayout_);
            ReleaseCOMObjects(heightMap_, heightMapView_, tex_, texView_);
            ReleaseCOMObjects(sampler_);
            SafeDeleteObjects(uniforms_);
            shader_.DestroyAllStages();
        }

    public:

    };
}

#endif //__TEST_HEIGHT_MAP_H__
