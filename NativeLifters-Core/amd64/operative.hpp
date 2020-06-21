#pragma once
#include <optional>
#include <vtil/arch>
#include "operations.hpp"

namespace vtil::lifter
{
    // Provides a simple wrapper over operands to support instruction emittion via operators
    //
    class operative : public operand
    {
    private:
        // Backing target basic block for code generation
        //
        basic_block* block = {};

    public:
        // Clones the current instance by copying its contents to a temporary register and
        // returning said register
        //
        operative clone()
        {
            fassert( is_valid() );

            // Create and populate a temp register to hold the current register's value
            // 
            auto tmp = block->tmp( size() );
            block->mov( tmp, decay( ) );

            // Create & return a new operative using the temp register
            //
            return { tmp, block };
        }

        // Executes the popcnt instruction on the current operative
        //
        operative& popcnt()
        {
            fassert( is_valid() );

            block->popcnt( decay( ) );

            return *this;
        }

        // Constructs the operative from a register and basic block
        // Optionally clones the operative on creation
        //
        operative( const operand& desc, basic_block* block, bool autoclone = false )
            : operand( desc ), block( block )
        {
            if ( autoclone )
                *this = clone();
        }

        // Decays back to operand type
        //
        operand& decay()
        {
            return *this;
        }

        const operand& decay( ) const
		{
			return *this;
		}

        explicit operator operand( )
        {
            return decay( );
        }

        // Assignment operators
        //
        template <typename T>
        operative& operator=( T&& other )
        {
            fassert( is_valid() );
            block->mov( decay(), other );
            return *this;
        }
        
        // Comparison operators
        //
		operative operator&&( const operative& other )
		{
			fassert( is_valid( ) );
			auto [a1, a2] = block->tmp( size( ), other.size( ) );

			block->tne( a1, decay( ), 0 );
			block->tne( a2, other.decay( ), 0 );
            block->band( a1, a2 );

            return { a1, block };
		}


        operative operator||( const operative& other )
        {
            fassert( is_valid( ) );
            auto [a1, a2] = block->tmp( size( ), other.size( ) );

            block->tne( a1, decay( ), 0 );
            block->tne( a2, other.decay( ), 0 );
            block->bor( a1, a2 );

            return { a1, block };
        }

        // Equality operators
		//

		template<typename T>
        operative operator==( T&& other ) const
        { 
            fassert( is_valid( ) );
            auto tmp = block->tmp( size( ) );
            block->te( tmp, decay( ), other );
            return { tmp, block };
        }

		template<typename T>
        operative operator!=( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
            block->tne( tmp, decay( ), other );
            return { tmp, block };
        }

		template<typename T>
        operative operator>=( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block->tge( tmp, decay( ), other );
			return { tmp, block };
        }

		template<typename T>
        operative operator>( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block->tg( tmp, decay( ), other );
			return { tmp, block };
        }

		template<typename T>
        operative operator<=( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block->tle( tmp, decay( ), other );
			return { tmp, block };
        }

		template<typename T>
        operative operator<( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block->tl( tmp, decay( ), other );
			return { tmp, block };
        }

		// Binary operators
        //

		template<typename T>
        operative operator&( T&& other ) const
        {
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->band( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator|( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->bor( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator^( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->bxor( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator+( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->add( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator-( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
                ->mov( tmp, decay( ) )
                ->sub( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator>>( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->bshl( tmp, other );
			return { tmp, block };
		}

		template<typename T>
		operative operator<<( T&& other ) const
		{
			fassert( is_valid( ) );
			auto tmp = block->tmp( size( ) );
			block
				->mov( tmp, decay( ) )
                ->bshr( tmp, other );
			return { tmp, block };
		}

		// Compound assignment operators
		//
		template<typename T>
		operative& operator+=( T&& other )
		{
			fassert( is_valid( ) );
			block->add( decay( ), other );
			return *this;
		}

		template<typename T>
		operative& operator-=( T&& other )
		{
			fassert( is_valid( ) );
			block->sub( decay( ), other );
			return *this;
		}
    };

    // Helpers to provide operation functions for operatives / operands
    //
    template <typename T, std::enable_if_t<std::is_base_of_v<operand, std::remove_cvref_t<T>>, int> = 0>
    bitcnt_t op_size( T value )
    {
        return value.size() * 8;
    }

    template <typename T, std::enable_if_t<std::is_same_v<operative, std::remove_cvref_t<T>>, int> = 0>
    T popcnt( T value )
    {
        return value.popcnt();
    }
}