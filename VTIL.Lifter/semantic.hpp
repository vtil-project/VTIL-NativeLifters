#pragma once
#include <cstdint>
#include <string>
#include <initializer_list>
#include <vtil/arch>
#include "native_operand.hpp"
#include "operand_desc.hpp"

namespace vtil::lifter
{
    // Arch-independent unique id for identifying an instruction
    //
    using ins_id = uint32_t;

    // Defines any flags that a semantic may have
    //
    enum semantic_flag : uint32_t
    {
        semantic_none = 1 << 0,

        semantic_halts = 1 << 1,

        semantic_modifies_ip = 1 << 2,

        semantic_emits = 1 << 3,
    };

    // This type describes the logic and semantics of any instruction to be lifted
    //
    struct semantic : reducable<semantic>
    {
        using fn_apply = void( * )( vtil::basic_block* block, const std::vector<native_operand>& operands );

        // Unique per-arch id
        //
        ins_id id;

        // Describes each operand type
        //
        const std::vector<operand_desc> operands;

        // Various flags for describing semantic's behaviour
        //
        semantic_flag flags;

        // Apply delegate
        //
        fn_apply apply;

        semantic( ins_id id, std::initializer_list<operand_desc> operands, semantic_flag flags, fn_apply apply )
            : id( id ), operands( operands ), flags( flags ), apply( apply )
        {}

        // Reduction
        //
        auto reduce() { return reference_as_tuple( id, operands ); }
    };
}