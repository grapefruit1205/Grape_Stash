#pragma once
#include <fstream>
#include <string>
using namespace std;
namespace yazi{
    namespace utility{

        class logger
        {
            public:
                enum Level{DEBUG = 0,INFO,WARN,ERROR,FATAL,LEVEL_COUNT};
                void log(Level level, const char* file, int line, const char* format,...);
            private:
                logger();
                ~logger();
            private:
                string m_filename;
                ofstream m_fout;
                static const char* s_level[LEVEL_COUNT];
        };
    }
}
