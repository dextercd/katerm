#ifndef GDTERM_PROGRAM_HPP
#define GDTERM_PROGRAM_HPP

#include <cstdint>

namespace katerm {

class program {
public:
    virtual void handle_bytes(char const*, std::size_t, bool more_data_coming) = 0;
    virtual ~program() = default;
};

} // katerm::

#endif // header guard
