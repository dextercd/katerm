#ifndef KATERM_POSITION_HPP
#define KATERM_POSITION_HPP

#include <iosfwd>

namespace katerm {

struct position {
    int x;
    int y;

    friend bool operator==(position left, position right)
    {
        return left.x == right.x && left.y == right.y;
    }

    friend bool operator!=(position left, position right)
    {
        return !(left == right);
    }
};

std::ostream& operator<<(std::ostream&, position);

} // katerm::

#endif // header guard
