/*================================================================
Filename: OWEShaderUncopiable.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __OWESHADER_UNCOPIABLE_H__
#define __OWESHADER_UNCOPIABLE_H__

namespace _OWEShaderAux
{
    class _Uncopiable
    {
    public:
        _Uncopiable(void) = default;
        _Uncopiable(const _Uncopiable &) = delete;
        _Uncopiable operator=(const _Uncopiable &) = delete;
    };
}

#endif //__OWESHADER_UNCOPIABLE_H__
