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
#pragma once
#include <vtil/amd64>
#include <vtil/arch>
#include <functional>
#include <unordered_map>

// This file defines any global arch-specific information for the AMD64 target
// architecture.
//
namespace vtil::lifter::amd64
{
	using operand_info = cs_x86_op;
	using instruction_info = vtil::amd64::instruction;
	using handler_map_t = std::unordered_map<x86_insn, std::function<void( basic_block*, const instruction_info& )>>;

	struct lifter_t
	{
		// Disassemble and process an instruction.
		// Returns the length of the instruction processed.
		//
		static size_t process( basic_block* block, uint64_t vip, uint8_t* code );
	};

	inline handler_map_t instruction_handlers = {};

	operand load_operand( basic_block* block, const instruction_info& insn, size_t idx );
	register_desc get_disp_from_operand( basic_block* block, const operand_info& operand );
	void store_operand( basic_block* block, const instruction_info& insn, size_t idx, const operand& source );

	static bool register_subhandlers( handler_map_t&& sub_handlers )
	{
		for ( auto&& [k, v] : sub_handlers )
		{
			fassert( !instruction_handlers.contains( k ) );
			instruction_handlers.emplace( k, std::move( v ) );
		}
		return true;
	}
};