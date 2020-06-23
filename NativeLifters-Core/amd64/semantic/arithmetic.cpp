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

// Various x86 arithmetic instructions.
// 
namespace vtil::lifter::amd64
{
	// Process the flags for a specific arithmetic instruction.
	//
	template<flags::flag_operation op>
	void process_flags( basic_block* block, const operand& lhs, const operand& rhs, const operand& result )
	{
		block
			->mov( flags::OF, flags::overflow<op>::flag( lhs, rhs, result ) )
			->mov( flags::CF, flags::carry<op>::flag( lhs, rhs, result ) )
			->mov( flags::SF, flags::sign( result ) )
			->mov( flags::ZF, flags::zero( result ) )
			->mov( flags::AF, flags::aux_carry( lhs, rhs, result ) )
			->mov( flags::PF, flags::parity( result ) );
	}

	// List of handlers.
	//
	static handler_map_t subhandlers = {
		{
			X86_INS_ADC,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto tmp = block->tmp( lhs.bit_count() );

				block
					->mov( tmp, lhs )
					->add( tmp, rhs )
					->add( tmp, flags::CF );

				process_flags<flags::flag_operation::add>( block, lhs, rhs, tmp );

				store_operand( block, insn, 0, { tmp } );
			}
		},
		{
			X86_INS_AAA,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto af = operative( flags::AF );
				auto al = X86_REG_AL;
				auto ax = X86_REG_AX;

				//
				// https://www.youtube.com/watch?v=fkFrAW217ZU
				//
				auto result = ( ( ( operative( al ) & 0xF ) > 9 ) | ( af == 1 ) );

				block
					->add( X86_REG_AX, __if( result, 0x106 ) )
					->mov( flags::AF, result )
					->mov( flags::CF, result )
					->mov( X86_REG_AL, ( operative( X86_REG_AL ) & 0xF ) );
			}
		},
		{
			X86_INS_DAA,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto old_al = operative( X86_REG_AL );
				auto old_cf = operative( flags::CF );
				auto af = operative( flags::AF );
				auto al = X86_REG_AL;

				auto result = ( ( ( operative( al ) & 0xF ) > 9 ) | ( af == 1 ) );

				block
					->mov( flags::CF, 0 )
					->add( X86_REG_AL, __if( result, 6 ) )
					->mov( flags::CF, __if( result, old_cf | operative( flags::CF ) ) )
					->mov( flags::AF, __if( result, 1 ) );

				result = ( ( old_al > 0x99 ) | ( old_cf == 1 ) );

				block
					->add( X86_REG_AL, __if( result, 0x60 ) )
					->mov( flags::CF, __if( result, 1 ) );
			}
		},
		{
			X86_INS_SBB,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto tmp = block->tmp( lhs.bit_count() );

				block
					->mov( tmp, lhs )
					->sub( tmp, rhs )
					->sub( tmp, flags::CF );

				process_flags<flags::flag_operation::sub>( block, lhs, rhs, tmp );

				store_operand( block, insn, 0, { tmp } );
			}
		},
		{
			X86_INS_MUL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto rhs = load_operand( block, insn, 0 );
				switch ( rhs.size() )
				{
					case 1:
					{
						auto tmp = block->tmp( 16 );
						block
							->mov( tmp, X86_REG_AL )
							->mul( tmp, rhs )
							->mov( X86_REG_AX, tmp )
							->tne( flags::CF, X86_REG_AH, 0 )
							->tne( flags::OF, X86_REG_AH, 0 );
						break;
					}

					case 2:
					{
						auto [lo, hi] = block->tmp( 16, 16 );
						block
							->mov( lo, X86_REG_AX )
							->mov( hi, X86_REG_AX )
							->mul( lo, rhs )
							->mulhi( hi, rhs )
							->mov( X86_REG_AX, lo )
							->mov( X86_REG_DX, hi )
							->tne( flags::CF, hi, 0 )
							->tne( flags::OF, hi, 0 );
						break;
					}

					case 4:
					{
						auto [lo, hi] = block->tmp( 32, 32 );
						block
							->mov( lo, X86_REG_EAX )
							->mov( hi, X86_REG_EAX )
							->mul( lo, rhs )
							->mulhi( hi, rhs )
							->mov( X86_REG_EAX, lo )
							->mov( X86_REG_EDX, hi )
							->tne( flags::CF, hi, 0 )
							->tne( flags::OF, hi, 0 );
						break;
					}

					case 8:
					{
						auto [lo, hi] = block->tmp( 64, 64 );
						block
							->mov( lo, X86_REG_RAX )
							->mov( hi, X86_REG_RAX )
							->mul( lo, rhs )
							->mulhi( hi, rhs )
							->mov( X86_REG_RAX, lo )
							->mov( X86_REG_RDX, hi )
							->tne( flags::CF, hi, 0 )
							->tne( flags::OF, hi, 0 );
						break;
					}
				}
			}
		},
		{
			X86_INS_IMUL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				switch ( insn.operands.size() )
				{
					case 1:
					{
						auto rhs = load_operand( block, insn, 0 );

						switch ( rhs.size() )
						{
							case 1:
							{
								auto tmp = block->tmp( 16 );
								block
									->mov( tmp, X86_REG_AL )
									->imul( tmp, rhs )
									->mov( X86_REG_AX, tmp )
									->tne( flags::CF, X86_REG_AH, 0 )
									->tne( flags::OF, X86_REG_AH, 0 );
								break;
							}

							case 2:
							{
								auto [lo, hi] = block->tmp( 16, 16 );
								block
									->mov( lo, X86_REG_AX )
									->mov( hi, X86_REG_AX )
									->imul( lo, rhs )
									->imulhi( hi, rhs )
									->mov( X86_REG_AX, lo )
									->mov( X86_REG_DX, hi )
									->tne( flags::CF, hi, 0 )
									->tne( flags::OF, hi, 0 );
								break;
							}

							case 4:
							{
								auto [lo, hi] = block->tmp( 32, 32 );
								block
									->mov( lo, X86_REG_EAX )
									->mov( hi, X86_REG_EAX )
									->imul( lo, rhs )
									->imulhi( hi, rhs )
									->mov( X86_REG_EAX, lo )
									->mov( X86_REG_EDX, hi )
									->tne( flags::CF, hi, 0 )
									->tne( flags::OF, hi, 0 );
								break;
							}

							case 8:
							{
								auto [lo, hi] = block->tmp( 64, 64 );
								block
									->mov( lo, X86_REG_RAX )
									->mov( hi, X86_REG_RAX )
									->imul( lo, rhs )
									->imulhi( hi, rhs )
									->mov( X86_REG_RAX, lo )
									->mov( X86_REG_RDX, hi )
									->tne( flags::CF, hi, 0 )
									->tne( flags::OF, hi, 0 );
								break;
							}
						}

						break;
					}

					case 2:
					{
						auto lhs = load_operand( block, insn, 0 );
						auto rhs = load_operand( block, insn, 1 );

						auto [lo, hi] = block->tmp( lhs.bit_count(), lhs.bit_count() );
						block
							->mov( lo, lhs )
							->mov( hi, lhs )
							->mul( lo, rhs )
							->mulhi( hi, rhs )
							->mov( flags::CF, ( flags::sign( { hi } ) != flags::sign( { lo } ) ))
							->mov( flags::OF, flags::CF );

						store_operand( block, insn, 0, lo );
						break;
					}

					case 3:
					{
						auto lhs = load_operand( block, insn, 1 );
						auto rhs = load_operand( block, insn, 2 );

						auto [lo, hi] = block->tmp( lhs.bit_count(), lhs.bit_count() );
						block
							->mov( lo, lhs )
							->mov( hi, lhs )
							->mul( lo, rhs )
							->mulhi( hi, rhs )
							->mov( flags::CF, ( flags::sign( { hi } ) != flags::sign( { lo } ) ))
							->mov( flags::OF, flags::CF );

						store_operand( block, insn, 0, lo );
						break;
					}
				}
			}
		},
		// NOTE:
		// In a shift, AF is either set to undefined or the same value.
		// This is annoying to implement and even more annoying to simplify, so
		// our "undefined behavior" is that the AF flag does not change during this 
		// operation. :^)
		// TODO: THE FLAGS SET HERE ARE WRONG; WE NEED TO CHECK FOR ZERO BEFORE MODIFYING THEM!
		{
			X86_INS_SHL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto result = ( operative( lhs ) << ( operative( rhs ) & ( lhs.size() == 8 ? 0x3F : 0x1F ) ) ).op;
				auto lhs_sign = flags::sign( { lhs } );

				block
					->mov( flags::CF, lhs_sign)
					->mov( flags::OF, ( flags::sign( { result } ) ^ lhs_sign ))
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_SHR,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto result = ( operative( lhs ) >> ( operative( rhs ) & ( lhs.size() == 8 ? 0x3F : 0x1F ) ) ).op;

				block
					->mov( flags::CF, ( operative( lhs ) & 1 ))
					->mov( flags::OF, flags::sign( { lhs } ))
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_SAR,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) );

				auto result = ( ( lhs >> ( rhs & ( lhs.op.size() == 8 ? 0x3F : 0x1F ) ) ) | ( lhs & ( 1ULL << ( lhs.op.bit_count() - 1 ) ) ) ).op;

				block
					->mov( flags::CF, ( lhs & 1 ))
					->mov( flags::OF, 0 )
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result );
			}
		},
		// --
		{
			X86_INS_ROL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto result = block->tmp( lhs.bit_count() );
				block
					->mov( result, lhs )
					->brol( result, rhs )
					->mov( flags::CF, ( operative( result ) & 1 ));

				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_ROR,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto result = block->tmp( lhs.bit_count() );
				block
					->mov( result, lhs )
					->bror( result, rhs )
					->mov( flags::CF, flags::sign( { result } ));

				store_operand( block, insn, 0, result );
			}
		},
		{
			X86_INS_RCL,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				// TODO: Fix me:
				unreachable();
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) ) & ( lhs.op.size() == 8 ? 0x3F : 0x1F );
				auto cf = operative( flags::CF );

				auto result = ( lhs << rhs ) | ( lhs >> ( rhs + 1 ) ) | ( cf.zext( lhs.op.bit_count() ) << ( rhs - 1 ) );
				auto carry_result = ( lhs & ( lhs.op.bit_count() << rhs ) );

				block
					->mov( flags::CF, carry_result)
					->mov( flags::OF, ( cf ^ flags::sign( result ) ));

				store_operand( block, insn, 0, result);
			}
		},
		{
			X86_INS_RCR,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				// TODO: Fix me:
				unreachable();
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto rhs = operative( load_operand( block, insn, 1 ) ) & ( lhs.op.size() == 8 ? 0x3F : 0x1F );
				auto cf = operative( flags::CF );

				auto result = ( lhs >> rhs ) | ( lhs << ( rhs - 1 ) ) | ( cf.zext( lhs.op.bit_count() ) << ( lhs.op.bit_count() - rhs ) );
				auto carry_result = ( lhs & ( 1ULL << ( rhs - 1 ) ) );

				block
					->mov( flags::CF, carry_result)
					->mov( flags::OF, ( cf ^ flags::sign( lhs ) ));

				store_operand( block, insn, 0, result);
			}
		},
		{
			X86_INS_INC,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto result = lhs + 1;

				block
					->mov( flags::AF, flags::aux_carry( lhs, { 1 }, result ))
					->mov( flags::OF, flags::overflow<flags::add>::flag( lhs, { 1 }, result ))
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result);
			}
		},
		{
			X86_INS_DEC,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto result = lhs - 1;

				block
					->mov( flags::AF, flags::aux_carry( lhs, { -1 }, result ))
					->mov( flags::OF, flags::overflow<flags::sub>::flag( lhs, { -1 }, result ))
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result);
			}
		},
		{
			X86_INS_NEG,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto result = 0 - lhs;

				block
					->mov( flags::CF, ( lhs != 0 ))
					->mov( flags::AF, flags::aux_carry( { 0 }, lhs, result ))
					->mov( flags::OF, flags::overflow<flags::sub>::flag( { 0 }, lhs, result ))
					->mov( flags::SF, flags::sign( result ))
					->mov( flags::ZF, flags::zero( result ))
					->mov( flags::PF, flags::parity( result ));

				store_operand( block, insn, 0, result);
			}
		},
		{
			X86_INS_NOT,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				store_operand( block, insn, 0, ( ~lhs ));
			}
		},
		{
			X86_INS_BSWAP,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = operative( load_operand( block, insn, 0 ) );
				auto tmp = block->tmp( lhs.op.bit_count() );

				block->mov( tmp, 0 );

				switch ( lhs.op.size() )
				{
					case 2:
						// this is UB, but seems to store 0.
						break;

					case 4:
						block
							->bor( tmp, ( ( lhs & 0xFF ) << 24 ))
							->bor( tmp, ( ( lhs & 0xFF00 ) << 8 ))
							->bor( tmp, ( ( lhs & 0xFF0000 ) >> 8 ))
							->bor( tmp, ( ( lhs & 0xFF000000 ) >> 24 ));
						break;

					case 8:
						block
							->bor( tmp, ( ( lhs & 0xFFULL ) << 56 ))
							->bor( tmp, ( ( lhs & 0xFF00ULL ) << 40 ))
							->bor( tmp, ( ( lhs & 0xFF0000ULL ) << 24 ))
							->bor( tmp, ( ( lhs & 0xFF000000ULL ) << 8 ))
							->bor( tmp, ( ( lhs & 0xFF00000000ULL ) >> 8 ))
							->bor( tmp, ( ( lhs & 0xFF0000000000ULL ) >> 24 ))
							->bor( tmp, ( ( lhs & 0xFF000000000000ULL ) >> 40 ))
							->bor( tmp, ( ( lhs & 0xFF00000000000000ULL ) >> 56 ));
						break;

					default:
						unreachable();
						break;
				}

				store_operand( block, insn, 0, tmp );
			}
		},
		{
			X86_INS_BSF,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto tmp = block->tmp( lhs.bit_count() );
				block
					->mov( tmp, rhs )
					->bsf( tmp )
					->te( flags::ZF, tmp, 0 )
					->sub( tmp, 1 ); // Value is UD if no bits set so -1 is acceptable.

				store_operand( block, insn, 0, tmp );
			}
		},
		{
			X86_INS_BSR,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto tmp = block->tmp( lhs.bit_count() );
				block
					->mov( tmp, rhs )
					->bsr( tmp )
					->te( flags::ZF, tmp, 0 )
					->sub( tmp, 1 ); // Value is UD if no bits set so -1 is acceptable.

				store_operand( block, insn, 0, tmp );
			}
		},
		{
			X86_INS_XADD,
			[ ] ( basic_block* block, const instruction_info& insn )
			{
				auto lhs = load_operand( block, insn, 0 );
				auto rhs = load_operand( block, insn, 1 );

				auto tmp = block->tmp( lhs.bit_count() );
				block
					->mov( tmp, lhs )
					->add( tmp, rhs );

				process_flags<flags::flag_operation::add>( block, lhs, rhs, tmp );

				store_operand( block, insn, 0, tmp );
				store_operand( block, insn, 1, rhs );
			}
		},
		
		// CX generics
		//
		#define DEFINE_CX(mnemonic, in, out)						   \
		{															   \
			mnemonic,											       \
			[ ] ( basic_block* block, const instruction_info& insn )   \
			{														   \
				block->mov( out, flags::sign( { in } ));		       \
				block->neg( out );									   \
			}														   \
		}
		#define DEFINE_CXE(mnemonic, in, out)						   \
		{															   \
			mnemonic,											       \
			[ ] ( basic_block* block, const instruction_info& insn )   \
			{														   \
				block->movsx( out, in );							   \
			}														   \
		}
		DEFINE_CX(  X86_INS_CWD,  X86_REG_AX,  X86_REG_DX  ),
		DEFINE_CX(  X86_INS_CDQ,  X86_REG_EAX, X86_REG_EDX ),
		DEFINE_CX(  X86_INS_CQO,  X86_REG_RAX, X86_REG_RDX ),
		DEFINE_CXE( X86_INS_CBW,  X86_REG_AL,  X86_REG_AX  ),
		DEFINE_CXE( X86_INS_CWDE, X86_REG_AX,  X86_REG_EAX ),
		DEFINE_CXE( X86_INS_CDQE, X86_REG_EAX, X86_REG_RAX ),
		
		// Binop generics
		//
		#define DEFINE_BINOP(mnemonic, name, op)						            \
		{															                \
			mnemonic,											                    \
			[ ] ( basic_block* block, const instruction_info& insn )                \
			{														                \
				auto lhs = load_operand( block, insn, 0 );                          \
				auto rhs = load_operand( block, insn, 1 );                          \
				auto tmp = block->tmp( lhs.bit_count() );                           \
				block->mov( tmp, lhs )->op( tmp, rhs );                             \
				process_flags<flags::flag_operation::op>( block, lhs, rhs, tmp ); \
				store_operand( block, insn, 0, tmp );                               \
			}														                \
		}
		DEFINE_BINOP( X86_INS_ADD, add, add  ),
		DEFINE_BINOP( X86_INS_SUB, sub, sub  ),
		DEFINE_BINOP( X86_INS_AND, and, band ),
		DEFINE_BINOP( X86_INS_XOR, xor, bxor ),
		DEFINE_BINOP( X86_INS_OR,  or,  bor  ),
	};

	static bool __init = register_subhandlers( std::move( subhandlers ) );
}