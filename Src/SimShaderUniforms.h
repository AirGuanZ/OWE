/*================================================================
Filename: SimShaderUniforms.h
Date: 2017.12.18
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_UNIFORMS_H__
#define __SIMSHADER_UNIFORMS_H__

#include "SimShader.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderStageUniforms
    {
    public:
        _ShaderStageUniforms(const _ShaderStage<StageSelector> &stage)
        {
            CBMgr_ = stage.GetConstantBufferCount() ? stage.CreateConstantBufferManager() : nullptr;
            SRMgr_ = stage.GetShaderResourceCount() ? stage.CreateShaderResourceManager() : nullptr;
            SSMgr_ = stage.GetShaderSamplerCount() ? stage.CreateShaderSamplerManager() : nullptr;
        }

        ~_ShaderStageUniforms(void)
        {
            SafeDeleteObjects(CBMgr_, SRMgr_, SSMgr_);
        }

        _ConstantBufferManager<StageSelector> *GetConstantBufferManager(void)
        {
            return CBMgr_;
        }

        _ShaderResourceManager<StageSelector> *GetShaderResourceManager(void)
        {
            return SRMgr_;
        }

        _ShaderSamplerManager<StageSelector> *GetShaderSamplerManager(void)
        {
            return SSMgr_;
        }

        template<typename BufferType, bool Dynamic>
        _ConstantBufferObject<BufferType, StageSelector, Dynamic>*
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data)
        {
            assert(CBMgr_ != nullptr);
            return CBMgr_->GetConstantBuffer<BufferType, Dynamic>(dev, name, data);
        }

        _ShaderResourceObject<StageSelector> *GetShaderResource(const std::string &name)
        {
            assert(SRMgr_ != nullptr);
            return SRMgr_->GetShaderResourceObject(name);
        }

        _ShaderSamplerObject<StageSelector> *GetShaderSampler(const std::string &name)
        {
            assert(SSMgr_ != nullptr);
            return SSMgr_->GetShaderSamplerObject(name);
        }

    private:
        _ConstantBufferManager<StageSelector> *CBMgr_;
        _ShaderResourceManager<StageSelector> *SRMgr_;
        _ShaderSamplerManager<StageSelector> *SSMgr_;
    };

    template<ShaderStageSelector...StageSelectors>
    class _ShaderUniformManager
    {
    public:
        _ShaderUniformManager(const _ShaderStage<StageSelectors>&...stages)
            : stageUniforms_(stages...)
        {

        }

        ~_ShaderUniformManager(void)
        {

        }

        template<ShaderStageSelector StageSelector>
        _ShaderStageUniforms<StageSelector> *GetStageUniforms(void)
        {
            return &std::get<FindInNumList<ShaderStageSelector, StageSelector, StageSelectors...>()>(stageUniforms_);
        }

        template<ShaderStageSelector StageSelector>
        _ConstantBufferManager<StageSelector> *GetConstantBufferManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetConstantBufferManager();
        }

        template<ShaderStageSelector StageSelector>
        _ShaderResourceManager<StageSelector> *GetShaderResourceManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetShaderResourceManager();
        }

        template<ShaderStageSelector StageSelector>
        _ShaderSamplerManager<StageSelector> *GetShaderSamplerManager(void)
        {
            return GetStageUniforms<StageSelector>()->GetShaderSamplerManager();
        }

        template<ShaderStageSelector StageSelector, typename BufferType, bool Dynamic = true>
        _ConstantBufferObject<BufferType, StageSelector, Dynamic>*
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data = nullptr)
        {
            return GetStageUniforms<StageSelector>()->GetConstantBuffer<BufferType, Dynamic>(dev, name, data);
        }

        template<ShaderStageSelector StageSelector>
        _ShaderResourceObject<StageSelector> *GetShaderResource(const std::string &name)
        {
            return GetStageUniforms<StageSelector>()->GetShaderResource(name);
        }

        template<ShaderStageSelector StageSelector>
        _ShaderSamplerObject<StageSelector> *GetShaderSamplerManager(const std::string &name)
        {
            return GetStageUniforms<StageSelector>()->GetShaderSampler(name);
        }

    private:
        std::tuple<_ShaderStageUniforms<StageSelectors>...> stageUniforms_;
    };
}

namespace SimShader
{
    template<ShaderStageSelector...StageSelectors>
    using ShaderUniforms = _SimShaderAux::_ShaderUniformManager<StageSelectors...>;
}

#endif //__SIMSHADER_UNIFORMS_H__
