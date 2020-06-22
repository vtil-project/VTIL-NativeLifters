// Various x86 arithmetic instructions.
// 

namespace vtil::lifter::amd64
{
	// Process the flags for a specific arithmetic instruction.
	//
	template<flags::flag_operation Op>
	void process_flags( basic_block* block, const operand& lhs, const operand& rhs, const operand& result )
	{
		block
			->mov( flags::OF, flags::overflow<Op>::flag( lhs, rhs, result).op )
			->mov( flags::CF, flags::carry<Op>::flag( lhs, rhs, result ).op )
			->mov( flags::SF, flags::sign( result ).op )
			->mov( flags::ZF, flags::zero( result ).op )
			->mov( flags::AF, flags::aux_carry( lhs, rhs, result ).op )
			->mov( flags::PF, flags::parity( result ).op );
	}

#define PROCESS_BINOP( name, op ) void process_##name( basic_block* block, const instruction_info& insn ) \
	{ \
		auto lhs = load_operand( block, insn, 0 ); \
		auto rhs = load_operand( block, insn, 1 ); \
		auto tmp = block->tmp( lhs.bit_count( ) ); \
		block->mov( tmp, lhs )->op( tmp, rhs ); \
		process_flags<flags::flag_operation::##op>( block, lhs, rhs, tmp ); \
		store_operand( block, insn, 0, tmp ); \
	}

	PROCESS_BINOP( add, add );
	PROCESS_BINOP( sub, sub );
	PROCESS_BINOP( and, band );
	PROCESS_BINOP( or, bor );
	PROCESS_BINOP( xor, bxor );

	void process_mul( basic_block* block, const instruction_info& insn )
	{
		auto rhs = load_operand( block, insn, 0 );

		block
			->mov( flags::PF, UNDEFINED )
			->mov( flags::AF, UNDEFINED )
			->mov( flags::ZF, UNDEFINED )
			->mov( flags::SF, UNDEFINED );

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

	void process_imul( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::PF, UNDEFINED )
			->mov( flags::AF, UNDEFINED )
			->mov( flags::ZF, UNDEFINED )
			->mov( flags::SF, UNDEFINED );

		switch ( insn.operands.size( ) )
		{
			case 1:
			{
				auto rhs = load_operand( block, insn, 0 );

				switch ( rhs.size( ) )
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

				auto [lo, hi] = block->tmp( lhs.bit_count( ), lhs.bit_count( ) );
				block
					->mov( lo, lhs )
					->mov( hi, lhs )
					->mul( lo, rhs )
					->mulhi( hi, rhs )
					->mov( flags::CF, ( flags::sign( { hi } ) != flags::sign( { lo } ) ).op )
					->mov( flags::OF, flags::CF );

				store_operand( block, insn, 0, lo );
				break;
			}

			case 3:
			{
				auto lhs = load_operand( block, insn, 1 );
				auto rhs = load_operand( block, insn, 2 );

				auto [lo, hi] = block->tmp( lhs.bit_count( ), lhs.bit_count( ) );
				block
					->mov( lo, lhs )
					->mov( hi, lhs )
					->mul( lo, rhs )
					->mulhi( hi, rhs )
					->mov( flags::CF, ( flags::sign( { hi } ) != flags::sign( { lo } ) ).op )
					->mov( flags::OF, flags::CF );
					
				store_operand( block, insn, 0, lo );
				break;
			}
		}
	}	

	// NOTE:
	// In a shift, AF is either set to undefined or the same value.
	// This is annoying to implement and even more annoying to simplify, so
	// our "undefined behavior" is that the AF flag does not change during this 
	// operation. :^)

	void process_shl( basic_block* block, const instruction_info& insn )
	{
		auto lhs = load_operand( block, insn, 0 );
		auto rhs = load_operand( block, insn, 1 );

		auto result = ( operative( lhs ) << ( operative( rhs ) & ( lhs.size( ) == 8 ? 0x3F : 0x1F ) ) ).op;
		auto lhs_sign = flags::sign( { lhs } );

		block
			->mov( flags::CF, lhs_sign.op )
			->mov( flags::OF, ( flags::sign( { result } ) ^ lhs_sign ).op )
			->mov( flags::SF, flags::sign( result ).op )
			->mov( flags::ZF, flags::zero( result ).op )
			->mov( flags::PF, flags::parity( result ).op );

		store_operand( block, insn, 0, result );
	}

	void process_shr( basic_block* block, const instruction_info& insn )
	{
		auto lhs = load_operand( block, insn, 0 );
		auto rhs = load_operand( block, insn, 1 );

		auto result = ( operative( lhs ) >> ( operative( rhs ) & ( lhs.size( ) == 8 ? 0x3F : 0x1F ) ) ).op;

		block
			->mov( flags::CF, ( operative( lhs ) & 1 ).op )
			->mov( flags::OF, flags::sign( { lhs } ).op )
			->mov( flags::SF, flags::sign( result ).op )
			->mov( flags::ZF, flags::zero( result ).op )
			->mov( flags::PF, flags::parity( result ).op );

		store_operand( block, insn, 0, result );
	}


	void process_adc( basic_block* block, const instruction_info& insn )
	{
		auto lhs = load_operand( block, insn, 0 );
		auto rhs = load_operand( block, insn, 1 );

		auto tmp = block->tmp( lhs.bit_count( ) );

		block
			->mov( tmp, lhs )
			->add( tmp, rhs )
			->add( tmp, flags::CF );

		process_flags<flags::flag_operation::add>( block, lhs, rhs, tmp );

		store_operand( block, insn, 1, { tmp } );
	}
}