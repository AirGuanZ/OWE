/*================================================================
Filename: SimShader.h
Date: 2017.12.6
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_H__
#define __SIMSHADER_H__

#include <tuple>
#include <utility>

#include "SimShaderStage.h"

namespace _SimShaderAux
{
    template<typename Func, typename Tu, size_t FI>
    inline void _DoForTupleElements(Func func, Tu &tu, std::index_sequence<FI>)
    {
        func(std::get<FI>(tu));
    }

    template<typename Func, typename Tu, size_t FI, size_t...OI>
    inline void _DoForTupleElements(Func func, Tu &tu, std::index_sequence<FI, OI...>)
    {
        func(std::get<FI>(tu));
        _DoForTupleElements<Func, Tu, OI...>(func, tu, std::index_sequence<OI...>());
    }

    template<typename Func, typename Tu>
    inline void DoForTupleElements(Func func, Tu &tu)
    {
        _DoForTupleElements<Func, Tu>(func, tu, std::make_index_sequence<std::tuple_size<Tu>::value>());
    }

    template<int v>
    inline constexpr int _FindInNumListAux(void)
    {
        return v < 0 ? v : (v + 1);
    }

    template<typename NumType, NumType Dst>
    inline constexpr int FindInNumList(void)
    {
        return -1;
    }

    template<typename NumType, NumType Dst, NumType First, NumType...Others>
    inline constexpr int FindInNumList(void)
    {
        return Dst == First ? 0 : _FindInNumListAux<FindInNumList<NumType, Dst, Others...>()>();
    }

    template<typename NumType>
    inline constexpr bool IsRepeated(void)
    {
        return false;
    }

    template<typename NumType, NumType First, NumType...Others>
    inline constexpr bool IsRepeated(void)
    {
        return (FindInNumList<NumType, First, Others...>() >= 0) || IsRepeated<NumType, Others...>();
    }

    struct _ShaderStagePtrInitializer
    {
        template<typename T>
        void operator()(T *&ptr)
        {
            ptr = nullptr;
        }
    };

    struct _ShaderStageDeleter
    {
        template<typename T>
        void operator()(T *&ptr)
        {
            SafeDeleteObjects(ptr);
        }
    };

    struct _ShaderStageBinder
    {
        template<typename T>
        void operator()(T *pStage)
        {
            assert(pStage != nullptr);
            pStage->Bind(DC_);
        }
        ID3D11DeviceContext *DC_;
    };

    struct _ShaderStageAvailableRec
    {
        template<typename T>
        void operator()(const T *ptr)
        {
            available = available && (ptr != nullptr);
        }

        bool available = true;
    };

    struct _ShaderStageUnbinder
    {
        template<typename T>
        void operator()(T *pStage)
        {
            assert(pStage != nullptr);
            pStage->Unbind(DC_);
        }
        ID3D11DeviceContext *DC_;
    };

    template<ShaderStageSelector...StageSelectors>
    class _Shader
    {
    public:
        _Shader(void)
        {
            assert((IsRepeated<ShaderStageSelector, StageSelectors...>() == false));
            assert((FindInNumList<ShaderStageSelector, SS_VS, StageSelectors...>() != -1));
            assert((FindInNumList<ShaderStageSelector, SS_PS, StageSelectors...>() != -1));

            _ShaderStagePtrInitializer setter;
            DoForTupleElements(setter, stages_);
        }

        ~_Shader(void)
        {
            DestroyAllStages();
        }

        template<ShaderStageSelector StageSelector>
        void InitStage(ID3D11Device *dev,
                       const std::string &src, std::string *errMsg = nullptr,
                       const std::string &target = _ShaderStage<StageSelector>::StageSpec::DefaultCompileTarget(),
                       const std::string &entry = "main")
        {
            auto &pStage = std::get<FindInNumList<ShaderStageSelector, StageSelector, StageSelectors>()>(stages_);
            SafeDelObjects(pStage);
            pStage = new _ShaderStage<StageSelector>(dev, src, errMsg, target, entry);
        }

        template<ShaderStageSelector StageSelector>
        void InitStage(ID3D11Device *dev, ID3D10Blob *shaderByteCode)
        {
            auto &pStage = std::get<FindInNumList<ShaderStageSelector, StageSelector, StageSelectors>()>(stages_);
            SafeDelObjects(pStage);
            pStage = new _ShaderStage<StageSelector>(dev, shaderByteCode);
        }

        void DestroyAllStages(void)
        {
            _ShaderStageDeleter deleter;
            DoForTupleElements(deleter, stages_);
        }

        bool IsAllStagesAvailable(void) const
        {
            _ShaderStageAvailableRec rec;
            DoForTupleElements(rec, stages_);
            return rec.available;
        }

        template<ShaderStageSelector StageSelector>
        _ShaderStage<StageSelector> *GetStage(void)
        {
            return std::get<FindInNumList<ShaderStageSelector, StageSelector, StageSelectors>()>(stages_);
        }

        void BindStages(ID3D11DeviceContext *DC)
        {
            _ShaderStageBinder binder = { DC };
            DoForTupleElements(binder, stages_);
        }

        void UnbindStages(ID3D11DeviceContext *DC)
        {
            _ShaderStageUnbinder unbinder = { DC };
            DoForTupleElements(unbinder, stages_);
        }

    private:
        std::tuple<_ShaderStage<StageSelectors>*...> stages_;
    };
}

#endif //__SIMSHADER_H__
