// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of mosquitto nor the names of its   
//    contributors may be used to endorse or promote products derived from   
//    this software without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#include <lifters/core>
#include <lifters/amd64>
#include <vtil/arch>
#include <vtil/compiler>
#include <memory>

using namespace vtil;

struct byte_input
{
	uint8_t* bytes;
	uint64_t size;

	bool is_valid( vip_t vip )
	{
		return vip < size;
	}

	uint8_t* get_at( vip_t offs )
	{
		return &bytes[ offs ];
	}
};

using amd64_recursive_descent = lifter::recursive_descent<byte_input, lifter::amd64::lifter_t>;

uint8_t code [ ] { 0xE8, 0x02, 0x00, 0x00, 0x00, 0xEB, 0x01, 0xC3, 0x31, 0xC0, 0xE8, 0xF8, 0xFF, 0xFF, 0xFF };

int main( int argc, char** argv )
{
	printf( "handler count: %lld\n", lifter::amd64::instruction_handlers.size() );

	byte_input input = { code, sizeof(code) };

	lifter::recursive_descent<byte_input, lifter::amd64::lifter_t> rec_desc( &input, 0 );
	rec_desc.entry->owner->routine_convention = amd64::preserve_all_convention;
	rec_desc.entry->owner->routine_convention.purge_stack = false;
	rec_desc.populate( rec_desc.entry );

	debug::dump( rec_desc.entry->owner );

	printf( "\nOPTIMIZING...\n\n" );

	optimizer::apply_all( rec_desc.entry->owner );

	debug::dump( rec_desc.entry->owner );
	return 0;
}