#pragma once
#include <cstdint>
#include <vtil/utility>
#include <vtil/arch>
#include "operative.hpp"

namespace vtil::lifter
{
	namespace amd64::flags
	{
		// Individual flag registers
		//
		inline static const vtil::register_desc CF = { vtil::register_physical | vtil::register_flags, 0, 1, 0 };
		inline static const vtil::register_desc PF = { vtil::register_physical | vtil::register_flags, 0, 1, 2 };
		inline static const vtil::register_desc AF = { vtil::register_physical | vtil::register_flags, 0, 1, 4 };
		inline static const vtil::register_desc ZF = { vtil::register_physical | vtil::register_flags, 0, 1, 6 };
		inline static const vtil::register_desc SF = { vtil::register_physical | vtil::register_flags, 0, 1, 7 };
		inline static const vtil::register_desc DF = { vtil::register_physical | vtil::register_flags, 0, 1, 10 };
		inline static const vtil::register_desc OF = { vtil::register_physical | vtil::register_flags, 0, 1, 11 };

		// Clears all tracked bits, deeming them undefined
		//
		static void clear( vtil::basic_block* block )
		{
			block
				->mov( CF, UNDEFINED )
				->mov( PF, UNDEFINED )
				->mov( AF, UNDEFINED )
				->mov( ZF, UNDEFINED )
				->mov( SF, UNDEFINED )
				->mov( DF, UNDEFINED )
				->mov( OF, UNDEFINED );
		}

		// Checks ZF
		//
		static operative zero( const operative& value )
		{
			return value == 0;
		}

		// Checks AF
		//
		static operative aux_carry( const operative& lhs, const operative& rhs, const operative& result )
		{
			return ( lhs ^ rhs ^ result ) & 0x10;
		}

		// Checks AF
		//
		static operative aux_carry( const operative& lhs, const operative& rhs, const operative& carry, const operative& result )
		{
			return ( lhs ^ rhs ^ carry ^ result ) & 0x10;
		}

		// Checks SF
		//
		static operative sign( const operative& value )
		{
			return ( value < 0 );
		}

		// Checks PF
		//
		static operative parity( const operative& value )
		{
			return ( value.popcnt( ) & 1 ) == 0;
		}

		// Specifies the type of operation flags should be determined for
		//
		enum flag_operation : uint32_t
		{
			add,
			sub,
			mul,
			imul,
			div,
			idiv,
			band,
			bor,
			bxor,
			bshl,
			bshr
		};

		// Overflow bit varies per operation, so it cannot have a generic computation
		//
		template <flag_operation Op>
		struct overflow;

		// Signed / Unsigned Add
		//
		template<>
		struct overflow<add>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				// Compute the sign bits from each value
				//
				auto lhs_sign = sign( lhs );
				auto rhs_sign = sign( rhs );
				auto res_sign = sign( result );

				return ( lhs_sign ^ res_sign ) & ( rhs_sign ^ res_sign );
			}
		};

		// Signed / Unsigned Subtract
		//
		template<>
		struct overflow<sub>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				// Compute the sign bits from each value
				//
				auto lhs_sign = sign( lhs );
				auto rhs_sign = sign( rhs );
				auto res_sign = sign( result );

				return ( lhs_sign ^ rhs_sign ) & ( lhs_sign ^ res_sign );
			}
		};

		// Unsigned Multiplication
		//
		template<>
		struct overflow<mul>
		{
			template <typename T>
			static T flag( T first, T second, T result_upper, T result_lower )
			{
				return result_upper != 0;
			}
		};

		// Binary bitwise and
		//
		template<>
		struct overflow<band>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};

		// Binary bitwise or
		//
		template<>
		struct overflow<bor>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};


		// Binary bitwise xor
		//
		template<>
		struct overflow<bxor>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};

		// Signed Multiplication
		//
		template<>
		struct overflow<imul>
		{
			// TODO: actually implement this

			template <typename T>
			static T flag( T first, T second, T result_upper, T result_lower )
			{
				return first;
			}

			template <typename T>
			static T flag( T first, T second, T result )
			{
				return first;
			}
		};

		// Carry bit varies per operation, so it cannot have a generic computation
		//
		template <flag_operation Op>
		struct carry;

		// Signed / Unsigned Add
		//
		template<>
		struct carry<add>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return result < lhs || result < rhs;
			}
		};

		// Signed / Unsigned Subtract
		//
		template<>
		struct carry<sub>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return lhs < rhs;
			}
		};

		// Binary bitwise and
		//
		template<>
		struct carry<band>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};

		// Binary bitwise or
		//
		template<>
		struct carry<bor>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};


		// Binary bitwise xor
		//
		template<>
		struct carry<bxor>
		{
			static operative flag( const operative& lhs, const operative& rhs, const operative& result )
			{
				return { 0 };
			}
		};

		// Unsigned Multiplication
		//
		template<>
		struct carry<mul>
		{
			template <typename T>
			static T flag( T first, T second, T result_upper, T result_lower )
			{

			}
		};

		// Signed Multiplication
		//
		template<>
		struct carry<imul>
		{
			template <typename T>
			static T flag( T first, T second, T result_upper, T result_lower )
			{

			}

			template <typename T>
			static T flag( T first, T second, T result )
			{

			}
		};
	}
}
