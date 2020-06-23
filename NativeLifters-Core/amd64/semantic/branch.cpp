#include "../amd64.hpp"
#include "../flags.hpp"

// Branching instructions.
// 

namespace vtil::lifter::amd64
{
	void process_jmp( basic_block* block, const instruction_info& insn )
	{
		block->jmp( load_operand( block, insn, 0 ) );
	}

	void process_call( basic_block* block, const instruction_info& insn )
	{

	}

	void process_ret( basic_block* block, const instruction_info& insn )
	{

	}

	void initialize_branch( )
	{
		operand_mappings[ X86_INS_JMP ] = process_jmp;
		operand_mappings[ X86_INS_CALL ] = process_call;
		operand_mappings[ X86_INS_RET ] = process_ret;
	}
}