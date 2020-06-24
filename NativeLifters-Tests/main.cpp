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
#include "fuzzer.hpp"

using namespace vtil;
using amd64_recursive_descent = lifter::recursive_descent<lifter::byte_input, lifter::amd64::lifter_t>;

int main( int argc, char** argv )
{
	uint8_t code[] = { 0x48, 0x89, 0xD8, 0x48, 0xFF, 0xC0, 0x53, 0x50, 0x48, 0x8D, 0x64, 0x24, 0x08, 0x59, 0x48, 0x01, 0xD9, 0x48, 0x0F, 0xCB, 0x0F, 0xCB, 0x48, 0x19, 0xC3, 0x48, 0xC1, 0xDB, 0x03 };
	lifter::byte_input input = { code, sizeof(code) };

	auto dasm = amd64::disasm( code, 0, sizeof( code ) );
	for ( auto& ins : dasm )
		logger::log( "%s\n", ins.to_string() );

	for ( int i = 0; i < 128; i++ )
		fuzz_step( input );

	amd64_recursive_descent rec_desc( &input, 0 );
	rec_desc.entry->owner->routine_convention = amd64::preserve_all_convention;
	rec_desc.entry->owner->routine_convention.purge_stack = false;
	rec_desc.explore();

	debug::dump( rec_desc.entry->owner );
	return 0;
}