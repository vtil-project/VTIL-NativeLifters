#include "../amd64.hpp"
#include "../flags.hpp"

// Various x86 comparison instructions.
// 

namespace vtil::lifter::amd64
{
	void process_cmp( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( block->tmp( lhs.op.bit_count( ) ) );

		block
			->movsx( rhs.op, load_operand( block, insn, 1 ) );

		auto result = lhs - rhs;

		block
			->mov( flags::CF, flags::carry<flags::sub>::flag( lhs, rhs, result ).op )
			->mov( flags::OF, flags::overflow<flags::sub>::flag( lhs, rhs, result ).op )
			->mov( flags::SF, flags::sign( result ).op )
			->mov( flags::ZF, flags::zero( result ).op )
			->mov( flags::AF, flags::aux_carry( lhs, rhs, result ).op )
			->mov( flags::PF, flags::parity( result ).op );
	}

	void process_test( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = lhs & rhs;

		block
			->mov( flags::CF, 0 )
			->mov( flags::OF, 0 )
			->mov( flags::SF, flags::sign( result ).op )
			->mov( flags::ZF, flags::zero( result ).op )
			->mov( flags::PF, flags::parity( result ).op );
	}

	void initialize_comparison( )
	{
		operand_mappings[ X86_INS_CMP ] = process_cmp;
		operand_mappings[ X86_INS_TEST ] = process_test;
	}
}