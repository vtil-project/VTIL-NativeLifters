// x86 memory transfer instructions.
// 

namespace vtil::lifter::amd64
{
	void process_mov( basic_block* block, const instruction_info& insn )
	{
		block
			->mov( get_operand( block, insn, 0 ), get_operand( block, insn, 1 ) );
	}
}