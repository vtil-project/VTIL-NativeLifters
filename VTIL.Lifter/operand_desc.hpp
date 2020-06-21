#pragma once
#include <optional>
#include <vtil/utility>

namespace vtil::lifter
{
    // Specifies types of operands, optionally as flags
    //
    enum operand_type : uint32_t
    {
        operand_type_none = 1 << 0,

        operand_type_register = 1 << 1,
                
        operand_type_memory = 1 << 2,
                
        operand_type_imm = 1 << 3,
    };

    // Describes requirements an operand must fulfill
    //
    struct operand_desc
    {
        // The operand types accepted by this descriptor
        //
        operand_type types;

        // Optionally, the required size of the operand
        //
        std::optional<bitcnt_t> size;

        operand_desc( operand_type types )
            : types( types )
        {}

        operand_desc( operand_type type, bitcnt_t size )
            : types( types ), size( size )
        {}
    };
}