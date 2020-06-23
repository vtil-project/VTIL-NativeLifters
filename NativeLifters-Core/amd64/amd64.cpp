#include "amd64.hpp"
#include "flags.hpp"

#include <vtil/amd64>

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
		void initialize_branch( );
		void initialize_comparison( );
		void initialize_flags( );
		void initialize_misc( );

		void initialize_mappings( )
		{
			initialize_arithmetic( );
			initialize_branch( );
			initialize_comparison( );
			initialize_flags( );
			initialize_misc( );
		}

		int lifter_t::process( basic_block* block, uint64_t vip, uint8_t* code )
		{
			const auto& insns = vtil::amd64::disasm( code, vip, 0 );
			if ( insns.empty( ) )
			{
				operand_mappings[ X86_INS_INVALID ]( block, { } );
				return 0;
			}

			const auto& insn = insns[ 0 ];
			code += insn.bytes.size( );

			batch_translator translator = { block };
			vtil::lifter::operative::translator = &translator;

			block->mov( X86_REG_RIP, vip + insn.bytes.size( ) );

			x86_insn opcode = static_cast< x86_insn >( insn.id );
			auto mapping = operand_mappings.find( opcode ); 

			auto is_invalid = mapping == operand_mappings.cend( );
			if ( !is_invalid )
			{
				for ( auto& operand : insn.operands )
				{
					if ( operand.type == X86_OP_REG && !vtil::amd64::is_generic( operand.reg ) )
					{
						is_invalid = true;
						break;
					}

					if ( operand.type == X86_OP_MEM )
					{
						if ( operand.mem.base != X86_OP_INVALID && !vtil::amd64::is_generic( operand.mem.base ) )
						{
							is_invalid = true;
							break;
						}

						if ( operand.mem.index != X86_OP_INVALID && !vtil::amd64::is_generic( operand.mem.index ) )
						{
							is_invalid = true;
							break;
						}
					}
				}
			}

			if ( is_invalid )
			{
				for ( auto& operand : insn.operands )
				{
					if ( operand.type == X86_OP_REG && ( operand.access & CS_AC_READ ) )
						block->vpinr( operand.reg );

					if ( operand.type == X86_OP_MEM )
					{
						if ( operand.mem.base != X86_OP_INVALID )
							block->vpinr( operand.mem.base );

						if ( operand.mem.index != X86_OP_INVALID )
							block->vpinr( operand.mem.index );
					}
				}

				for ( auto byte : insn.bytes )
					block->vemit( byte );

				for ( auto& operand : insn.operands )
					if ( operand.type == X86_OP_REG && ( operand.access & CS_AC_WRITE ) )
						block->vpinw( operand.reg );

				if ( insn.eflags & X86_EFLAGS_MODIFY_CF )
					block->vpinw( flags::CF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_DF )
					block->vpinw( flags::DF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_OF )
					block->vpinw( flags::OF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_ZF )
					block->vpinw( flags::ZF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_PF )
					block->vpinw( flags::PF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_AF )
					block->vpinw( flags::AF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_SF )
					block->vpinw( flags::SF );

				if ( insn.eflags & X86_EFLAGS_MODIFY_IF )
					block->vpinw( flags::IF );

				// Once again, our "undefined" behavior is that undefined flags stay exactly the same. :)
			}
			else
			{
				// Call the instruction handler.
				mapping->second( block, insn );
			}

			return insn.bytes.size( );
		}
	}
}