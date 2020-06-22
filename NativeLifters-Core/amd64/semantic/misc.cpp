#include "../amd64.hpp"
#include "../flags.hpp"

namespace vtil::lifter::amd64
{
	void process_invalid( basic_block* block, const instruction_info& insn )
	{
		block->vemits( "int 0xB" );
	}

	void process_lea( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, get_disp_from_operand( block, insn.operands[ 1 ] ) );
	}

	void process_mov( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
	}

	void process_movsx( basic_block* block, const instruction_info& insn )
	{
		auto result = block->tmp( insn.operands[ 0 ].size * 8 );
		block->movsx( result, load_operand( block, insn, 1 ) );
		store_operand( block, insn, 0, result );
	}

	void initialize_misc( )
	{
		operand_mappings[ X86_INS_INVALID ] = process_invalid;
		operand_mappings[ X86_INS_LEA ] = process_lea;
		operand_mappings[ X86_INS_MOV ] = process_mov;
		operand_mappings[ X86_INS_MOVABS ] = process_mov;
		operand_mappings[ X86_INS_MOVZX ] = process_mov;
		operand_mappings[ X86_INS_MOVSX ] = process_movsx;
		operand_mappings[ X86_INS_MOVSXD ] = process_movsx;
	}
}