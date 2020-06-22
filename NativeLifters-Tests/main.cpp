#include <lifters/amd64>

#include <vtil/vtil>

using namespace vtil;

int main( int argc, char** argv )
{
	basic_block* blk = basic_block::begin( 0 );
	blk->owner->routine_convention = preserve_all_convention;

	uint8_t code [ ] { 0x48, 0xC7, 0xC0, 0x0A, 0x00, 0x00, 0x00, 0x48, 0xC1, 0xD9, 0x0B, 0x48, 0xC7, 0xC1, 0x01, 0x00, 0x00, 0x00, 0x48, 0xC1, 0xC0, 0x04, 0x48, 0xC1, 0xC8, 0x04, 0xFF, 0xE0 };

	lifter::amd64::initialize_mappings( );

	auto insns = amd64::disasm( code, 0, sizeof( code ) );
	for ( auto& ins : insns )
	{
		printf( "[LIFT] %s\n", ins.to_string().c_str() );
		lifter::amd64::process( ins, blk );
	}

	if (!blk->is_complete( ))
		blk->vexit( 0ULL );

	debug::dump( blk );

	optimizer::apply_all( blk->owner );

	debug::dump( blk );
}