#include <lifters/amd64>

#include <vtil/vtil>

using namespace vtil;

int main( int argc, char** argv )
{
	basic_block* blk = basic_block::begin( 0 );

	uint8_t code [ ] { 0x48, 0x01, 0xC1 };

	auto insns = capstone::disasm( code, 0, sizeof( code ) );
	for ( auto& ins : insns )
	{
		printf( "%s\n", ins.to_string().c_str() );
		lifter::amd64::process( ins, blk );
	}

	//optimizer::apply_all( blk );

	debug::dump( blk );
}