/*================================================================
Filename: SimShaderReleaseCOMObjects.h
Date: 2017.12.6
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_RELEASE_COM_OBJECTS_H__
#define __SIMSHADER_RELEASE_COM_OBJECTS_H__

namespace _SimShaderAux
{
    template<typename T>
    void SafeDeleteObjects(T &ptr)
    {
        if(ptr)
        {
            delete ptr;
            ptr = nullptr;
        }
    }

    template<typename T, typename... Others>
    void SafeDeleteObjects(T &ptr, Others&... others)
    {
        SafeDeleteObjects(ptr);
        SafeDeleteObjects(others...);
    }

    template<typename T>
    void ReleaseCOMObjects(T &ptr)
    {
        if(ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

    template<typename T, typename... Others>
    void ReleaseCOMObjects(T &ptr, Others&... others)
    {
        ReleaseCOMObjects(ptr);
        ReleaseCOMObjects(others...);
    }
}

#endif //__SIMSHADER_RELEASE_COM_OBJECTS_H__
