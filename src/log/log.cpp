#include "log.h"


extern "C" {

bool log_get_message(char* buf, int len) {
    if (Log::_stream.getline(buf, len)) {
        return true;
    }
    else {
        Log::_stream.clear();
        return false;
    }
} 

}

