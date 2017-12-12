/*================================================================
Filename: SimShaderFatalError.h
Date: 2017.11.24
Created by AirGuanZ
================================================================*/
#ifndef __SIMSHADER_FATAL_ERROR_H__
#define __SIMSHADER_FATAL_ERROR_H__

#include <stdexcept>
#include <string>

namespace _SimShaderAux
{
    class SimShaderError : public std::exception
    {
    public:
        SimShaderError(const char *msg)
            : exception(msg)
        {

        }

        SimShaderError(const std::string &msg)
            : exception(msg.c_str())
        {

        }

        const char *what(void) const
        {
            return std::exception::what();
        }
    };
}

#endif //__SIMSHADER_FATAL_ERROR_H__
