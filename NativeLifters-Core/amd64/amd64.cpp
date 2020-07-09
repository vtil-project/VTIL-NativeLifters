// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#include "amd64.hpp"
#include "flags.hpp"

namespace vtil::lifter::amd64
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
				unreachable();
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
			{
				operand op = { ( x86_reg ) opr.reg };
				if ( op.bit_count() == 32 )
				{
					operand op_hi = op;
					op_hi.reg().bit_offset += 32;
					block->mov( op_hi, 0ull );
				}
				block->mov( op, source );
				break;
			}
			case X86_OP_MEM:
			{
				block->str( get_disp_from_operand( block, opr ), 0, source );
				break;
			}
			case X86_OP_IMM:
			default:
				unreachable();
		}
	}

	size_t lifter_t::process( basic_block* block, uint64_t vip, uint8_t* code )
	{
		const auto& insns = vtil::amd64::disasm( code, vip, 0 );
		if ( insns.empty() )
		{
			handle_instruction( block, { .id = X86_INS_INVALID } );
			return 0;
		}

		const auto& insn = insns[ 0 ];
		code += insn.bytes.size();

		batch_translator translator = { block };
		lifter::operative::translator = &translator;

		block->mov( X86_REG_RIP, vip + insn.bytes.size() );

		// Validate operands:
		//
		bool is_invalid = false;
		for ( auto& operand : insn.operands )
		{
			if ( operand.type == X86_OP_REG && !vtil::amd64::is_generic( operand.reg ) )
			{
				is_invalid = true;
				break;
			}

			if ( operand.type == X86_OP_MEM )
			{
				if ( operand.mem.base != X86_REG_INVALID && !vtil::amd64::is_generic( operand.mem.base ) )
				{
					is_invalid = true;
					break;
				}

				if ( operand.mem.index != X86_REG_INVALID && !vtil::amd64::is_generic( operand.mem.index ) )
				{
					is_invalid = true;
					break;
				}
			}
		}
		
		// If is invalid or could not handle:
		//
		if ( is_invalid || !handle_instruction( block, insn ) )
		{
			// Hint all side effects.
			//
			for ( auto& operand : insn.operands )
			{
				if ( operand.type == X86_OP_REG && ( operand.access & CS_AC_READ ) )
					block->vpinr( operand.reg );

				if ( operand.type == X86_OP_MEM )
				{
					if ( operand.mem.base != X86_REG_INVALID )
						block->vpinr( operand.mem.base );

					if ( operand.mem.index != X86_REG_INVALID )
						block->vpinr( operand.mem.index );
				}
			}

			for ( auto byte : insn.bytes )
				block->vemit( byte );

			for ( auto& operand : insn.operands )
				if ( operand.type == X86_OP_REG && ( operand.access & CS_AC_WRITE ) )
					block->vpinw( operand.reg );

			if ( insn.eflags & X86_EFLAGS_MODIFY_CF ) block->vpinw( flags::CF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_DF ) block->vpinw( flags::DF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_OF ) block->vpinw( flags::OF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_ZF ) block->vpinw( flags::ZF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_PF ) block->vpinw( flags::PF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_AF ) block->vpinw( flags::AF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_SF ) block->vpinw( flags::SF );
			if ( insn.eflags & X86_EFLAGS_MODIFY_IF ) block->vpinw( flags::IF );
		}

		// Enforce undefined bits.
		//
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_OF ) block->mov( flags::OF, UNDEFINED );
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_SF ) block->mov( flags::SF, UNDEFINED );
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_ZF ) block->mov( flags::ZF, UNDEFINED );
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_PF ) block->mov( flags::PF, UNDEFINED );
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_AF ) block->mov( flags::AF, UNDEFINED );
		if ( insn.eflags & X86_EFLAGS_UNDEFINED_CF ) block->mov( flags::CF, UNDEFINED );

		return insn.bytes.size();
	}
};