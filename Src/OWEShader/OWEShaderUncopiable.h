/*================================================================
Filename: OWEShaderUncopiable.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __OWESHADER_UNCOPIABLE_H__
#define __OWESHADER_UNCOPIABLE_H__

namespace OWEShaderAux
{
    class Uncopiable
    {
    public:
        Uncopiable(void) = default;
        Uncopiable(const Uncopiable &) = delete;
        Uncopiable operator=(const Uncopiable &) = delete;
    };
}

#endif //__OWESHADER_UNCOPIABLE_H__
