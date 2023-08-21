#include "logger.h"
using namespace yazi::utility;

const char* logger::s_level[LEVEL_COUNT] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

logger::logger()
{

}

logger::~logger()
{

}
