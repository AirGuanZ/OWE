/*================================================================
Filename: SimShaderResource.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_RESOURCE_H__
#define __SIMSHADER_RESOURCE_H__

#include <map>

#include "SimShaderFatalError.h"
#include "SimShaderObjectBinding.h"
#include "SimShaderUncopiable.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderResourceObject;

    template<ShaderStageSelector>
    class _ShaderStage;

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
    class _ShaderResourceManager : public _Uncopiable
    {
    public:
        using RscObj = _ShaderResourceObject<StageSelector>;

        ~_ShaderResourceManager(void)
        {
            for(auto it : SRs_)
            {
                if(it.second.obj)
                    delete it.second.obj;
            }
        }

        void AddShaderResource(const std::string &name, UINT slot, ID3D11ShaderResourceView *initSRV = nullptr)
        {
            auto it = SRs_.find(name);
            if(it != SRs_.end())
                throw SimShaderError("Shader resource name repeated: " + name);
            SRs_[name] = _SRRec{ slot, new RscObj(slot, initSRV) };
        }

        RscObj *GetShaderResourceObject(const std::string &name)
        {
            auto it = SRs_.find(name);
            it(it == SRs_.end())
                throw SimShaderError("Shader resource not found: " + name);
            
            assert(it.second.obj != nullptr);
            return it.second.obj;
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            for(auto it : SRs_)
            {
                assert(it.second.obj);
                it.second.obj->Bind(DC);
            }
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            for(auto it : SRs_)
            {
                assert(it.second.obj);
                it.second.obj->Unbind(DC);
            }
        }

    private:
        friend class _ShaderStage<StageSelector>;

        struct _SRRec
        {
            UINT slot;
            RscObj *obj;
        };

        _ShaderResourceManager(const std::map<std::string, _SRRec> &src)
            : SRs_(src)
        {

        }

    private:
        std::map<std::string, _SRRec> SRs_;
    };
}

#endif //__SIMSHADER_RESOURCE_H__
