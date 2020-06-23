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

// Flag manipulation instructions.
//
namespace vtil::lifter::amd64
{
	void process_clc( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::CF, 0 );
	}

	void process_cld( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::DF, 0 );
	}

	void process_cli( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::IF, 0 );
	}

	void process_stc( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::CF, 1 );
	}

	void process_std( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::DF, 1 );
	}

	void process_sti( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::IF, 1 );
	}

	void process_cmc( basic_block* block, const instruction_info& insn )
	{
		block
			->bnot( flags::CF );
	}

	void process_seta( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( ( cf == 0 ) & ( zf == 0 ) ) );
	}

	void process_setae( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );

		store_operand( block, insn, 0, ( cf == 0 ) );
	}


	void process_setb( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::CF } );
	}

	void process_setbe( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( cf | zf ) );
	}

	void process_sete( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::ZF } );
	}

	void process_setge( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );

		store_operand( block, insn, 0, ( sf & of ) );
	}

	void process_setg( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( ( zf == 0 ) & ( sf == of ) ) );
	}

	void process_setle( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( zf & ( sf != of ) ) );
	}

	void process_setl( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( sf != of ) );
	}

	void process_setne( basic_block* block, const instruction_info& insn )
	{
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( zf == 0 ) );
	}

	void process_setno( basic_block* block, const instruction_info& insn )
	{
		operative of( flags::OF );

		store_operand( block, insn, 0, ( of == 0 ) );
	}

	void process_setnp( basic_block* block, const instruction_info& insn )
	{
		operative pf( flags::PF );

		store_operand( block, insn, 0, ( pf == 0 ) );
	}

	void process_setns( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );

		store_operand( block, insn, 0, ( sf == 0 ) );
	}

	void process_seto( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::OF } );
	}

	void process_setp( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::PF } );
	}

	void process_sets( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::SF } );
	}

	operative simple_bt( basic_block* block, const operative& lhs, const operative& rhs )
	{
		uint64_t mask;
		switch ( lhs.op.size() )
		{
			case 2:
				mask = 0xF;
				break;

			case 4:
				mask = 0x1F;
				break;

			case 8:
				mask = 0x3F;
				break;

			default:
				unreachable();
				break;
		}

		auto result = rhs & mask;
		block->mov( flags::CF, ( ( lhs & ( 1ULL << result ) ) != 0 ) );
		return result;
	}

	void process_bt( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		simple_bt( block, lhs, rhs );
	}

	void process_btc( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs ^ ( 1ULL << result ) ) );
	}

	void process_btr( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs & ~( 1ULL << result ) ) );
	}

	void process_bts( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs | ( 1ULL << result ) ) );
	}

	static bool __init = register_subhandlers( {
		{X86_INS_CLD  , process_cld },
		{X86_INS_CLI  , process_cli },
		{X86_INS_STC  , process_stc },
		{X86_INS_STD  , process_std },
		{X86_INS_STI  , process_sti },
		{X86_INS_CMC  , process_cmc },
		{X86_INS_SETA , process_seta },
		{X86_INS_SETAE, process_setae },
		{X86_INS_SETB , process_setb },
		{X86_INS_SETBE, process_setbe },
		{X86_INS_SETE , process_sete },
		{X86_INS_SETGE, process_setge },
		{X86_INS_SETG , process_setg },
		{X86_INS_SETLE, process_setle },
		{X86_INS_SETL , process_setl },
		{X86_INS_SETNE, process_setne },
		{X86_INS_SETNO, process_setno },
		{X86_INS_SETNP, process_setnp },
		{X86_INS_SETNS, process_setns },
		{X86_INS_SETO , process_seto },
		{X86_INS_SETP , process_setp },
		{X86_INS_SETS , process_sets },
		{X86_INS_BT   , process_bt },
		{X86_INS_BTC  , process_btc },
		{X86_INS_BTR  , process_btr },
		{X86_INS_BTS  , process_bts }
	} );
}