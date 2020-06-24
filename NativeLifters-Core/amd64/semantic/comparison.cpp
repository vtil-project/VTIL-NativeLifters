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

// Various x86 comparison instructions.
// 
namespace vtil::lifter::amd64
{
	// List of handlers.
	//
	handler_map_t comparison_handlers = {
		{
			X86_INS_CMP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( block->tmp( lhs.op.bit_count() ) );

				block->movsx( rhs, load_operand( block, insn, 1 ) );

				auto result = lhs - rhs;

				block
					->mov( flags::CF, flags::carry<flags::sub>::flag( lhs, rhs, result ) )
					->mov( flags::OF, flags::overflow<flags::sub>::flag( lhs, rhs, result ) )
					->mov( flags::SF, flags::sign( result ) )
					->mov( flags::ZF, flags::zero( result ) )
					->mov( flags::AF, flags::aux_carry( lhs, rhs, result ) )
					->mov( flags::PF, flags::parity( result ) );
			}
		},
		{
			X86_INS_TEST,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				auto result = lhs & rhs;

				block
					->mov( flags::CF, 0 )
					->mov( flags::OF, 0 )
					->mov( flags::SF, flags::sign( result ) )
					->mov( flags::ZF, flags::zero( result ) )
					->mov( flags::PF, flags::parity( result ) );
			}
		},
		{
			X86_INS_CMPXCHG,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto accumulator = operative( X86_REG_RAX );
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );
				auto temp = lhs;

				block
					->mov( flags::ZF, 
						   __if( accumulator == temp, 1 ) )
					->mov( X86_REG_RAX, 
						   __if( accumulator == temp, lhs ) | 
						   __if( ~( accumulator == temp ), temp ) );

				store_operand( block, insn, 0, 
							   __if( accumulator == temp, rhs ) | 
							   __if( ~(accumulator == temp), temp ) );
			}
		},
		{
			X86_INS_CMOVA,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative cf( flags::CF );
				operative zf( flags::ZF );

				auto result = ( zf == 0 ) & ( cf == 0 );
				
				store_operand( block, insn, 0, 
							   __if( result, rhs ) | 
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVAE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative cf( flags::CF );

				store_operand( block, insn, 0,
							   __if( ( cf == 0 ), rhs ) |
							   __if( ( cf == 1 ), lhs ) );
			}
		},
		{
			X86_INS_CMOVB,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative cf( flags::CF );

				store_operand( block, insn, 0,
							   __if( ( cf == 1 ), rhs ) |
							   __if( ( cf == 0 ), lhs ) );
			}
		},
		{
			X86_INS_CMOVBE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative cf( flags::CF );
				operative zf( flags::ZF );

				auto result = ( ( zf == 1 ) | ( cf == 1 ) );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative zf( flags::ZF );

				store_operand( block, insn, 0,
							   __if( ( zf == 1 ), rhs ) |
							   __if( ( zf == 0 ), lhs ) );
			}
		},
		{
			X86_INS_CMOVG,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );
				operative zf( flags::ZF );
				operative of( flags::OF );

				auto result = ( ( zf == 0 ) & ( sf == of ) );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVGE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );
				operative of( flags::OF );

				auto result = ( sf == of );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );
				operative of( flags::OF );

				auto result = ( sf != of );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVLE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );
				operative zf( flags::ZF );
				operative of( flags::OF );

				auto result = ( ( zf == 1 ) | ( sf != of ) );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVNE,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative zf( flags::ZF );

				auto result = ( zf == 0 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVNO,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative of( flags::OF );

				auto result = ( of == 0 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVNP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative pf( flags::PF );

				auto result = ( pf == 0 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVNS,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );

				auto result = ( sf == 0 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVO,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative of( flags::OF );

				auto result = ( of == 1 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative pf( flags::PF );

				auto result = ( pf == 1 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
		{
			X86_INS_CMOVS,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				operative sf( flags::SF );

				auto result = ( sf == 1 );

				store_operand( block, insn, 0,
							   __if( result, rhs ) |
							   __if( ~result, lhs ) );
			}
		},
	};
}