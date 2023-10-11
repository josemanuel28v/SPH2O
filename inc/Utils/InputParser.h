#ifndef _INPUT_PARSER_H_
#define _INPUT_PARSET_H_

#include <string>
#include <vector>
#include <algorithm>

#include "Common/numeric_types.h"

class InputParser
{
    public:

        InputParser (int argc, char *argv[])
        {
            for (int i = 1; i < argc; ++i)
                tokens.push_back(std::string(argv[i]));
        }

        static std::string getCmdOption(const std::string& option) 
        {
            std::vector<std::string>::const_iterator itr;
            itr =  find(tokens.begin(), tokens.end(), option);

            if (itr != tokens.end() && ++itr != tokens.end())
                return *itr;
            else
                return std::string();
        }

        static bool cmdOptionExists(const std::string& option) 
        {
            return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
        }

        static uint size()
        {
            return static_cast<uint>(tokens.size());
        }

    private:

        inline static std::vector<std::string> tokens;      // Vector con las cadenas del array de argumentos
};

#endif