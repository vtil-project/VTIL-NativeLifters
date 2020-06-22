#include <lifters/core>
#include <lifters/amd64>

#include <vtil/vtil>

#include <memory>

using namespace vtil;

struct byte_input
{
	uint8_t* bytes;
	uint8_t* get_at( vip_t offs )
	{
		return &bytes[ offs ];
	}
};

using amd64_recursive_descent = lifter::recursive_descent<byte_input, lifter::amd64::lifter_t>;

uint8_t code [ ] { 0xEB, 0x00, 0x48, 0x8B, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x00, 0xFF, 0xE0 };

int main( int argc, char** argv )
{
	lifter::amd64::initialize_mappings( );

	byte_input input = { code };

	lifter::recursive_descent<byte_input, lifter::amd64::lifter_t> rec_desc( &input, 0 );
	rec_desc.populate( rec_desc.entry );

	debug::dump( rec_desc.entry );
}