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

// Branching instructions.
// 
namespace vtil::lifter::amd64
{
	// List of handlers.
	//
	static handler_map_t subhandlers = {
		{
			X86_INS_JMP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block->jmp( load_operand( block, insn, 0 ) );
			}
		},
		{
			X86_INS_JAE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative cf( flags::CF );

				block
					->js( ( cf == 0 ), 
						  load_operand( block, insn, 0 ), 
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JA,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative cf( flags::CF );
				operative zf( flags::ZF );

				block
					->js( ( cf == 0 ) & ( zf == 0 ), 
						  load_operand( block, insn, 0 ), 
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JBE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative cf( flags::CF );
				operative zf( flags::ZF );

				block
					->js( ( cf == 1 ) & ( zf == 1 ), 
						  load_operand( block, insn, 0 ), 
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JB,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative cf( flags::CF );

				block
					->js( ( cf == 1 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JCXZ,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block
					->js( ( operative( X86_REG_CX ) == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JECXZ,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block
					->js( ( operative( X86_REG_ECX ) == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative zf( flags::ZF );

				block
					->js( ( zf == 1 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JGE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );
				operative of( flags::OF );

				block
					->js( ( sf == of ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JG,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );
				operative of( flags::OF );
				operative zf( flags::ZF );

				block
					->js( ( zf == 0 ) & ( sf == of ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JLE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );
				operative of( flags::OF );
				operative zf( flags::ZF );

				block
					->js( ( zf == 1 ) & ( sf != of ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );
				operative of( flags::OF );

				block
					->js( ( sf != of ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JNE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative zf( flags::ZF );

				block
					->js( ( zf == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JNO,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative of( flags::OF );

				block
					->js( ( of == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JNP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative pf( flags::PF );

				block
					->js( ( pf == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JNS,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );

				block
					->js( ( sf == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JO,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative of( flags::OF );

				block
					->js( ( of == 1 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative pf( flags::PF );

				block
					->js( ( pf == 1 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JRCXZ,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				block
					->js( ( operative( X86_REG_CX ) == 0 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_JS,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				operative sf( flags::SF );

				block
					->js( ( sf == 1 ),
						  load_operand( block, insn, 0 ),
						  insn.address + insn.bytes.size() );
			}
		},
		{
			X86_INS_CALL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto vip_after_insn = insn.address + insn.bytes.size();
				block->sub( X86_REG_RSP, 8 );
				block->str( X86_REG_RSP, 0, vip_after_insn );
				block->jmp( load_operand( block, insn, 0 ) );
			}
		},
		{
			X86_INS_RET,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto to_pop_after_ret = insn.operands.empty() ? 0ULL : insn.operands[ 0 ].imm;
				auto retaddr = block->tmp( 64 );
				block->ldd( retaddr, X86_REG_RSP, 0 );
				block->add( X86_REG_RSP, to_pop_after_ret + 8 );
				block->jmp( retaddr );
			}
		}
	};

	static bool __init = register_subhandlers( std::move( subhandlers ) );
}