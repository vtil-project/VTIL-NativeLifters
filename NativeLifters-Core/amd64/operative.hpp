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
			: op( value, sizeof( T ) * 8 )
		{ }

		operative( operand op )
			: op( std::move( op ) )
		{ }

		operative( const operative& lhs, math::operator_id opr, const operative& rhs )
		{
			auto elhs = lhs.op.is_register( )
				? symbolic::make_register_ex( lhs.op.reg( ), true )
				: symbolic::expression { lhs.op.imm( ).u64, lhs.op.bit_count( ) };
			auto erhs = rhs.op.is_register( )
				? symbolic::make_register_ex( rhs.op.reg( ), true )
				: symbolic::expression { rhs.op.imm( ).u64, rhs.op.bit_count( ) };

			op = *translator << symbolic::expression { elhs, opr, erhs };
		}

		operative( math::operator_id opr, const operative& rhs )
		{
			auto erhs = rhs.op.is_register( )
				? symbolic::make_register_ex( rhs.op.reg( ), true )
				: symbolic::expression { rhs.op.imm( ).u64, rhs.op.bit_count( ) };

			op = *translator << symbolic::expression { opr, erhs };
		}

		operative& operator=( const operative& o )
		{
			translator->block->push_back( { &ins::mov, { op, o.op } } );
			return *this;
		}

		operative operator&& ( const operative& o )
		{
			return ( *this != 0 ) & ( o != 0 );
		}

		operative operator|| ( const operative& o )
		{
			return ( *this != 0 ) | ( o != 0 );
		}

		operative popcnt( ) const
		{
			auto tmp = translator->block->tmp( op.bit_count( ) );
			translator->block
				->mov( tmp, op )
				->popcnt( tmp );
			return { operand{ tmp } };
		}
	};
}