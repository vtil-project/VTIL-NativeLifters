#include <vtil/amd64>
#include <vtil/arch>

#include <capstone/capstone.h>

#include <map>

// This file defines any global arch-specific information for the AMD64 target
// architecture.
//
namespace vtil
{
	// Allows creation of register_desc's from the x86_reg enumeration
	//
	template<>
	struct register_cast< x86_reg >
	{
		register_desc operator()( x86_reg value )
		{
			auto [base, offset, size] = amd64::resolve_mapping( value );
			return register_desc( register_physical, base, size * 8, offset * 8 );
		}
	};

	namespace lifter
	{
		using instruction_info = vtil::amd64::instruction;
		using operand_info = cs_x86_op;

		namespace amd64
		{
			inline std::map<x86_insn, void( * )( basic_block* block, const instruction_info& insn )> operand_mappings;
			
			void initialize_mappings( );

			register_desc get_disp_from_operand( basic_block* block, const operand_info& operand );
			operand load_operand( basic_block* block, const instruction_info& insn, size_t idx );
			void store_operand( basic_block* block, const instruction_info& insn, size_t idx, const operand& source );

			void process( instruction_info& insn, basic_block* block );
		}
	}
}
