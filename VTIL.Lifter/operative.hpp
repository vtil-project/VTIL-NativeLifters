#pragma once
#include <optional>
#include <vtil/arch>
#include "operations.hpp"

namespace vtil::lifter
{
    // Provides a simple wrapper over operands to support instruction emittion via operators
    //
    class operative : operand
    {
    private:
        // Backing target basic block for code generation
        //
        basic_block* block = {};

    public:
        // Determines whether the instance is valid for operations
        //
        bool is_valid()
        {
            return block && is_register();
        }

        // Clones the current instance by copying its contents to a temporary register and
        // returning said register
        //
        operative clone()
        {
            fassert( is_valid() );

            // Create and populate a temp register to hold the current register's value
            // 
            auto tmp = block->tmp( size() );
            block->mov( tmp, this );

            // Create & return a new operative using the temp register
            //
            return { tmp, block };
        }

        // Executes the popcnt instruction on the current operative
        //
        operative& popcnt()
        {
            fassert( is_valid() );

            block->popcnt( this );

            return *this;
        }

        // Constructs the operative from a register and basic block
        // Optionally clones the operative on creation
        //
        template <typename T, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<T>, operand>, int> = 0>
        operative( T desc, basic_block* block, bool autoclone = false )
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

        // Assignment operators
        //
        template <typename T>
        operative& operator=( T other )
        {
            fassert( is_valid() );
            block->mov( this, other );
            return *this;
        }

        // Equality operators
        //
        template <typename T>
        operative& operator==( T other )
        {
            fassert( is_valid() );
            block->te( this, this, other );
            return *this;
        }

        template <typename T>
        operative& operator!=( T other )
        {
            fassert( is_valid() );
            block->tne( this, this, other );
            return *this;
        }

        template <typename T>
        operative& operator>=( T other )
        {
            fassert( is_valid() );
            block->tge( this, this, other );
            return *this;
        }

        template <typename T>
        operative& operator>( T other )
        {
            fassert( is_valid() );
            block->tg( this, this, other );
            return *this;
        }

        template <typename T>
        operative& operator<=( T other )
        {
            fassert( is_valid() );
            block->tle( this, this, other );
            return *this;
        }

        template <typename T>
        operative& operator<( T other )
        {
            fassert( is_valid() );
            block->tl( this, this, other );
            return *this;
        }

        // 2 sided arithmetic operators
        //
        template <typename T>
        operative& operator+( T other )
        {
            fassert( is_valid() );
            block->add( this, other );
            return *this;
        }

        template <typename T>
        operative& operator-( T other )
        {
            fassert( is_valid() );
            block->sub( this, other );
            return *this;
        }

        template <typename T>
        operative& operator^( T other )
        {
            fassert( is_valid() );
            block->bxor( this, other );
            return *this;
        }

        template <typename T>
        operative& operator|( T other )
        {
            fassert( is_valid() );
            block->bor( this, other );
            return *this;
        }

        template <typename T>
        operative& operator&( T other )
        {
            fassert( is_valid() );
            block->band( this, other );
            return *this;
        }

        template <typename T>
        operative& operator<<( T other )
        {
            fassert( is_valid() );
            block->bshl( this, other );
            return *this;
        }

        template <typename T>
        operative& operator>>( T other )
        {
            fassert( is_valid() );
            block->bshr( this, other );
            return *this;
        }

        // Single-sided arithmetic operators
        //
        template <typename T>
        operative& operator!()
        {
            fassert( is_valid() );

            // Logical NOT: (!A) == (A == 0)
            // 
            block->te( this, this, 0 );
            return *this;
        }

        template <typename T>
        operative& operator-()
        {
            fassert( is_valid() );
            block->neg( this );
            return *this;
        }

        template <typename T>
        operative& operator~()
        {
            fassert( is_valid() );
            block->bnot( this );
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