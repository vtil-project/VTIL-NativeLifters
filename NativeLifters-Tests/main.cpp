#include <lifters/core>
#include <lifters/amd64>

#include <vtil/vtil>

#include <memory>

using namespace vtil;

struct byte_input
{
	uint8_t* bytes;
	uint64_t size;

	bool is_valid( vip_t vip )
	{
		printf( "check %llx\n", vip );
		return vip < size;
	}

	uint8_t* get_at( vip_t offs )
	{
		printf( "read %llx\n", offs );
		return &bytes[ offs ];
	}
};

using amd64_recursive_descent = lifter::recursive_descent<byte_input, lifter::amd64::lifter_t>;

uint8_t code [ ] { 0x48, 0x31, 0xC0, 0x48, 0xFF, 0xC0, 0x48, 0xF7, 0xD8, 0x48, 0x99 };

int main( int argc, char** argv )
{
	lifter::amd64::initialize_mappings( );

	byte_input input = { code, sizeof(code) };

	lifter::recursive_descent<byte_input, lifter::amd64::lifter_t> rec_desc( &input, 0 );
	rec_desc.populate( rec_desc.entry );

	optimizer::apply_all( rec_desc.entry->owner );

	debug::dump( rec_desc.entry->owner );
}