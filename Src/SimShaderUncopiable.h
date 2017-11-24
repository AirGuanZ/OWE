/*================================================================
Filename: SimShaderUncopiable.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_UNCOPIABLE_H__
#define __SIMSHADER_UNCOPIABLE_H__

namespace _SimShaderAux
{
    class _Uncopiable
    {
    public:
        _Uncopiable(void) = default;
        _Uncopiable(const _Uncopiable &) = delete;
        _Uncopiable operator=(const _Uncopiable &) = delete;
    };
}

#endif //__SIMSHADER_UNCOPIABLE_H__
