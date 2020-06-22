#include <unordered_set>
#include <deque>

#include <vtil/vtil>

namespace vtil::lifter
{
	// Generic recursive descent parser used for exploring control flow.
	//

	template<typename Input, typename Arch>
	struct recursive_descent
	{
		// Input descriptor.
		//
		Input* input;

		// Entry block.
		//
		basic_block* entry;

		// Instructions corresponding to their basic blocks.
		//
		std::unordered_map<uint64_t, basic_block*> leaders;

		// Start recursive descent.
		//
		void populate( basic_block* start_block )
		{
			// While the basic block is not complete, populate with instructions.
			//
			uint64_t vip = start_block->entry_vip;
			uint8_t* entry_ptr = nullptr;
			if ( !input->get_at( vip, entry_ptr ) )
			{
				start_block->vexit( -1ULL );
				return;
			}

			leaders.emplace( vip, start_block );

			while ( !start_block->is_complete( ) )
			{
				auto offs = Arch::process( start_block, vip, entry_ptr );

				entry_ptr += offs;
				vip += offs;
				
				if ( auto ldr = leaders.find( vip ); ldr != leaders.cend( ) )
				{
					start_block
						->jmp( vip )
						->fork( vip );

					break;
				}
			}

			// Run local optimizer passes on this block.
			//
			optimizer::apply_all( start_block );

			// Try to explore branches.
			//
			cached_tracer local_tracer = {};
			auto lbranch_info = optimizer::aux::analyze_branch( start_block, &local_tracer, true, true );

			for ( auto branch : lbranch_info.destinations )
			{
				if ( branch.is_constant( ) )
				{
					const auto branch_imm = *branch.get<uint64_t>( );
					if ( auto next_blk = start_block->fork( branch_imm ) )
					{
						printf( "target: %llx\n", branch_imm );
						populate( next_blk );
					}
				}
			}
		}

		recursive_descent( Input* input, uint64_t entry_point ) : input(input), leaders( { } )
		{
			entry = basic_block::begin( entry_point );
		}
	};
}