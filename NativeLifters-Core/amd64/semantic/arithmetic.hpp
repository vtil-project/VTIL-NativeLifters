// Various x86 arithmetic instructions.
// 

namespace vtil::lifter::amd64
{
	// Process the flags for a specific arithmetic instruction.
	//
	template<flags::flag_operation Op>
	void process_flags( basic_block* block, const operative& lhs, const operative& rhs, const operative& result )
	{
		operative { flags::OF, block } = flags::overflow<Op>::flag( lhs, rhs, result );
		operative { flags::CF, block } = flags::carry<Op>::flag( lhs, rhs, result );
		operative { flags::SF, block } = flags::sign( result );
		operative { flags::ZF, block } = flags::zero( result );
		operative { flags::AF, block } = flags::aux_carry( lhs, rhs, result );
		operative { flags::PF, block } = flags::parity( result );
	}

	void process_add( basic_block* block, const instruction_info& insn )
	{
		// Define & clone writeable lhs, define rhs
		//
		operative lhs = { get_operand( block, insn, 0 ), block };
		operative rhs = { get_operand( block, insn, 1 ), block };

		operative lhs_copy = lhs.clone( );

		// Perform operation
		//
		lhs += rhs;

		// Process flags values
		// 
		process_flags<flags::flag_operation::add>( block, lhs, rhs, lhs_copy );
	}

	void process_sub( basic_block* block, const instruction_info& insn )
	{
		// Define & clone writeable lhs, define rhs
		//
		operative lhs = { get_operand( block, insn, 0 ), block };
		operative rhs = { get_operand( block, insn, 1 ), block };

		operative lhs_copy = lhs.clone( );

		// Perform operation
		//
		lhs -= rhs;

		// Process flags values
		// 
		process_flags<flags::flag_operation::sub>( block, lhs, rhs, lhs_copy );
	}
}