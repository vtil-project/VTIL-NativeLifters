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
#include <cstdint>
#include <vtil/utility>
#include <vtil/arch>
#include "../core/operative.hpp"

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
		inline static const vtil::register_desc IF = { vtil::register_physical | vtil::register_flags, 0, 1, 9 };
		inline static const vtil::register_desc DF = { vtil::register_physical | vtil::register_flags, 0, 1, 10 };
		inline static const vtil::register_desc OF = { vtil::register_physical | vtil::register_flags, 0, 1, 11 };

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
			return ( ( value & 0xFF ).popcnt() & 1 ) == 0;
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
	}
}