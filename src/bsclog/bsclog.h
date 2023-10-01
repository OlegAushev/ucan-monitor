#pragma once


#include <string>
#include <vector>


class bsclog {
private:
    //static inline std::vector<std::ostream&> _ostreams;
public:
    static void add_sink(std::ostream& sink);
};
