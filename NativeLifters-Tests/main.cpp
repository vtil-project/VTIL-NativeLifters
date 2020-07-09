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
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
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
using namespace logger;
using amd64_recursive_descent = lifter::recursive_descent<lifter::byte_input, lifter::amd64::lifter_t>;

static bool run_test(uint64_t address, const char* assembly, const char* file, int line, bool optimize, bool dump_info)
{
	std::vector<uint8_t> code = amd64::assemble(assembly);
	if (code.empty())
	{
		log<CON_RED>("Failed to assemble (%s:%d):\n", file, line);
		log("%s\n", assembly);
		return false;
	}

	
	lifter::byte_input input = { code.data(), code.size(), address };

	auto dasm = amd64::disasm(code.data(), address, code.size());
	for (auto& ins : dasm)
		log("%s\n", ins.to_string());

	auto passed = true;
	for (int i = 0; i < 128; i++)
	{
		if (!fuzz_step(input, optimize, dump_info))
		{
			passed = false;
		}
	}

	if (passed)
	{
		log<CON_GRN>("Test passed!\n\n");
	}
	else
	{
		log("\nVTIL:\n");
		amd64_recursive_descent rec_desc( &input, input.base );
		rec_desc.entry->owner->routine_convention = amd64::preserve_all_convention;
		rec_desc.entry->owner->routine_convention.purge_stack = false;
		rec_desc.explore();

		// Insert proper vexits
		rec_desc.entry->owner->for_each([](basic_block* blk)
		{
			if (blk->stream.back().base == &ins::jmp && blk->next.empty())
				blk->stream.back().base = &ins::vexit;
		});

		debug::dump(rec_desc.entry);

		log<CON_RED>("Test failed! (%s:%d)\n\n", file, line);
	}

	return passed;
}

#ifdef _WIN32
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif // _WIN32

#define TEST_ADDR(address, assembly) tests.push_back({ address, assembly, __FILENAME__, __LINE__ })
#define TEST(assembly) TEST_ADDR(0, assembly)

struct Test
{
	uint64_t address = 0;
	const char* assembly = nullptr;
	const char* file = nullptr;
	int line = 0;
};

static bool runTests()
{
	std::vector<Test> tests;

	TEST(R"(
		push rbx
		mov rbx, rsp

		enter 0x40, 0
		mov rax, rsp
		mov rcx, rbp

		mov rsp, rbx
		pop rbx
	)");
	TEST("mov ebx, eax");
	TEST("xor rax, rbx");
	TEST("and rax, rcx");
	TEST("cmp eax, ebx");
	TEST("cmp rbp, rsp");
	TEST(R"(
		enter 0x40, 1
		leave
)");
	TEST_ADDR(0x140001000, R"(push rax
pop rbx)");
	/* TODO: expect failure: TEST(R"(
		enter 0xFFFF, 0
		leave
)");*/

	size_t passed = 0;
	for (size_t i = 0; i < tests.size(); i++)
	{
		const auto& test = tests[i];
		passed += run_test(test.address, test.assembly, test.file, test.line, false, false);
	}

	log("%zu/%zu tests passed\n", passed, tests.size());
	return passed == tests.size();
}

#define EXPERIMENT(address, assembly) run_test(address, assembly, __FILENAME__, __LINE__, false, false)

int main(int argc, char** argv)
{
	if (argc > 1 && strcmp(argv[1], "--tests") == 0)
	{
		return runTests() ? 0 : 1;
	}

	{



		std::vector<uint8_t> code = amd64::assemble( R"(
        xor     ecx, ecx
        xor     eax, eax
        lea     edx, [rax + rcx]
        test    cl, 8
        je      .LBB0_4
        jmp     .LBB0_2
.LBB0_7:                               
        add     ecx, 2
        cmp     ecx, 512
        je      .LBB0_10
        lea     edx, [rax + rcx]
        test    cl, 8
        je      .LBB0_4
.LBB0_2:
        test    cl, 16
        je      .LBB0_8
        or      edx, 1
.LBB0_4:                           
        lea     eax, [rdx + rcx]
        add     eax, 1
        test    cl, 8
        je      .LBB0_7
        test    cl, 16
        je      .LBB0_9
        or      eax, 1
        jmp     .LBB0_7
.LBB0_8:
        add     eax, ecx
        ret
.LBB0_9:
        lea     eax, [rdx + rcx]
        add     eax, 1
.LBB0_10:
        ret

	)" );
		lifter::byte_input input = { code.data(), code.size() };

		auto dasm = amd64::disasm( code.data(), 0, code.size() );
		for ( auto& ins : dasm )
			logger::log( "%s\n", ins.to_string() );

		amd64_recursive_descent rec_desc( &input, 0 );
		rec_desc.entry->owner->routine_convention = amd64::default_call_convention;
		rec_desc.entry->owner->routine_convention.purge_stack = false;
		rec_desc.explore();

		rec_desc.entry->owner->for_each( [ ] ( basic_block* blk )
		{
			if ( blk->stream.back().base == &ins::jmp && blk->next.empty() )
				blk->stream.back().base = &ins::vexit;
		} );

		save_routine( rec_desc.entry->owner, "S:\\test_loop.vtil" );

		optimizer::local_pass<optimizer::collective_local_pass>{}( rec_desc.entry->owner );



		debug::dump( rec_desc.entry->owner );

		optimizer::apply_all_profiled( rec_desc.entry->owner );
		debug::dump( rec_desc.entry->owner );
	}




	// Experiment with things
	EXPERIMENT(0x140001000, "mov rax, 0x1234");

	return 0;
}