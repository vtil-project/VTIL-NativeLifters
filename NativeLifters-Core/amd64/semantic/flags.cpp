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

		store_operand( block, insn, 0, ( ( cf == 0 ) & ( zf == 0 ) ).op );
	}

	void process_setae( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );

		store_operand( block, insn, 0, ( cf == 0 ).op );
	}


	void process_setb( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::CF } );
	}

	void process_setbe( basic_block* block, const instruction_info& insn )
	{
		operative cf( flags::CF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( cf | zf ).op );
	}

	void process_sete( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::ZF } );
	}

	void process_setge( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );

		store_operand( block, insn, 0, ( sf & of ).op );
	}

	void process_setg( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( ( zf == 0 ) & ( sf == of ) ).op );
	}

	void process_setle( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( zf & ( sf != of ) ).op );
	}

	void process_setl( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );
		operative of( flags::OF );
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( sf != of ).op );
	}

	void process_setne( basic_block* block, const instruction_info& insn )
	{
		operative zf( flags::ZF );

		store_operand( block, insn, 0, ( zf == 0 ).op );
	}

	void process_setno( basic_block* block, const instruction_info& insn )
	{
		operative of( flags::OF );

		store_operand( block, insn, 0 , ( of == 0 ).op );
	}

	void process_setnp( basic_block* block, const instruction_info& insn )
	{
		operative pf( flags::PF );

		store_operand( block, insn, 0 , ( pf == 0 ).op );
	}

	void process_setns( basic_block* block, const instruction_info& insn )
	{
		operative sf( flags::SF );

		store_operand( block, insn, 0 , ( sf == 0 ).op );
	}

	void process_seto( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::OF } );
	}

	void process_setp( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::PF } );
	}

	void process_sets( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, { flags::SF } );
	}
	
	operative simple_bt( basic_block* block, const operative& lhs, const operative& rhs )
	{
		uint64_t mask;
		switch ( lhs.op.size( ) )
		{
			case 2:
				mask = 0xF;
				break;

			case 4:
				mask = 0x1F;
				break;

			case 8:
				mask = 0x3F;
				break;

			default:
				fassert( false );
				break;
		}

		auto result = rhs & mask;
		block->mov( flags::CF, ( ( lhs& ( 1ULL << result ) ) != 0 ).op );
		return result;
	}

	void process_bt( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		simple_bt( block, lhs, rhs );
	}

	void process_btc( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs ^ ( 1ULL << result ) ).op );
	}

	void process_btr( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs & ~( 1ULL << result ) ).op );
	}

	void process_bts( basic_block* block, const instruction_info& insn )
	{
		auto lhs = operative( load_operand( block, insn, 0 ) );
		auto rhs = operative( load_operand( block, insn, 1 ) );

		auto result = simple_bt( block, lhs, rhs );

		store_operand( block, insn, 0, ( lhs | ( 1ULL << result ) ).op );
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
		operand_mappings[ X86_INS_BT ] = process_bt;
		operand_mappings[ X86_INS_BTC ] = process_btc;
		operand_mappings[ X86_INS_BTR ] = process_btr;
		operand_mappings[ X86_INS_BTS ] = process_bts;
	}
}