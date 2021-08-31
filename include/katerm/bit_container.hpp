#ifndef KATERM_BIT_CONTAINER_HPP
#define KATERM_BIT_CONTAINER_HPP

#include <type_traits>

namespace katerm {

template<class bit_type>
class bit_container
{
    using data_type = std::underlying_type_t<bit_type>;
    data_type data{};

public:
    bit_container() = default;
    bit_container(bit_type bit)
    {
        data = static_cast<data_type>(bit);
    }

    void set(bit_container bits)
    {
        data |= bits.data;
    }

    void set(bit_container bits, bool set_)
    {
        if (set_)
            set(bits);
        else
            unset(bits);
    }


    bool is_set(bit_container bits) const
    {
        return (data & bits.data) == bits.data;
    }

    void unset(bit_container bits)
    {
        data &= ~bits.data;
    }

    friend bool operator==(bit_container left, bit_container right)
    {
        return left.data == right.data;
    }

    friend bool operator!=(bit_container left, bit_container right)
    {
        return !(left == right);
    }
};

} // katerm::

#endif // header guard
