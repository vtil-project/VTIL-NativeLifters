#include "amd64.hpp"
#include "flags.hpp"

// Instruction semantics defined here.
#include "semantic/arithmetic.hpp"
#include "semantic/invalid.hpp"

namespace vtil::lifter::amd64
{
	// Create a series of instructions representing memory displacement given the current operand and basic block.
	// 
	register_desc get_disp_from_operand( basic_block* block, const operand_info& operand )
	{
		// Create a temporary register to store the current displacement.
		auto current_offs = block->tmp( );

		block
			// Move the index register into the offset
			->mov( current_offs, operand.mem.index )

			// Multiply it by the scale (unsigned).
			->mul( current_offs, operand.mem.scale );

			// Add the base offset.
			->add( current_offs, operand.mem.base )

			// Add the displacement offset.
			->add( current_offs, operand.mem.disp );

		// Return resulting temporary.
		return current_offs;
	}

	// Get a register, immediate, or memory operand from the given capstone instruction.
	//
	operand get_operand( basic_block* block, const instruction_info& insn, size_t idx )
	{
		// Grab the op	erand at the current index.
		const auto& operand = insn.operands[ idx ];

		// Check the type and return relevant value accordingly.
		switch ( operand->type )
		{
			case X86_OP_IMM:
				return operand.imm;
			case X86_OP_REG:
				return operand.reg;
			case X86_OP_MEM:
				return get_disp_from_operand( block, operand );
		}
	}

	void process( x86_insn insn, basic_block* block, const instruction_info& insn )
	{
		switch ( insn )
		{
			case X86_INS_ADD:
				process_add( block, insn );
				break;

			case X86_INS_SUB:
				process_add( block, insn );
				break;

			default:
				process_invalid( block );
				break;
		}
	}
}