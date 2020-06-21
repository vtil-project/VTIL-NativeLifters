#pragma once
#include <cstdint>
#include <vtil/utility>
#include <vtil/arch>
#include "../operations.hpp"
#include "../operative.hpp"

namespace vtil::lifter
{
    namespace amd64::flags
    {
        // Individual flag registers
        //
        static const vtil::register_desc CF = { vtil::register_physical | vtil::register_flags, 0, 1, 0 };
        static const vtil::register_desc PF = { vtil::register_physical | vtil::register_flags, 0, 1, 2 };
        static const vtil::register_desc AF = { vtil::register_physical | vtil::register_flags, 0, 1, 4 };
        static const vtil::register_desc ZF = { vtil::register_physical | vtil::register_flags, 0, 1, 6 };
        static const vtil::register_desc SF = { vtil::register_physical | vtil::register_flags, 0, 1, 7 };
        static const vtil::register_desc DF = { vtil::register_physical | vtil::register_flags, 0, 1, 10 };
        static const vtil::register_desc OF = { vtil::register_physical | vtil::register_flags, 0, 1, 11 };

        // Clears all tracked bits, deeming them undefined
        //
        static void clear(vtil::basic_block* block)
        {
            block
                ->mov( CF, UNDEFINED )
                ->mov( PF, UNDEFINED )
                ->mov( AF, UNDEFINED )
                ->mov( ZF, UNDEFINED )
                ->mov( SF, UNDEFINED )
                ->mov( DF, UNDEFINED )
                ->mov( OF, UNDEFINED );
        }

        // Checks ZF
        //
        template <typename T>
        static T zero( T value )
        {
            return value == 0;
        }

        // Checks AF
        //
        template <typename T, typename R>
        static T aux_carry( T lhs, R rhs, R result )
        {
            return ( lhs ^ rhs ^ result ) & 0x10;
        }

        // Checks AF
        //
        template <typename T, typename R>
        static T aux_carry( T lhs, R rhs, R carry, R result )
        {
            return ( lhs ^ rhs ^ carry ^ result ) & 0x10;
        }

        // Checks SF
        //
        template <typename T>
        static bool sign( T value )
        {
            return get_sign( value ) & 1;
        }

        // Checks PF
        //
        template <typename T>
        static bool parity( T value )
        {
            return !( popcnt( value ) & 1 );
        }

        // Specifies the type of operation flags should be determined for
        //
        enum flag_operation : uint32_t
        {
            flag_operation_add,

            flag_operation_sub,

            flag_operation_div,

            flag_operation_mul,

            flag_operation_idiv,

            flag_operation_imul,
        };

        // Overflow bit varies per operation, so it cannot have a generic computation
        //
        template <flag_operation Op>
        struct overflow;

        // Signed / Unsigned Add
        //
        template<>
        struct overflow<flag_operation_add>
        {
            template <typename T>
            static T flag( T lhs, T rhs, T result )
            {
                // Compute the sign bits from each value
                //
                T lhs_sign = get_sign( lhs );
                T rhs_sign = get_sign( rhs );
                T res_sign = get_sign( result );

                return ( lhs_sign ^ res_sign ) + ( rhs_sign ^ res_sign ) == 2;
            }
        };

        // Signed / Unsigned Subtract
        //
        template<>
        struct overflow<flag_operation_sub>
        {
            template <typename T>
            static T flag( T lhs, T rhs, T result )
            {
                // Compute the sign bits from each value
                //
                T lhs_sign = get_sign( lhs );
                T rhs_sign = get_sign( rhs );
                T res_sign = get_sign( result );

                return ( lhs_sign ^ rhs_sign ) + ( lhs_sign ^ res_sign ) == 2;
            }
        };

        // Unsigned Multiplication
        //
        template<>
        struct overflow<flag_operation_mul>
        {
            template <typename T>
            static T flag( T first, T second, T result_upper, T result_lower )
            {
                return result_upper != 0;
            }
        };

        // Signed Multiplication
        //
        template<>
        struct overflow<flag_operation_imul>
        {
            // TODO: actually implement this

            template <typename T>
            static T flag( T first, T second, T result_upper, T result_lower )
            {
                return first;
            }

            template <typename T>
            static T flag( T first, T second, T result )
            {
                return first;
            }
        };

        // Carry bit varies per operation, so it cannot have a generic computation
        //
        template <flag_operation Op>
        struct carry;

        // Signed / Unsigned Add
        //
        template<>
        struct carry<flag_operation_add>
        {
            template <typename T>
            static T flag( T lhs, T rhs, T result )
            {
                return result < lhs || result < rhs
            }
        };

        // Signed / Unsigned Subtract
        //
        template<>
        struct carry<flag_operation_sub>
        {
            template <typename T>
            static T flag( T lhs, T rhs, T result )
            {
                return lhs < rhs;
            }
        };

        // Unsigned Multiplication
        //
        template<>
        struct carry<flag_operation_mul>
        {
            template <typename T>
            static T flag( T first, T second, T result_upper, T result_lower )
            {

            }
        };

        // Signed Multiplication
        //
        template<>
        struct carry<flag_operation_imul>
        {
            template <typename T>
            static T flag( T first, T second, T result_upper, T result_lower )
            {

            }

            template <typename T>
            static T flag( T first, T second, T result )
            {

            }
        };
    }
}