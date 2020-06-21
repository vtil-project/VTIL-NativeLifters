#pragma once
#include <type_traits>
#include <vtil/utility>

namespace vtil::lifter
{
    // Determines the integer's size in bits
    //
    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    bitcnt_t op_size()
    {
        return sizeof T * 8;
    }

    // Counts the number of set bits in the integer
    //
    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T popcnt( T value )
    {
        return __popcnt( value );
    }

    // Sign extends the specifies integer
    //
    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    T signed_extend( T value )
    {

    }

    // Retrieves the most-significant bit
    //
    template <typename T>
    static T get_sign( T value )
    {
        return value >> ( op_size( value ) - 1 );
    }
}