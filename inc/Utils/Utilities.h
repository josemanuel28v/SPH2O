#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <string>

namespace Utilities
{
    bool isNumber(const std::string& str) 
    {
        for (char c : str) 
        {
            if (!std::isdigit(c)) 
            {
                return false;
            }
        }
        return true;
    }
}

#endif