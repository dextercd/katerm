#include <iostream>

#include <katerm/position.hpp>

namespace katerm {

std::ostream& operator<<(std::ostream& os, position const p)
{
    return os << '(' << p.x << "; " << p.y << ')';
}

} // katerm::
