#include <lifters/amd64>

#include <vtil/vtil>

using namespace vtil;

int main( int argc, char** argv )
{
	basic_block* blk = basic_block::begin( 0 );
	blk->owner->routine_convention = preserve_all_convention;

	uint8_t code [ ] { 0x48, 0x31, 0xC0, 0x48, 0x11, 0xC0 };

	auto insns = capstone::disasm( code, 0, sizeof( code ) );
	for ( auto& ins : insns )
	{
		printf( "[LIFT] %s\n", ins.to_string().c_str() );
		lifter::amd64::process( ins, blk );
	}

	blk->vexit( 0ULL );

	debug::dump( blk );

	optimizer::apply_all( blk->owner );

	debug::dump( blk );
}