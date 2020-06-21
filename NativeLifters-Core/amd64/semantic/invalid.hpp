namespace vtil::lifter::amd64
{
	void process_invalid( basic_block* block )
	{
		block->vemits( "int 0xB" );
	}
}