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
// 3. Neither the name of mosquitto nor the names of its   
//    contributors may be used to endorse or promote products derived from   
//    this software without specific prior written permission.   
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
#include "../amd64.hpp"
#include "../flags.hpp"

namespace vtil::lifter::amd64
{
	// List of handlers.
	//
	handler_map_t misc_handlers = {
		{
			X86_INS_INVALID,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block->vemits( "int 0xB" );
				block->vexit( invalid_vip );
			}
		},
		{
			X86_INS_ENTER,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto alloc_size = insn.operands[ 0 ].imm;
				auto nesting_level = insn.operands[ 1 ].imm % 32;

				//
				// LOCK prefix #UD in protected-mode.
				//
				if ( insn.prefix[ 0 ] == X86_PREFIX_LOCK )
					block->vemits( "int 0xB" );

				auto op_size = insn.addr_size * 8;
				auto frame_tmp = block->tmp( op_size );
				x86_reg spr, fpr;

				switch ( op_size )
				{
					case 16:
						spr = X86_REG_SP;
						fpr = X86_REG_BP;
						break;
					case 32:
						spr = X86_REG_ESP;
						fpr = X86_REG_EBP;
						break;
					case 64:
						spr = X86_REG_RSP;
						fpr = X86_REG_RBP;
						break;
				}

				block
					->push( fpr )
					->mov( spr, spr )
					->mov( frame_tmp, spr );

				if ( nesting_level == 0 )
				{
					block
						->mov( fpr, frame_tmp )
						->sub( spr, alloc_size );
				}
				else if( nesting_level > 1 )
				{
					for ( auto it = 1; it < nesting_level; it++ )
					{
						block
							->sub( fpr, insn.addr_size )
							->push( fpr );
					}
				}
				else
					block->push( frame_tmp );
			}
		},
		{
			X86_INS_LEAVE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto op_size = insn.addr_size * 8;
				x86_reg spr, fpr;

				switch ( op_size )
				{
					case 16:
						spr = X86_REG_SP;
						fpr = X86_REG_BP;
						break;
					case 32:
						spr = X86_REG_ESP;
						fpr = X86_REG_EBP;
						break;
					case 64:
						spr = X86_REG_RSP;
						fpr = X86_REG_RBP;
						break;
				}

				block
					->mov( spr, fpr )
					->pop( fpr );
			}
		},
		{
			X86_INS_LEA,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				store_operand( block, insn, 0, get_disp_from_operand( block, insn.operands[ 1 ] ) );
			}
		},
		{
			X86_INS_MOV,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
			}
		},
		{
			X86_INS_MOVABS,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
			}
		},
		{
			X86_INS_MOVZX,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
			}
		},
		{
			X86_INS_MOVSX,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto result = block->tmp( insn.operands[ 0 ].size * 8 );
				block->movsx( result, load_operand( block, insn, 1 ) );
				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_MOVSXD,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto result = block->tmp( insn.operands[ 0 ].size * 8 );
				block->movsx( result, load_operand( block, insn, 1 ) );
				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_PUSH,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto oper = load_operand( block, insn, 0 );
				if ( oper.bit_count() == 16 )
					block->sub( X86_REG_RSP, 2 );
				else
					block->sub( X86_REG_RSP, 8 );
				block->str( X86_REG_RSP, 0, oper );
			}
		},
		{
			X86_INS_POP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto tmp = block->tmp( 64 );
				block->ldd( tmp, X86_REG_RSP, block->sp_offset );
				store_operand( block, insn, 0, tmp );
				if ( insn.operands[ 0 ].size == 2 )
					block->add( X86_REG_RSP, 2 );
				else
					block->add( X86_REG_RSP, 8 );
			}
		},
		{
			X86_INS_XCHG,
			[]( basic_block* block, const instruction_info& insn )
			{
				auto op0 = load_operand( block, insn, 0 );
				auto op1 = load_operand( block, insn, 1 );

				auto [t0, t1] = block->tmp( op0.bit_count(), op1.bit_count() );

				block
					->mov( t0, op0 )
					->mov( t1, op1 );

				store_operand( block, insn, 0, t1 );
				store_operand( block, insn, 1, t0 );
			}
		},
		{
			X86_INS_PUSHFQ,
			[]( basic_block* block, const instruction_info& insn )
			{
				block->pushf();
			}
		},
		{
			X86_INS_POPFQ,
			[]( basic_block* block, const instruction_info& insn )
			{
				block->popf();
			}
		}
	};
}