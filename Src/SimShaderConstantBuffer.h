/*================================================================
Filename: SimShaderConstantBuffer.h
Date: 2017.11.23
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_CONSTANT_BUFFER_H__
#define __SIMSHADER_CONSTANT_BUFFER_H__

#include <map>
#include <typeinfo>
#include <vector>

#include "SimShaderFatalError.h"
#include "SimShaderGenBuffer.h"
#include "SimShaderObjectBinding.h"
#include "SimShaderUncopiable.h"

namespace _SimShaderAux
{
    template<ShaderStageSelector _ShaderSelector>
    class _ConstantBufferObjectBase;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferAttributes;

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferObject;

    template<ShaderStageSelector StageSelector>
    class _ConstantBufferManager;

    template<ShaderStageSelector _ShaderSelector>
    class _ConstantBufferObjectBase
    {
    public:
        _ConstantBufferObjectBase(UINT slot, ID3D11Buffer *buf)
            : slot_(slot), buf_(buf)
        {

        }

        virtual ~_ConstantBufferObjectBase(void)
        {

        }

        void Bind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            _BindConstantBuffer<_ShaderSelector>(devCon, slot_, buf_);
        }

        void Unbind(ID3D11DeviceContext *devCon) const
        {
            assert(devCon);
            _BindConstantBuffer<_ShaderSelector>(devCon, slot_, nullptr);
        }
        
    protected:
        UINT slot_;
        ID3D11Buffer *buf_;
    };

    template<typename _BufferType, ShaderStageSelector _StageSelector, bool _IsDynamic>
    class _ConstantBufferAttributes
    {
    public:
        using MyType = _ConstantBufferObject<_BufferType, _StageSelector, _IsDynamic>;
        using BufferType = _BufferType;

        static constexpr ShaderStageSelector ShaderStage = _StageSelector;
        static constexpr bool IsDynamic = _IsDynamic;
    };

    //Static constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class _ConstantBufferObject<_BufferType, _StageSelector, false>
        : public _ConstantBufferAttributes<_BufferType, _StageSelector, false>,
          public _ConstantBufferObjectBase<_StageSelector>,
          public _Uncopiable
    {
    private:
        friend class _ConstantBufferManager<_StageSelector>;

        _ConstantBufferObject(ID3D11DeviceContext *devCon, UINT slot, const _BufferType *data)
            : _ConstantBufferObjectBase(slot, nullptr)
        {
            assert(devCon && data);
            D3D11_SUBRESOURCE_DATA dataDesc = { &data, 0, 0 };
            buf_ = _GenConstantBuffer(dev, sizeof(_BufferType), true, &dataDesc);
            if(!buf_)
                throw SimShaderError("Failed to create constant buffer");
        }

        ~_ConstantBufferObject(void)
        {
            if(buf_)
                buf_->Release();
        }
    };

    //Dynamic constant buffer object
    template<typename _BufferType, ShaderStageSelector _StageSelector>
    class _ConstantBufferObject<_BufferType, _StageSelector, true>
        : public _ConstantBufferAttributes<_BufferType, _StageSelector, true>,
          public _ConstantBufferObjectBase<_StageSelector>,
          public _Uncopiable
    {
    public:
        void SetBufferData(ID3D11DeviceContext *devCon, const BufferType &data)
        {
            assert(devCon != nullptr && buf_);
            devCon->UpdateSubresource(buf_, 0, nullptr, &data, 0, 0);
        }

    private:
        friend class _ConstantBufferManager<_StageSelector>;

        _ConstantBufferObject(ID3D11Device *dev, UINT slot, const _BufferType *data = nullptr)
            : _ConstantBufferObjectBase(slot, nullptr)
        {
            assert(dev);
            if(data)
            {
                D3D11_SUBRESOURCE_DATA dataDesc = { &data, 0, 0 };
                buf_ = _GenConstantBuffer(dev, sizeof(_BufferType), true, &dataDesc);
            }
            else
                buf_ = _GenConstantBuffer(dev, sizeof(_BufferType), true, nullptr);
            if(!buf_)
                throw SimShaderError("Failed to create constant buffer");
        }

        ~_ConstantBufferObject(void)
        {
            if(buf_)
                buf_->Release();
        }
    };

    template<ShaderStageSelector StageSelector>
    class _ConstantBufferManager
    {
    public:
        _ConstantBufferManager(void) = default;

        ~_ConstantBufferManager(void)
        {
            for(const auto it : CBs_)
            {
                if(it.second.obj)
                    delete it.second.obj;
            }
        }

        void AddBuffer(const std::string &name, UINT slot, UINT byteSize)
        {
            assert(!name.empty() && byteSize > 0);
            if(CBs_.find(name) != CBs_.end())
                throw SimShaderError("Constant buffer name repeated: " + name);
            CBs_[name] = { slot, byteSize, true, nullptr };
        }

        void SetBufferImmutable(const std::string &name)
        {
            auto it = CBs_.find(name);
            if(it == CBs_.end())
                throw SimShaderError("Constant buffer not found: " + name);
            it->second.dynamic = false;
        }

        void SetBuffersImmutable(const std::vector<std::string> &names)
        {
            for(const std::string &name : names)
                SetBufferImmutable(name);
        }

        void SetBufferMutable(const std::string &name)
        {
            auto it = CBs_.find(name);
            if(it == CBs_.end())
                throw SimShaderError("Constant buffer not found: " + name);
            it->second.dynamic = true;
        }

        void SetBuffersMutable(const std::vector<std::string> &names)
        {
            for(const std::string &name : names)
                SetBufferMutable(name);
        }

        void SetAllBuffersImmutable(void)
        {
            for(auto it : CBs_)
                it->second.dynamic = false;
        }

        void SetAllBuffersMutable(void)
        {
            for(auto it : CBs_)
                it->second.dynamic = true;
        }

        template<typename BufferType, bool IsDynamic>
        _ConstantBufferObject<BufferType, StageSelector, IsDynamic> *
        GetConstantBuffer(ID3D11Device *dev, const std::string &name, const BufferType *data = nullptr)
        {
            using ResultType = _ConstantBufferObject<BufferType, StageSelector, IsDynamic>;

            assert(dev);
            assert(IsDynamic || data);

            auto it = CBs_.find(name);
            if(it == CBs_.end())
                throw SimShaderError(("Constant buffer not found: " + name).c_str());
            _CBRec &rec = it->second;

            if(rec.obj)
            {
                if(typeid(rec.obj) != typeid(_ConstantBufferObject<BufferType, StageSelector, IsDynamic>))
                    throw SimShaderError("Inconsistent constant buffer type");
                return reinterpret_cast<ResultType*>(rec.obj);
            }

            if(IsDynamic != rec.dynamic)
                throw SimShaderError("Inconsistent constant buffer usage");
            if(sizeof(BufferType) != rec.byteSize)
                throw SimShaderError("Inconstent constant buffer size");

            rec.obj = new ResultType(dev, rec.slot, data);
            return reinterpret_cast<ResultType*>(rec.obj);
        }

        void Bind(void)
        {
            for(auto it : CBs_)
            {
                if(it->second.obj)
                    it->second.obj->Bind();
            }
        }

        void Unbind(void)
        {
            for(auto it : CBs_)
            {
                if(it->second.obj)
                    it->second.obj->Unbind();
            }
        }

    private:
        struct _CBRec
        {
            UINT slot;
            UINT byteSize;
            bool dynamic;
            _ConstantBufferObjectBase<StageSelector> *obj;
        };
        using CBTable = std::map<std::string, _CBRec>;

        CBTable CBs_;
    };
}

#endif //__SIMSHADER_CONSTANT_BUFFER_H__
