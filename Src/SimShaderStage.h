/*================================================================
Filename: SimShaderStage.h
Date: 2017.11.26
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_STAGE_H__
#define __SIMSHADER_STAGE_H__

#include <cassert>
#include <d3d11.h>

#include "SimShaderObjectBinding.h"
#include "SimShaderReflection.h"
#include "SimShaderReleaseCOMObjects.h"
#include "SimShaderUncopiable.h"

#include "SimShaderConstantBuffer.h"
#include "SimShaderSampler.h"
#include "SimShaderResource.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector>
    class _ShaderStageSpec;

    template<>
    class _ShaderStageSpec<SS_VS>
    {
    public:
        using D3DShaderType = ID3D11VertexShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, SIZE_T length)
        {
            assert(dev != nullptr);
            assert(shaderCompiled != nullptr && length > 0);
            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreateVertexShader(shaderCompiled, length, nullptr, &rt)))
                return nullptr;
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC != nullptr);
            DC->VSSetShader(shader, nullptr, 0);
        }
    };

    template<>
    class _ShaderStageSpec<SS_PS>
    {
    public:
        using D3DShaderType = ID3D11PixelShader;

        static D3DShaderType *InitShader(ID3D11Device *dev, void *shaderCompiled, SIZE_T length)
        {
            assert(dev != nullptr);
            assert(shaderCompiled != nullptr && length > 0);
            D3DShaderType *rt = nullptr;
            if(FAILED(dev->CreatePixelShader(shaderCompiled, length, nullptr, &rt)))
                return nullptr;
            return rt;
        }

        static void BindShader(ID3D11DeviceContext *DC, D3DShaderType *shader)
        {
            assert(DC != nullptr);
            DC->PSSetShader(shader, nullptr, 0);
        }
    };

    template<ShaderStageSelector StageSelector>
    class _ShaderStage : public _Uncopiable
    {
    public:
        using StageSpec = _ShaderStageSpec<StageSelector>;

        _ShaderStage(ID3D11Device *dev, ID3D10Blob *shaderByteCode)
            : shaderByteCode_(shaderByteCode)
        {
            assert(dev != nullptr && shaderByteCode != nullptr);

            shaderByteCode->AddRef();
            shader_ = StageSpec::InitShader(dev, shaderByteCode->GetBufferPointer(),
                                                 shaderByteCode->GetBufferSize());

            //初始化各种empty XX records

            ID3D11ShaderReflection *ref = _GetShaderReflection(shaderByteCode->GetBufferPointer(),
                                                               shaderByteCode->GetBufferSize());
            if(!ref)
                throw SimShaderError("Failed to initialize shader reflection");

            std::map<std::string, _CBInfo> CBInfos;
            _GetConstantBuffers(ref, &CBInfos);
            for(auto it : CBInfos)
                emptyCBRec_[it.first] = { it.second.slot, it.second.byteSize, nullptr };
            CBInfos.clear();

            std::map<std::string, UINT> STexInfos;
            _GetShaderTextures(ref, &STexInfos);
            for(auto it : STexInfos)
                emptySRRec_[it.first] = { it.second, nullptr };
            STexInfos.clear();

            std::map<std::string, UINT> SSamInfos;
            _GetShaderSamplers(ref, &SSamInfos);
            for(auto it : SSamInfos)
                emptySSRec_[it.first] = { it.second, nullptr };
            SSamInfos.clear();

            ReleaseCOMObjects(ref);
        }

        ~_ShaderStage(void)
        {
            ReleaseCOMObjects(shader_, shaderByteCode_);
        }

        void BindShader(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            StageSpec::BindShader(DC, shader_);
        }

        void UnbindShader(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            StageSpec::BindShader(DC, nullptr);
        }

        _ConstantBufferManager<StageSelector> *CreateConstantBufferManager(void)
        {
            return new _ConstantBufferManager<StageSelector>(emptyCBRec_);
        }

        _ShaderResourceManager<StageSelector> *CreateShaderResourceManager(void)
        {
            return new _ShaderResourceManager<StageSelector>(emptySRRec_);
        }

        _ShaderSamplerManager<StageSelector> *CreateShaderSamplerManager(void)
        {
            return new _ShaderSamplerManager<StageSelector>(emptySSRec_);
        }

        void *GetShaderByteCode(void)
        {
            assert(shaderByteCode_);
            return shaderByteCode_->GetBufferPointer();
        }

        UINT GetShaderByteCodeSize(void)
        {
            assert(shaderByteCode_);
            shaderByteCode_->GetBufferSize();
        }

    private:
        typename StageSpec::D3DShaderType *shader_;
        ID3D10Blob *shaderByteCode_;

        std::map<std::string, typename _ConstantBufferManager<StageSelector>::_CBRec> emptyCBRec_;
        std::map<std::string, typename _ShaderResourceManager<StageSelector>::_SRRec> emptySRRec_;
        std::map<std::string, typename _ShaderSamplerManager<StageSelector>::_SSRec> emptySSRec_;
    };
}

#endif //__SIMSHADER_STAGE_H__
