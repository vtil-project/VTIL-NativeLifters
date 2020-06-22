#include <lifters/core>
#include <lifters/amd64>

#include <vtil/vtil>

#include <memory>

using namespace vtil;

struct byte_input
{
	uint8_t* bytes;
	uint64_t size;

	bool get_at( vip_t offs, uint8_t*& out_val )
	{
		if ( offs >= size )
			return false;

		out_val = &bytes[ offs ];
		return true;
	}
};

using amd64_recursive_descent = lifter::recursive_descent<byte_input, lifter::amd64::lifter_t>;

uint8_t code [ ] { 0x48, 0x31, 0xC0, 0x50, 0x59, 0xFF, 0x21 };

int main( int argc, char** argv )
{
	lifter::amd64::initialize_mappings( );

	byte_input input = { code, sizeof(code) };

	lifter::recursive_descent<byte_input, lifter::amd64::lifter_t> rec_desc( &input, 0 );
	rec_desc.populate( rec_desc.entry );

	optimizer::apply_all( rec_desc.entry->owner );

	debug::dump( rec_desc.entry->owner );
}