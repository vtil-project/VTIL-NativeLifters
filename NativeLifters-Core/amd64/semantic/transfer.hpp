// x86 memory transfer instructions.
// 

namespace vtil::lifter::amd64
{
	void process_mov( basic_block* block, const instruction_info& insn )
	{
		store_operand( block, insn, 0, load_operand( block, insn, 1 ) );
	}
}