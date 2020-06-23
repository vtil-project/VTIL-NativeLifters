#include "emulator.hpp"
#include <vtil/io>

// Stack delta if this were to be used as stack.
// - (Thanks ICC)
//
#ifdef _MSC_VER
extern "C" void emulate_and_return( emulator * _this );
#endif

// Invokes routine at the pointer given with the current context and updates the context.
// - Template argument is a small trick to make it work with ICC, declaring a constexpr within the scope does not work.
//
void emulator::invoke( const void* routine_pointer )
{
    // Set the runtime RIP.
    //
    __rip = routine_pointer;

#ifndef _MSC_VER
    _asm
    {
        // Replace the current stack pointer with a this pointer, save the previous stack pointer.
        //
        mov     rax,	rsp
        mov     rsp,	this
        add     rsp,	0x20
		mov     [rsp+0x188], rax

		lea rax, qword ptr [rsp+0x178]

		push    qword ptr [rax]
		pushfq
		pop     qword ptr [rax]
		popfq

		xchg    rax, [rsp+0x100]
		xchg    rbx, [rsp+0x108]
		xchg    rcx, [rsp+0x110]
		xchg    rdx, [rsp+0x118]
		xchg    rsi, [rsp+0x120]
		xchg    rdi, [rsp+0x128]
		xchg    rbp, [rsp+0x130]
		xchg    r8,  [rsp+0x138]
		xchg    r9,  [rsp+0x140]
		xchg    r10, [rsp+0x148]
		xchg    r11, [rsp+0x150]
		xchg    r12, [rsp+0x158]
		xchg    r13, [rsp+0x160]
		xchg    r14, [rsp+0x168]
		xchg    r15, [rsp+0x170]

		call    qword ptr [rsp+0x180]
	
		xchg    rax, [rsp+0x100]
		xchg    rbx, [rsp+0x108]
		xchg    rcx, [rsp+0x110]
		xchg    rdx, [rsp+0x118]
		xchg    rsi, [rsp+0x120]
		xchg    rdi, [rsp+0x128]
		xchg    rbp, [rsp+0x130]
		xchg    r8,  [rsp+0x138]
		xchg    r9,  [rsp+0x140]
		xchg    r10, [rsp+0x148]
		xchg    r11, [rsp+0x150]
		xchg    r12, [rsp+0x158]
		xchg    r13, [rsp+0x160]
		xchg    r14, [rsp+0x168]
		xchg    r15, [rsp+0x170]

		lea rax, qword ptr [rsp+0x178]

		push    qword ptr [rax]
		pushfq
		pop     qword ptr [rax]
		popfq


		mov     rsp, [rsp+0x188]
    }
#else
    emulate_and_return( this );
#endif
}

// Resolves the offset<0> where the value is saved at for the given register
// and the number of bytes<1> it takes.
//
std::pair<int32_t, uint8_t> emulator::resolve( x86_reg reg ) const
{
    auto [base_reg, offset, size] = vtil::amd64::resolve_mapping( reg );

    const void* base;
    switch ( base_reg )
    {
        case X86_REG_RAX:	base = &v_rax;					break;
        case X86_REG_RBP:	base = &v_rbp;					break;
        case X86_REG_RBX:	base = &v_rbx;					break;
        case X86_REG_RCX:	base = &v_rcx;					break;
        case X86_REG_RDI:	base = &v_rdi;					break;
        case X86_REG_RDX:	base = &v_rdx;					break;
        case X86_REG_RSI:	base = &v_rsi;					break;
        case X86_REG_R8: 	base = &v_r8;					break;
        case X86_REG_R9: 	base = &v_r9;					break;
        case X86_REG_R10:	base = &v_r10;					break;
        case X86_REG_R11:	base = &v_r11;					break;
        case X86_REG_R12:	base = &v_r12;					break;
        case X86_REG_R13:	base = &v_r13;					break;
        case X86_REG_R14:	base = &v_r14;					break;
        case X86_REG_R15:	base = &v_r15;					break;
        default:            unreachable();
    }

    return { ( ( uint8_t* ) base - ( uint8_t* ) this ) + offset, size };
}

// Sets the value of a register.
//
emulator& emulator::set( x86_reg reg, uint64_t value )
{
    auto [off, sz] = resolve( reg );
    memcpy( ( uint8_t* ) this + off, &value, sz );
    return *this;
}

// Gets the value of a register.
//
uint64_t emulator::get( x86_reg reg ) const
{
    uint64_t value = 0;
    auto [off, sz] = resolve( reg );
    memcpy( &value, ( uint8_t* ) this + off, sz );
    return value;
}