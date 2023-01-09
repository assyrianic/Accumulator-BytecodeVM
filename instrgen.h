#ifndef VMACHINE_INSTR_GEN
#	define VMACHINE_INSTR_GEN

#include "vmachine.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline size_t vmachine_instr_gen(
	uint32_t     instrbuf[const restrict static 1],
	enum VMachineInstrSet op,
	...
) {
	if( op >= MaxOps || op < halt ) {
		return 0;
	}
	
	size_t cells = sizeof(InstrFunc*) / sizeof *instrbuf;
	if( instrbuf != NULL ) {
		union VMachineVal faddr = { .uintptr = ( uintptr_t )(opcode_func[op]) };
		instrbuf[0] = faddr.uint32a[0];
		instrbuf[1] = faddr.uint32a[1];
	}
	
	va_list ap; va_start(ap, op);
	switch( op ) {
		/// 0 operands:
		case halt: case nop: case ret: case callr:
		case f32TOf64: case f64TOf32: case iTOf64: case iTOf32: case f64TOi: case f32TOi:
		case leave: case zero: case pushlr: case poplr:
		case inc: case dec:
			break;

		/// single 4-byte operand.
		case enter: case add: case sub: case mul: case divi: case mod:
		case _and: case _or: case _xor: case sll: case srl: case sra:
		case slt: case sltu: case cmp:
		case ld1: case ld2: case ld4: case ld8: case ldu1: case ldu2: case ldu4:
		case st1: case st2: case st4: case st8:
		case jmp: case jnz: case jz: case call:
		case lea:
		{
			const uint32_t oper1 = va_arg(ap, uint32_t);
			if( instrbuf != NULL ) {
				instrbuf[2] = oper1;
			}
			cells += sizeof oper1 / sizeof *instrbuf;
			break;
		}
		
		/// single 8-byte operand
		case imm:
		{
			const union VMachineVal oper1 = va_arg(ap, union VMachineVal);
			if( instrbuf != NULL ) {
				instrbuf[2] = oper1.uint32a[0];
				instrbuf[3] = oper1.uint32a[1];
			}
			cells += sizeof oper1 / sizeof *instrbuf;
			break;
		}
		
		/// two 4-byte operands.
		case jeq: case jlt: case jltu: case jle: case jleu:
		{
			const uint32_t oper1 = va_arg(ap, uint32_t);
			const uint32_t oper2 = va_arg(ap, uint32_t);
			if( instrbuf != NULL ) {
				instrbuf[2] = oper1;
				instrbuf[3] = oper2;
			}
			cells += (sizeof oper1 + sizeof oper2) / sizeof *instrbuf;
			break;
		}
	}
	return cells;
}

#ifdef __cplusplus
}
#endif

#endif /** VMACHINE_INSTR_GEN */