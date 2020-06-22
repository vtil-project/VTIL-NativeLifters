#include "../amd64.hpp"
#include "../flags.hpp"

// Flag manipulation instructions.
//
namespace vtil::lifter::amd64
{
	void process_clc( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::CF, 0 );
	}

	void process_cld( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::DF, 0 );
	}

	void process_cli( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::IF, 0 );
	}

	void process_stc( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::CF, 1 );
	}

	void process_std( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::DF, 1 );
	}

	void process_sti( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( flags::IF, 1 );
	}

	void process_cmc( basic_block* block, const instruction_info& insn )
	{
		block
			->bnot( flags::CF );
	}

	void process_seta( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( ( cf == 0 ) & ( zf == 0 ) ).op );
	}

	void process_setae( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );

		block
			->mov( load_operand( block, insn, 0 ), ( cf == 0 ).op );
	}


	void process_setb( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );

		block
			->mov( load_operand( block, insn, 0 ), ( cf == 1 ).op );
	}

	void process_setbe( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( ( cf == 1 ) | ( zf == 1 ) ).op );
	}

	void process_sete( basic_block* block, const instruction_info& insn )
	{
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( zf == 1 ).op );
	}

	void process_setge( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );

		block
			->mov( load_operand( block, insn, 0 ), (sf == of).op );
	}

	void process_setg( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( ( zf == 0 ) & ( sf == of ) ).op );
	}

	void process_setle( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( ( zf == 1 ) & ( sf != of ) ).op );
	}

	void process_setl( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( ( sf != of ) ).op );
	}

	void process_setne( basic_block* block, const instruction_info& insn )
	{
		operative zf( flags::ZF );

		block
			->mov( load_operand( block, insn, 0 ), ( zf == 0 ).op );
	}

	void process_setno( basic_block* block, const instruction_info& insn )
	{
		operative of( flags::OF );

		block
			->mov( load_operand( block, insn, 0 ), ( of == 0 ).op );
	}

	void process_setnp( basic_block* block, const instruction_info& insn )
	{
		operative pf( flags::PF );

		block
			->mov( load_operand( block, insn, 0 ), ( pf == 0 ).op );
	}

	void process_setns( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );

		block
			->mov( load_operand( block, insn, 0 ), ( sf == 0 ).op );
	}

	void process_seto( basic_block* block, const instruction_info& insn )
	{
		operative of( flags::OF );

		block
			->mov( load_operand( block, insn, 0 ), ( of == 1 ).op );
	}

	void process_setp( basic_block* block, const instruction_info& insn )
	{
		operative pf( flags::PF );

		block
			->mov( load_operand( block, insn, 0 ), ( pf == 1 ).op );
	}

	void process_sets( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );

		block
			->mov( load_operand( block, insn, 0 ), ( sf == 1 ).op );
	}

	void initialize_flags( )
	{
		operand_mappings[ X86_INS_CLC ] = process_clc;
		operand_mappings[ X86_INS_CLD ] = process_cld;
		operand_mappings[ X86_INS_CLI ] = process_cli;
		operand_mappings[ X86_INS_STC ] = process_stc;
		operand_mappings[ X86_INS_STD ] = process_std;
		operand_mappings[ X86_INS_STI ] = process_sti;
		operand_mappings[ X86_INS_CMC ] = process_cmc;
		operand_mappings[ X86_INS_SETA ] = process_seta;
		operand_mappings[ X86_INS_SETAE ] = process_setae;
		operand_mappings[ X86_INS_SETB ] = process_setb;
		operand_mappings[ X86_INS_SETBE ] = process_setbe;
		operand_mappings[ X86_INS_SETE ] = process_sete;
		operand_mappings[ X86_INS_SETGE ] = process_setge;
		operand_mappings[ X86_INS_SETG ] = process_setg;
		operand_mappings[ X86_INS_SETLE ] = process_setle;
		operand_mappings[ X86_INS_SETL ] = process_setl;
		operand_mappings[ X86_INS_SETNE ] = process_setne;
		operand_mappings[ X86_INS_SETNO ] = process_setno;
		operand_mappings[ X86_INS_SETNP ] = process_setnp;
		operand_mappings[ X86_INS_SETNS ] = process_setns;
		operand_mappings[ X86_INS_SETO ] = process_seto;
		operand_mappings[ X86_INS_SETP ] = process_setp;
		operand_mappings[ X86_INS_SETS ] = process_sets;
	}
}