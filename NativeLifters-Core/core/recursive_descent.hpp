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
#pragma once
#include <vtil/arch>
#include <vtil/compiler>

#include <unordered_set>
#include <deque>
#include "processing_flags.hpp"

namespace vtil::lifter
{
	struct byte_input
	{
		uint8_t* bytes = nullptr;
		uint64_t size = 0;
		uint64_t base = 0;

		bool is_valid( vip_t vip ) const
		{
			return vip >= base && ( vip - base ) < size;
		}

		uint8_t* get_at( vip_t vip ) const
		{
			dassert( is_valid( vip ) );
			return &bytes[ vip - base ];
		}
	};

	// Generic recursive descent parser used for exploring control flow.
	//
	template<typename input_type, typename arch>
	struct recursive_descent
	{
		// Input descriptor.
		//
		const input_type* input;

		// Entry block.
		//
		basic_block* entry;

		// Instructions corresponding to their basic blocks.
		//
		std::unordered_map<uint64_t, basic_block*> leaders;

		// Constructor.
		//
		recursive_descent( const input_type* input, uint64_t entry_point, processing_flags flags = {} ) : input( input ), leaders( { } )
		{
			entry = basic_block::begin( entry_point );
			entry->owner->alloc( 64 ); // reserve one internal for RIP.
			entry->owner->context.get<processing_flags>() = flags;
		}

		// Start recursive descent.
		//
		void populate( basic_block* start_block )
		{
			// While the basic block is not complete, populate with instructions.
			//
			uint64_t vip = start_block->entry_vip;
			uint8_t* entry_ptr = input->get_at( vip );

			leaders.emplace( vip, start_block );

			while ( true )
			{
				if ( !input->is_valid( vip ) )
				{
					start_block->vexit( vip );
					return;
				}

				auto offs = arch::process( start_block, vip, entry_ptr );
				entry_ptr += offs;
				vip += offs;

				if ( start_block->is_complete() )
				{
					if ( start_block->back().base == &ins::vxcall )
						return populate( start_block->fork( vip ) );
					else if ( start_block->back().base == &ins::vexit )
						return;
					else
						break;
				}
				
				if ( auto ldr = leaders.find( vip ); ldr != leaders.cend( ) )
				{
					start_block
						->jmp( vip )
						->fork( vip );
					break;
				}
			}

			// Try to explore branches.
			// - Do not set resolving of opaques since this block can be jumped into 
			//   later on, we cannot make these kind of assumptions in this scope.
			//
			cached_tracer local_tracer = {};
			auto lbranch_info = optimizer::aux::analyze_branch( 
				start_block, 
				&local_tracer, 
				{ .cross_block = true, .pack = true } 
			);
			fassert( !lbranch_info.is_vm_exit );

			// If not all constants, vmexit, declare preserve all.
			//
			for ( auto branch : lbranch_info.destinations )
			{
				if ( !branch->is_constant() )
				{
					fassert( start_block->back().base == &ins::jmp );
					start_block->wback().base = &ins::vexit;
					start_block->owner->routine_convention = amd64::preserve_all_convention;
					return;
				}
			}

			// Follow the branches.
			//
			for ( auto branch : lbranch_info.destinations )
			{
				const auto branch_imm = *branch->get<vip_t>();
				if ( auto next_blk = start_block->fork( branch_imm ) )
				{
					if ( input->is_valid( branch_imm ) )
						populate( next_blk );
					else
						next_blk->vexit( branch_imm );
				}
			}
		}

		void explore()
		{
			populate( entry );
			//std::unordered_set<basic_block*> entries { entry };
			//
			//bool changed;
			//do
			//{
			//	changed = false;
			//
			//	// Populate entries.
			//	//
			//	for ( auto entry_blk : entries )
			//		populate( entry_blk );
			//
			//	// Clear entries vector.
			//	//
			//	entries.clear();
			//
			//	// For every block in the routine
			//	//
			//	for ( auto [vip, explored_block] : entry->owner->explored_blocks )
			//	{
			//		// Try to explore branches.
			//		//
			//		cached_tracer local_tracer = {};
			//		auto lbranch_info = optimizer::aux::analyze_branch( explored_block, &local_tracer, true, true );
			//
			//		// For every branch
			//		//
			//		for ( auto branch : lbranch_info.destinations )
			//		{
			//			// Is the branch result a constant/
			//			//
			//			if ( branch.is_constant() )
			//			{
			//				const auto branch_imm = *branch.get< uint64_t >();
			//
			//				// If we're not jumping to invalid_vip
			//				if ( branch_imm == invalid_vip )
			//					continue;
			//
			//				// Have we found something unexplored?
			//				//
			//
			//				if ( leaders.find( branch_imm ) == leaders.cend())
			//				{
			//					// Set to changed, add to entries.
			//					//
			//
			//					changed = true;
			//					entries.emplace( entry->fork( branch_imm ));
			//				}
			//			}
			//		}
			//	}
			//}
			//while ( changed );
		}
	};
}