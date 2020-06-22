#include "../amd64.hpp"
#include "../flags.hpp"

namespace vtil::lifter::amd64
{
	void process_invalid( basic_block* block, const instruction_info& insn )
	{
		block->vemits( "int 0xB" );
	}

	void process_mov( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
	}

	void initialize_misc( )
	{
		operand_mappings[ X86_INS_INVALID ] = process_invalid;
		operand_mappings[ X86_INS_MOV ] = process_mov;
	}
}