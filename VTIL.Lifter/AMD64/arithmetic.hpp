#pragma once
#include <vtil/arch>
#include "amd64.hpp"
#include "../semantic.hpp"
#include "../operative.hpp"
#include "flags.hpp"

namespace vtil::lifter
{
    namespace amd64
    {
        inline const semantic add_reg_reg =
        {
            // Identifier
            X86_INS_ADD,

            // Operand Descriptors
            {
                { operand_type_register }, { operand_type_register }
            },

            // Flags
            semantic_none,

            // Application delegate
            []( vtil::basic_block* block, const std::vector<native_operand>& operands )
            {
                // Define result
                //
                operative result = { operands[ 0 ].detail->reg, block };

                // Define & clone writeable lhs, define rhs
                //
                operative lhs = { operands[ 0 ].detail->reg, block, true };
                operative rhs = { operands[ 1 ].detail->reg, block };

                // Perform operation
                //
                result = lhs + rhs;
                
                // Update flags
                //
                operative{ flags::OF, block } = flags::overflow<flags::flag_operation_add>::flag( lhs, rhs, result );
                operative{ flags::CF, block } = flags::carry<flags::flag_operation_add>::flag( lhs, rhs, result );
                operative{ flags::SF, block } = flags::sign( result );
                operative{ flags::ZF, block } = flags::zero( result );
                operative{ flags::AF, block } = flags::aux_carry( lhs, rhs, result );
                operative{ flags::PF, block } = flags::parity( result );
            }
        };
    }
}