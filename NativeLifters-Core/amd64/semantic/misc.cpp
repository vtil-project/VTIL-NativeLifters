#include "../amd64.hpp"
#include "../flags.hpp"

namespace vtil::lifter::amd64
{
	void process_invalid( basic_block* block, const instruction_info& insn )
	{
		block->vemits( "int 0xB" );
		block->vexit( -1ULL );
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

	void process_push( basic_block* block, const instruction_info& insn )
	{
		auto oper = load_operand( block, insn, 0 );

		if ( oper.bit_count( ) == 16 )
		{
			block
				->sub( X86_REG_RSP, 2 );
		}
		else
		{
			block
				->sub( X86_REG_RSP, 8 );
		}

		block
			->str( X86_REG_RSP, 0, oper );
	}

	void process_pop( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { X86_REG_RSP } );

		if ( insn.operands[ 0 ].size == 2 )
		{
			block
				->add( X86_REG_RSP, 2 );
		}
		else
		{
			block
				->add( X86_REG_RSP, 8 );
		}
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
		operand_mappings[ X86_INS_PUSH ] = process_push;
		operand_mappings[ X86_INS_POP ] = process_pop;
	}
}