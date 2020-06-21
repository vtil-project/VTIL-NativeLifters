#pragma once

#include <capstone/capstone.h>

// This file defines any global arch-specific information for the AMD64 target
// architecture.
//
namespace vtil
{
    // Allows creation of register_desc's from the x86_reg enumeration
    //
    template<>
    struct register_cast< x86_reg >
    {
        register_desc operator()( x86_reg value )
        {
            auto [base, offset, size] = amd64::resolve_mapping( value );
            return register_desc( register_physical, base, size * 8, offset * 8 );
        }
    };

    namespace lifter
    {
        using instruction_info = cs_insn;
        using operand_info = cs_x86_op;
    }
}