#pragma once
#include <memory>
#include "operand_desc.hpp"
#include "arch.hpp"

namespace vtil::lifter
{
    // An arch-independent interpretation of a native operand
    //
    struct native_operand
    {
        // The type of the operand
        // NOTE: This can only be a single flag
        //
        operand_type type;

        // The size of the operand, in bits
        //
        bitcnt_t size;

        // Contains arch-specific information about the operand
        //
        std::unique_ptr<operand_info> detail;

        // Default constructor / move.
        //
        native_operand( native_operand&& ) = default;
        native_operand& operator=( native_operand&& ) = default;

        native_operand( operand_type type, bitcnt_t size, std::unique_ptr<operand_info> detail )
            : type( type ), size( size ), detail( std::move( detail ) )
        {}
    };
}