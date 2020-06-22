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
		auto lhs = get_operand( block, insn, 0 ); \
		auto rhs = get_operand( block, insn, 1 ); \
		auto tmp = block->tmp( lhs.bit_count( ) ); \
		block->mov( tmp, lhs )->op( tmp, rhs ); \
		process_flags<flags::flag_operation::##op>( block, lhs, rhs, tmp ); \
		block->mov( lhs, tmp ); \
	}

	PROCESS_BINOP( add, add );
	PROCESS_BINOP( sub, sub );
	//PROCESS_BINOP( mul, mul );
	//PROCESS_BINOP( imul, imul );
	//PROCESS_BINOP( and, band );
	//PROCESS_BINOP( or, bor );
	//PROCESS_BINOP( xor, bxor );
	//PROCESS_BINOP( shl, bshl );
	//PROCESS_BINOP( shr, bshr );

	void process_adc( basic_block* block, const instruction_info& insn )
	{
		auto lhs = get_operand( block, insn, 0 );
		auto rhs = get_operand( block, insn, 1 );

		auto tmp = block->tmp( lhs.bit_count( ) );

		block
			->mov( tmp, lhs )
			->add( tmp, rhs )
			->add( tmp, flags::CF );

		process_flags<flags::flag_operation::add>( block, lhs, rhs, tmp );

		block
			->mov( lhs, tmp );
	}
}