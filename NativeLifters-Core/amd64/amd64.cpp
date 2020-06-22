#include <cstdint>

#include "amd64.hpp"
#include "flags.hpp"

namespace vtil::lifter
{
	namespace amd64
	{
		// Create a series of instructions representing memory displacement given the current operand and basic block.
		// 
		register_desc get_disp_from_operand( basic_block* block, const operand_info& operand )
		{
			// Create a temporary register to store the current displacement.
			auto current_offs = block->tmp( 64 );

			// Zero it out, in case one of these doesn't actually have a value and we add junk data.
			block
				->mov( current_offs, 0ULL );

			if ( operand.mem.index != X86_REG_INVALID )
			{
				block
					// Move the index register into the offset
					->mov( current_offs, operand.mem.index )

					// Multiply it by the scale (unsigned).
					->mul( current_offs, operand.mem.scale );
			}

			if ( operand.mem.base != X86_REG_INVALID )
			{
				block
					// Add the base offset.
					->add( current_offs, operand.mem.base );
			}

			block
				// Add the displacement offset.
				->add( current_offs, operand.mem.disp );

			// Return resulting temporary.
			return current_offs;
		}

		// Load a register, immediate, or memory operand from the given instruction and operand index.
		//
		operand load_operand( basic_block* block, const instruction_info& insn, size_t idx )
		{
			// Grab the operand at the current index.
			const auto& opr = insn.operands[ idx ];

			// Check the type and return relevant value accordingly.
			switch ( opr.type )
			{
				case X86_OP_IMM:
					return { opr.imm, opr.size * 8 };
				case X86_OP_REG:
					return { opr.reg };
				case X86_OP_MEM:
				{
					auto tmp = block->tmp( opr.size * 8 );
					block
						->ldd( tmp, get_disp_from_operand( block, opr ), 0 );
					return { tmp };
				}
				default:
					fassert( false );
			}
		}

		// Store to a register or memory operand given the instruction, block, operand index, and source operand.
		//
		void store_operand( basic_block* block, const instruction_info& insn, size_t idx, const operand& source )
		{
			// Grab the operand at the current index.
			const auto& opr = insn.operands[ idx ];

			// Check the type and return relevant value accordingly.
			switch ( opr.type )
			{
				case X86_OP_REG:
					block
						->mov( opr.reg, source );
					break;
				case X86_OP_MEM:
				{
					block
						->str( get_disp_from_operand( block, opr ), 0, source );
					break;
				}
				case X86_OP_IMM:
				default:
					fassert( false );
			}
		}

		void initialize_arithmetic( );
		void initialize_misc( );

		void initialize_mappings( )
		{
			initialize_arithmetic( );
			initialize_misc( );
		}

		void process( vtil::amd64::instruction& insn, basic_block* block )
		{
			batch_translator translator = { block };
			vtil::lifter::operative::translator = &translator;

			x86_insn opcode = static_cast< x86_insn >( insn.id );
			if ( auto mapping = operand_mappings.find( opcode ); mapping == operand_mappings.cend( ) )
				operand_mappings[ X86_INS_INVALID ]( block, insn );
			else
				mapping->second( block, insn );
		}
	}
}