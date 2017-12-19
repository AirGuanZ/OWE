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

        static std::string DefaultCompileTarget(void)
        {
            return "vs_5_0";
        }

        static ID3D10Blob *CompileShader(
            const std::string &src, std::string *errMsg,
            const std::string &target, const std::string &entry)
        {
            if(errMsg)
                *errMsg = "";

            ID3D10Blob *rt = nullptr, *err = nullptr;
            HRESULT hr = D3DCompile(src.data(), src.size(),
                                    nullptr, nullptr, nullptr, entry.c_str(),
                                    target.c_str(), 0, 0, &rt, &err);
            if(FAILED(hr))
            {
                if(err && errMsg)
                    *errMsg = reinterpret_cast<char*>(err->GetBufferPointer());
                ReleaseCOMObjects(rt);
            }

            ReleaseCOMObjects(err);
            return rt;
        }

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

        static std::string DefaultCompileTarget(void)
        {
            return "ps_5_0";
        }

        static ID3D10Blob *CompileShader(
            const std::string &src, std::string *errMsg,
            const std::string &target, const std::string &entry)
        {
            if(errMsg)
                *errMsg = "";

            ID3D10Blob *rt = nullptr, *err = nullptr;
            HRESULT hr = D3DCompile(src.data(), src.size(),
                nullptr, nullptr, nullptr, entry.c_str(),
                target.c_str(), 0, 0, &rt, &err);
            if(FAILED(hr))
            {
                if(err && errMsg)
                    *errMsg = reinterpret_cast<char*>(err->GetBufferPointer());
                ReleaseCOMObjects(rt);
            }

            ReleaseCOMObjects(err);
            return rt;
        }

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

        static constexpr ShaderStageSelector Stage = StageSelector;

        _ShaderStage(ID3D11Device *dev, const std::string &src,
                     const std::string &target = StageSpec::DefaultCompileTarget(),
                     const std::string &entry = "main")
        {
            assert(dev != nullptr);

            std::string dummyErrMsg;
            shaderByteCode_ = StageSpec::CompileShader(src, &dummyErrMsg, target, entry);
            if(!shaderByteCode_)
                throw SimShaderError(dummyErrMsg.c_str());

            shader_ = StageSpec::InitShader(dev, shaderByteCode_->GetBufferPointer(),
                                                 shaderByteCode_->GetBufferSize());
            if(!shader_)
            {
                ReleaseCOMObjects(shaderByteCode_);
                throw SimShaderError("Failed to create D3D shader object");
            }

            InitializeShaderRecords();
        }

        _ShaderStage(ID3D11Device *dev, ID3D10Blob *shaderByteCode)
            : shaderByteCode_(shaderByteCode)
        {
            assert(dev != nullptr && shaderByteCode != nullptr);

            shaderByteCode_ = shaderByteCode;
            shaderByteCode_->AddRef();
            shader_ = StageSpec::InitShader(dev, shaderByteCode_->GetBufferPointer(),
                                                 shaderByteCode_->GetBufferSize());
            if(!shader_)
            {
                ReleaseCOMObjects(shaderByteCode_);
                throw SimShaderError("Failed to create D3D shader object");
            }

            InitializeShaderRecords();
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

        _ConstantBufferManager<StageSelector> *CreateConstantBufferManager(void) const
        {
            return new _ConstantBufferManager<StageSelector>(emptyCBRec_);
        }

        _ShaderResourceManager<StageSelector> *CreateShaderResourceManager(void) const
        {
            return new _ShaderResourceManager<StageSelector>(emptySRRec_);
        }

        _ShaderSamplerManager<StageSelector> *CreateShaderSamplerManager(void) const
        {
            return new _ShaderSamplerManager<StageSelector>(emptySSRec_);
        }

        size_t GetConstantBufferCount(void) const
        {
            return emptyCBRec_.size();
        }

        size_t GetShaderResourceCount(void) const
        {
            return emptySRRec_.size();
        }

        size_t GetShaderSamplerCount(void) const
        {
            return emptySSRec_.size();
        }

        const void *GetShaderByteCode(void) const
        {
            assert(shaderByteCode_);
            return shaderByteCode_->GetBufferPointer();
        }

        UINT GetShaderByteCodeSize(void) const
        {
            assert(shaderByteCode_);
            return shaderByteCode_->GetBufferSize();
        }

    private:
        void InitializeShaderRecords(void)
        {
            ID3D11ShaderReflection *ref = _GetShaderReflection(shaderByteCode_->GetBufferPointer(),
                                                               shaderByteCode_->GetBufferSize());
            if(!ref)
                throw SimShaderError("Failed to initialize shader reflection");

            std::map<std::string, _CBInfo> CBInfos;
            _GetConstantBuffers(ref, &CBInfos);
            for(auto &it : CBInfos)
                emptyCBRec_[it.first] = { it.second.slot, it.second.byteSize, nullptr };
            CBInfos.clear();

            std::map<std::string, UINT> STexInfos;
            _GetShaderTextures(ref, &STexInfos);
            for(auto &it : STexInfos)
                emptySRRec_[it.first] = { it.second, nullptr };
            STexInfos.clear();

            std::map<std::string, UINT> SSamInfos;
            _GetShaderSamplers(ref, &SSamInfos);
            for(auto &it : SSamInfos)
                emptySSRec_[it.first] = { it.second, nullptr };
            SSamInfos.clear();

            ReleaseCOMObjects(ref);
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
