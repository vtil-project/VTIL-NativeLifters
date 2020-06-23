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
	static handler_map_t subhandlers = {
		{
			X86_INS_CMP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( block->tmp( lhs.op.bit_count() ) );

				block
					->movsx( rhs.op, load_operand( block, insn, 1 ) );

				auto result = lhs - rhs;

				block
					->mov( flags::CF, flags::carry<flags::sub>::flag( lhs, rhs, result ).op )
					->mov( flags::OF, flags::overflow<flags::sub>::flag( lhs, rhs, result ).op )
					->mov( flags::SF, flags::sign( result ).op )
					->mov( flags::ZF, flags::zero( result ).op )
					->mov( flags::AF, flags::aux_carry( lhs, rhs, result ).op )
					->mov( flags::PF, flags::parity( result ).op );
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
					->mov( flags::SF, flags::sign( result ).op )
					->mov( flags::ZF, flags::zero( result ).op )
					->mov( flags::PF, flags::parity( result ).op );
			}
		},
	};

	static bool __init = register_subhandlers( std::move( subhandlers ) );
}