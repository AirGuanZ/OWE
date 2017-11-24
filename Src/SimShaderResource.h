/*================================================================
Filename: SimShaderResource.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_RESOURCE_H__
#define __SIMSHADER_RESOURCE_H__

#include "SimShaderFatalError.h"
#include "SimShaderObjectBinding.h"
#include "SimShaderUncopiable.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderResourceObject;

    template<ShaderStageSelector StageSelector>
    class _ShaderResourceManager;

    template<ShaderStageSelector StageSelector>
    class _ShaderResourceObject : public _Uncopiable
    {
    public:
        void SetShaderResource(ID3D11ShaderResourceView *SRV)
        {
            if(SRV_)
                SRV_->Release();
            SRV_ = SRV;
            if(SRV_)
                SRV_->AddRef();
        }

        void Bind(ID3D11DeviceContext *devCon)
        {
            assert(devCon != nullptr);
            _BindShaderResource<StageSelector>(devCon, slot_, SRV_);
        }

        void Unbind(ID3D11DeviceContext *devCon)
        {
            assert(devCon != nullptr);
            _BindShaderResource<StageSelector>(devCon, slot_, nullptr);
        }

    private:
        friend class _ShaderResourceManager<StageSelector>;

        _ShaderResourceObject(UINT slot, ID3D11ShaderResourceView *SRV = nullptr)
            : slot_(slot), SRV_(SRV)
        {
            if(SRV_)
                SRV_->AddRef();
        }

        ~_ShaderResourceObject(void)
        {
            if(SRV_)
                SRV_->Release();
        }

    private:
        UINT slot_;
        ID3D11ShaderResourceView *SRV_;
    };

    template<ShaderStageSelector StageSelector>
    class _ShaderResourceManager
    {
    public:

    private:
        
    };
}

#endif //__SIMSHADER_RESOURCE_H__
