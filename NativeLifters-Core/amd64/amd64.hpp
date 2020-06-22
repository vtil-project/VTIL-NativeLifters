#include <vtil/amd64>
#include <vtil/arch>

#include <capstone/capstone.h>

#include <map>

// This file defines any global arch-specific information for the AMD64 target
// architecture.
//
namespace vtil::lifter::amd64
{
	using instruction_info = vtil::amd64::instruction;
	using operand_info = cs_x86_op;

	inline std::map<x86_insn, void( * )( basic_block* block, const instruction_info& insn )> operand_mappings;

	void initialize_mappings( );

	register_desc get_disp_from_operand( basic_block* block, const operand_info& operand );
	operand load_operand( basic_block* block, const instruction_info& insn, size_t idx );
	void store_operand( basic_block* block, const instruction_info& insn, size_t idx, const operand& source );
	register_desc get_rip_relative( basic_block* block, const instruction_info& insn, size_t idx );

	void process( instruction_info& insn, basic_block* block );
}