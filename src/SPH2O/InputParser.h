#include <string>
#include <vector>
#include <algorithm>
#include "../Common/numeric_types.h"

using namespace std;

class InputParser
{
    public:

        InputParser (int argc, char *argv[])
        {
            for (int i = 1; i < argc; ++i)
                tokens.push_back(string(argv[i]));
        }

        static string getCmdOption(string option) 
        {
            vector<string>::const_iterator itr;
            itr =  find(tokens.begin(), tokens.end(), option);

            if (itr != tokens.end() && ++itr != tokens.end())
                return *itr;
            else
                return string();
        }

        static bool cmdOptionExists(string option) 
        {
            return find(tokens.begin(), tokens.end(), option) != tokens.end();
        }

        static uint size()
        {
            return static_cast<uint>(tokens.size());
        }

    private:

        inline static vector<string> tokens;      // Vector con las cadenas del array de argumentos
};