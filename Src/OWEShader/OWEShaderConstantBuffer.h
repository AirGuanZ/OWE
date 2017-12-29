/*================================================================
Filename: OWEShaderConstantBuffer.h
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#ifndef __OWESHADER_CONSTANT_BUFFER_H__
#define __OWESHADER_CONSTANT_BUFFER_H__

#include <cstring>
#include <map>
#include <typeinfo>
#include <vector>

#include "OWEShaderFatalError.h"
#include "OWEShaderGenBuffer.h"
#include "OWEShaderObjectBinding.h"
#include "OWEShaderReleaseCOMObjects.h"
#include "OWEShaderUncopiable.h"

namespace OWEShaderAux
{
    template<ShaderStageSelector ShaderSelector>
    class ConstantBufferObjectBase;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class ConstantBufferAttributes;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class ConstantBufferObject;

    template<ShaderStageSelector>
    class ShaderStage;

    template<ShaderStageSelector StageSelector>
    class ConstantBufferManager;

    template<ShaderStageSelector ShaderSelector>
    class ConstantBufferObjectBase
    {
    public:
        ConstantBufferObjectBase(UINT slot, ID3D11Buffer *buf)
            : slot_(slot), buf_(buf)
        {

        }

        virtual ~ConstantBufferObjectBase(void)
        {

        }

        void Bind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            BindConstantBuffer<ShaderSelector>(devCon, slot_, buf_);
        }

        void Unbind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            BindConstantBuffer<ShaderSelector>(devCon, slot_, nullptr);
        }
        
    protected:
        UINT slot_;
        ID3D11Buffer *buf_;
    };

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class ConstantBufferAttributes
    {
    public:
        using MyType = ConstantBufferObject<_BufferType, _StageSelector, _IsDynamic>;
        using BufferType = _BufferType;

        static constexpr ShaderStageSelector ShaderStage = _StageSelector;
        static constexpr bool IsDynamic = _IsDynamic;
    };

    //Static constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class ConstantBufferObject<_BufferType, _StageSelector, false>
        : public ConstantBufferAttributes<_BufferType, _StageSelector, false>,
          public ConstantBufferObjectBase<_StageSelector>,
          public Uncopiable
    {
    private:
        friend class ConstantBufferManager<_StageSelector>;

        ConstantBufferObject(ID3D11DeviceContext *devCon, UINT slot, const _BufferType *data)
            : ConstantBufferObjectBase(slot, nullptr)
        {
            assert(devCon && data);
            D3D11_SUBRESOURCE_DATA dataDesc = { &data, 0, 0 };
            buf_ = GenConstantBuffer(dev, sizeof(_BufferType), false, &dataDesc);
            if(!buf_)
                throw OWEShaderError("Failed to create constant buffer");
        }

        ~ConstantBufferObject(void)
        {
            ReleaseCOMObjects(buf_);
        }
    };

    //Dynamic constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class ConstantBufferObject<_BufferType, _StageSelector, true>
        : public ConstantBufferAttributes<_BufferType, _StageSelector, true>,
          public ConstantBufferObjectBase<_StageSelector>,
          public Uncopiable
    {
    public:
        void SetBufferData(ID3D11DeviceContext *devCon, const BufferType &data)
        {
            assert(devCon != nullptr && buf_ != nullptr);
            D3D11_MAPPED_SUBRESOURCE mappedRsc;
            devCon->Map(buf_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRsc);
            std::memcpy(mappedRsc.pData, &data, sizeof(_BufferType));
            devCon->Unmap(buf_, 0);
        }

    private:
        friend class ConstantBufferManager<_StageSelector>;

        ConstantBufferObject(ID3D11Device *dev, UINT slot, const _BufferType *data = nullptr)
            : ConstantBufferObjectBase(slot, nullptr)
        {
            assert(dev);
            if(data)
            {
                D3D11_SUBRESOURCE_DATA dataDesc = { &data, 0, 0 };
                buf_ = GenConstantBuffer(dev, sizeof(_BufferType), true, &dataDesc);
            }
            else
                buf_ = GenConstantBuffer(dev, sizeof(_BufferType), true, nullptr);
            if(!buf_)
                throw OWEShaderError("Failed to create constant buffer");
        }

        ~ConstantBufferObject(void)
        {
            ReleaseCOMObjects(buf_);
        }
    };

    template<ShaderStageSelector StageSelector>
    class ConstantBufferManager : public Uncopiable
    {
    public:
        ConstantBufferManager(void) = default;

        ~ConstantBufferManager(void)
        {
            for(auto &it : CBs_)
                SafeDeleteObjects(it.second.obj);
        }

        void AddBuffer(const std::string &name, UINT slot, UINT byteSize)
        {
            assert(!name.empty() && byteSize > 0);
            if(CBs_.find(name) != CBs_.end())
                throw OWEShaderError("Constant buffer name repeated: " + name);
            CBs_[name] = CBRec{ slot, byteSize, nullptr };
        }

        template<typename BufferType, bool IsDynamic = true>
        ConstantBufferObject<BufferType, StageSelector, IsDynamic> *
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data = nullptr)
        {
            using ResultType = ConstantBufferObject<BufferType, StageSelector, IsDynamic>;

            assert(dev);
            assert(IsDynamic || data);

            auto &it = CBs_.find(name);
            if(it == CBs_.end())
                throw OWEShaderError(("Constant buffer not found: " + name).c_str());
            CBRec &rec = it->second;

            if(rec.obj)
            {
                if(typeid(*rec.obj) != typeid(ConstantBufferObject<BufferType, StageSelector, IsDynamic>))
                    throw OWEShaderError("Inconsistent constant buffer type");
                return reinterpret_cast<ResultType*>(rec.obj);
            }

            if(sizeof(BufferType) != rec.byteSize)
                throw OWEShaderError("Inconstent constant buffer size");

            rec.obj = new ResultType(dev, rec.slot, data);
            return reinterpret_cast<ResultType*>(rec.obj);
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            for(auto it : CBs_)
            {
                if(it.second.obj)
                    it.second.obj->Bind(DC);
            }
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            for(auto it : CBs_)
            {
                if(it.second.obj)
                    it.second.obj->Unbind(DC);
            }
        }

    private:
        friend class ShaderStage<StageSelector>;

        struct CBRec
        {
            UINT slot;
            UINT byteSize;
            ConstantBufferObjectBase<StageSelector> *obj;
        };
        using CBTable = std::map<std::string, CBRec>;
        
        ConstantBufferManager(const CBTable &src)
            : CBs_(src)
        {

        }

    private:
        CBTable CBs_;
    };
}

#endif //__OWESHADER_CONSTANT_BUFFER_H__
