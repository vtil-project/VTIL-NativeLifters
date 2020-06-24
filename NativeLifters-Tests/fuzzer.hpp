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
#pragma once
#include <lifters/core>
#include <lifters/amd64>
#include <random>
#include "emulator/emulator.hpp"
#include "emulator/rwx_allocator.hpp"

using namespace vtil;

using amd64_recursive_descent = lifter::recursive_descent<lifter::byte_input, lifter::amd64::lifter_t>;

static void fuzz_step( const lifter::byte_input& input )
{
	std::vector GP_REGS = {
		X86_REG_RAX,
		X86_REG_RBP,
		X86_REG_RBX,
		X86_REG_RCX,
		X86_REG_RDI,
		X86_REG_RDX,
		X86_REG_RSI,
		X86_REG_R8,
		X86_REG_R9,
		X86_REG_R10,
		X86_REG_R11,
		X86_REG_R12,
		X86_REG_R13,
		X86_REG_R14,
		X86_REG_R15,
	};

	auto gen_random = [ ] () -> uint64_t
	{
		// TODO: Improve to prioritize edge cases.
		//
		static std::random_device rd;
		static std::mt19937 gen( rd() );
		static std::uniform_int_distribution<uint64_t> distrib{};

		switch ( distrib( gen ) % 7 )
		{
			case 0: return distrib( gen );
			case 1: return 0x4A403F5FA2C7490D;
			case 2: return 0xFFFFFFFFFFFFFFFF;
			case 3: return 0x7FFFFFFFFFFFFFFF;
			case 4: return 0x7FFFFFFF;
			case 5: return 0x7FFF;
			case 6: return 0x7F;
		}
		unreachable();
	};

	// Lift all bytes
	//
	amd64_recursive_descent rec_desc( &input, 0 );
	rec_desc.entry->owner->routine_convention = amd64::preserve_all_convention;
	rec_desc.entry->owner->routine_convention.purge_stack = false;
	rec_desc.explore();
	auto rtn = rec_desc.entry->owner;

	// Hardware emulator.
	//
	emulator emu;

	// Symbolic virtual machine.
	//
	lambda_vm<symbolic_vm> vm;
	vm.hooks.execute = [ & ] ( const instruction& ins )
	{
		// If an unknown instruction is hit:
		//
		if ( *ins.base == vtil::ins::vemit ) unreachable();

		// If hint is hit, skip.
		//
		if ( *ins.base == vtil::ins::vpinr ) return true;
		if ( *ins.base == vtil::ins::vpinw ) return true;

		// If branch is hit, exit VM if jcc/jmp, continue if vmexit. 
		//
		if ( ins.base->is_branching_virt() ) return false;
		if ( ins.base->is_branching_real() ) return true;

		// If none matches, redirect to original handler.
		//
		return vm.symbolic_vm::execute( ins );
	};

	// Set I/O.
	//
	for ( operand op : GP_REGS )
	{
		uint64_t value = gen_random();
		emu.set( ( x86_reg ) op.reg().local_id, value );
		vm.write_register( op.reg(), value );
	}

	// Debug
	//
	//optimizer::apply_all( rtn );

	// Begin executing in the virtual machine:
	//
	auto it = rtn->entry_point->begin();
	while ( true )
	{
		// Run until it VM exits:
		//
		auto lim = vm.run( it, true );
		if ( lim.is_end() ) break;

		auto get_imm = [ & ] ( const operand& op ) -> vip_t
		{
			if ( op.is_immediate() )
				return op.imm().u64;
			return *vm.read_register( op.reg() ).get<vip_t>();
		};

		// Handle JCC:
		//
		if ( *lim->base == ins::js )
		{
			vip_t next = *vm.read_register( lim->operands[ 0 ].reg() ).get<bool>()
				? get_imm( lim->operands[ 1 ] )
				: get_imm( lim->operands[ 2 ] );
			it = rtn->explored_blocks[ next ]->begin();
			vm.write_register( REG_SP, vm.read_register( REG_SP ) + lim.container->sp_offset );
			continue;
		}
		// Handle JMP:
		//
		else if ( *lim->base == ins::jmp )
		{
			it = rtn->explored_blocks[ get_imm( lim->operands[ 0 ] ) ]->begin();
			vm.write_register( REG_SP, vm.read_register( REG_SP ) + lim.container->sp_offset );
			continue;
		}

		// Shouldn't be reached.
		//
		unreachable();
	}
	
	// Dump some info.
	//
	//debug::dump( rtn );
	//for ( auto& [k, v] : vm.register_state )
	//	logger::log( "%s => %s\n", k, v );

	// Begin executing in the hardware emulator.
	//
	std::vector<uint8_t, mem::rwx_allocator<uint8_t>> test = { 
		input.bytes, 
		input.bytes + input.size 
	};
	test.push_back( 0xC3 );
	emu.invoke( test.data() );

	bool passed = true;
	// Print register state:
	//
	using namespace vtil::logger;
	for ( operand op : GP_REGS )
	{
		uint64_t emu_v = emu.get( ( x86_reg ) op.reg().local_id );
		uint64_t vm_v = *vm.read_register( op.reg() ).get<uint64_t>();
		if ( emu_v != vm_v )
		{
			log<CON_BRG>( "%-8s: ", op );
			log<CON_GRN>( "%p ", emu.get( ( x86_reg ) op.reg().local_id ) );
			log<CON_RED>( "%p\n", *vm.read_register( op.reg() ).get<uint64_t>() );
			passed = false;
		}
	}

	// Push flag state:
	//
	math::bit_vector emu_v = math::bit_vector{ emu.v_rflags, 32 };
	math::bit_vector vm_v = vm.read_register( REG_FLAGS ).value.resize( 32 );
	if ( ( vm_v.known_mask() & emu.v_rflags ) != vm_v.known_one() )
	{
		log<CON_BRG>( "%-8s: ", "eflags" );
		log<CON_GRN>( "%s\n", math::bit_vector{ emu.v_rflags, 32 } );
		log<CON_RED>( "          %s\n", vm.read_register( REG_FLAGS ).value.resize( 32 ) );
		passed = false;
	}
	
	if ( passed )
		log<CON_PRP>( "Test passed!\n" );
	else
		log<CON_RED>( "Test failed!\n" );
}