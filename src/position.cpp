#include <iostream>

#include <katerm/position.hpp>

namespace gd100 {

std::ostream& operator<<(std::ostream& os, position const p)
{
    return os << '(' << p.x << "; " << p.y << ')';
}

} // gd100::
