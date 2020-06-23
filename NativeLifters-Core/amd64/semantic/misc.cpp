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
	static handler_map_t subhandlers = {
		{
			X86_INS_INVALID,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block->vemits( "int 0xB" );
				block->vexit( invalid_vip );
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
				store_operand( block, insn, 0, { X86_REG_RSP } );
				if ( insn.operands[ 0 ].size == 2 )
					block->add( X86_REG_RSP, 2 );
				else
					block->add( X86_REG_RSP, 8 );
			}
		},
	};

	static bool __init = register_subhandlers( std::move( subhandlers ) );	
}