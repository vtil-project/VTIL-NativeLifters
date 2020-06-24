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
#include <vtil/arch>

namespace vtil::lifter
{
	// Provides a simple wrapper over operands to support instruction emittion via operators
	//
	struct operative : math::operable<operative>
	{
		operand op;
		inline static thread_local batch_translator* translator = nullptr;

		template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
		operative( T value )
			: op( value, sizeof( T ) * 8 ) {}

		operative( operand op )
			: op( std::move( op ) ) {}

		operative( const operative& lhs, math::operator_id opr, const operative& rhs )
		{
			auto elhs = lhs.op.is_register()
				? symbolic::make_register_ex( lhs.op.reg(), true )
				: symbolic::expression{ lhs.op.imm().u64, lhs.op.bit_count() };
			auto erhs = rhs.op.is_register()
				? symbolic::make_register_ex( rhs.op.reg(), true )
				: symbolic::expression{ rhs.op.imm().u64, rhs.op.bit_count() };

			op = *translator << symbolic::variable::pack_all( symbolic::expression{ elhs, opr, erhs } );
		}

		operative( math::operator_id opr, const operative& rhs )
		{
			auto erhs = rhs.op.is_register()
				? symbolic::make_register_ex( rhs.op.reg(), true )
				: symbolic::expression{ rhs.op.imm().u64, rhs.op.bit_count() };

			op = *translator << symbolic::variable::pack_all( symbolic::expression{ opr, erhs } );
		}

		bitcnt_t bit_count()
		{
			return op.bit_count();
		}

		operative& operator=( const operative& o )
		{
			translator->block->push_back( { &ins::mov, { op, o.op } } );
			return *this;
		}

		operative operator!= ( const operative& o )
		{
			return ( *this == o ) == 0;
		}

		operative operator&& ( const operative& o )
		{
			return ( *this != 0 ) & ( o != 0 );
		}

		operative operator|| ( const operative& o )
		{
			return ( *this != 0 ) | ( o != 0 );
		}

		operative popcnt() const
		{
			auto tmp = translator->block->tmp( 32 );
			translator->block
				->mov( tmp, op )
				->popcnt( tmp );
			return operand{ tmp };
		}

		operative zext( bitcnt_t bit_size ) const
		{
			auto tmp = translator->block->tmp( bit_size );
			translator->block
				->mov( tmp, op );
			return { operand{ tmp } };
		}

		operative sext( bitcnt_t bit_size ) const
		{
			auto tmp = translator->block->tmp( bit_size );
			translator->block
				->movsx( tmp, op );
			return { operand{ tmp } };
		}
	};
};

// Abuse register cast to make operative castable into an operand.
//
namespace vtil
{
	template<>
	struct register_cast<lifter::operative>
	{
		auto&& operator()( lifter::operative&& opr )     { return opr.op.descriptor; }
		auto& operator()( const lifter::operative& opr ) { return opr.op.descriptor; }
	};
};