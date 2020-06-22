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

	//TODO: SETcc

	void initialize_flags( )
	{
		operand_mappings[ X86_INS_CLC ] = process_clc;
		operand_mappings[ X86_INS_CLD ] = process_cld;
		operand_mappings[ X86_INS_CLI ] = process_cli;
		operand_mappings[ X86_INS_STC ] = process_stc;
		operand_mappings[ X86_INS_STD ] = process_std;
		operand_mappings[ X86_INS_STI ] = process_sti;
		operand_mappings[ X86_INS_CMC ] = process_cmc;
	}
}