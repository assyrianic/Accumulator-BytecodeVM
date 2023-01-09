#include "vmachine.h"

//#define VMACHINE_DBG

#ifdef VMACHINE_DBG
static FILE *vm_output;
static void prep_vm_output() {
	vm_output = fopen("vmachine_output.txt", "w+");
}
static void close_vm_output() {
	fclose(vm_output);
	vm_output = NULL;
}
#	define VMACHINE_OPCODE    fprintf(vm_output, "%s -> rA: '%u'\n", __func__, rA.uint32);
#else
#	define VMACHINE_OPCODE 
#endif

void vmachine_run(
	const size_t          stksize,
	uint8_t               stkmem[static stksize],
	const uint32_t        ip[static 1],
	struct VMachineState *state
) {
#ifdef VMACHINE_DBG
	prep_vm_output();
#endif
	uintptr_t sp = ( uintptr_t )(&stkmem[stksize - sizeof(union VMachineVal)]);
	uintptr_t fp = 0, lr = 0;
    union VMachinePtr pc = { .uint32 = &ip[0] };
    union VMachineVal rA = {0};
	VMACHINE_OPCODE
	vmachine_exec(INSTR_ARGS);
#ifdef VMACHINE_DBG
	close_vm_output();
#endif
}

void vmachine_exec(INSTR_PARAMS) {
	VMACHINE_OPCODE
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func_halt(INSTR_PARAMS) {
	VMACHINE_OPCODE
	state->saved_ra = rA;
}
void func_nop(INSTR_PARAMS) {
	VMACHINE_OPCODE
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_imm(INSTR_PARAMS) {
	VMACHINE_OPCODE
    rA = *pc.val++;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_lea(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	rA.uintptr = fp + offset;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// arithmetic ops.
void func_add(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.int64 += rbp[i].int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_vadd(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
	const uint_fast8_t vec_len = state->vec_len;
	if( state->elem_len==sizeof(int8_t) ) {
		int8_t *const restrict r1 = ( int8_t* )( rA.uintptr );
		const int8_t *const r2 = ( const int8_t* )(rbp + i);
		for( size_t i=0; i < vec_len; i++ ) {
			r1[i] += r2[i];
		}
	} else if( state->elem_len==sizeof(int16_t) ) {
		int16_t *const restrict r1 = ( int16_t* )( rA.uintptr );
		const int16_t *const r2 = ( const int16_t* )(rbp + i);
		for( size_t i=0; i < vec_len; i++ ) {
			r1[i] += r2[i];
		}
	} else if( state->elem_len==sizeof(int32_t) ) {
		int32_t *const restrict r1 = ( int32_t* )( rA.uintptr );
		const int32_t *const r2 = ( const int32_t* )(rbp + i);
		for( size_t i=0; i < vec_len; i++ ) {
			r1[i] += r2[i];
		}
	} else {
		int64_t *const restrict r1 = ( int64_t* )( rA.uintptr );
		const int64_t *const r2 = ( const int64_t* )(rbp + i);
		for( size_t i=0; i < vec_len; i++ ) {
			r1[i] += r2[i];
		}
	}
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sub(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.int64 -= rbp[i].int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_mul(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.int64 *= rbp[i].int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_divi(INSTR_PARAMS) {
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 /= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_mod(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 %= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__and(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 &= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__or(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 |= rbp[i].int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__xor(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 ^= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sll(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 <<= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_srl(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 >>= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sra(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.int64 >>= rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_slt(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 = rA.int64 < rbp[i].int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sltu(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 = rA.uint64 < rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_cmp(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    rA.uint64 = rA.uint64==rbp[i].uint64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// Load & Stores
void func_ld1(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const int8_t *const restrict ptr = ( const int8_t* )(fp + offset);
	rA.int64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ld2(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const int16_t *const restrict ptr = ( const int16_t* )(fp + offset);
	rA.int64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ld4(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const int32_t *const restrict ptr = ( const int32_t* )(fp + offset);
	rA.int64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ld8(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const union VMachineVal *const restrict ptr = ( const union VMachineVal* )(fp + offset);
	rA = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ldu1(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const uint8_t *const restrict ptr = ( const uint8_t* )(fp + offset);
	rA.uint64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ldu2(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const uint16_t *const restrict ptr = ( const uint16_t* )(fp + offset);
	rA.uint64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ldu4(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	const uint32_t *const restrict ptr = ( const uint32_t* )(fp + offset);
	rA.uint64 = *ptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func_st1(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	uint8_t *const restrict ptr = ( uint8_t* )(fp + offset);
	*ptr = rA.uint8;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_st2(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	uint16_t *const restrict ptr = ( uint16_t* )(fp + offset);
	*ptr = rA.uint16;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_st4(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	uint32_t *const restrict ptr = ( uint32_t* )(fp + offset);
	*ptr = rA.uint32;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_st8(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
	union VMachineVal *const restrict ptr = ( union VMachineVal* )(fp + offset);
	*ptr = rA;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// Jumps
void func_jmp(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
    pc.uint32 += offset;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_jnz(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
    if( rA.uint64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jz(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
    if( !rA.uint64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jeq(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const int_fast32_t offset = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    if( rA.uint64==rbp[i].uint64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jlt(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const int_fast32_t offset = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    if( rA.int64 < rbp[i].int64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jltu(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const int_fast32_t offset = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    if( rA.uint64 < rbp[i].uint64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jle(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const int_fast32_t offset = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    if( rA.int64 <= rbp[i].int64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}
void func_jleu(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    const int_fast32_t offset = *pc.int32++;
    const union VMachineVal *const restrict rbp = ( const union VMachineVal* )(fp);
    if( rA.uint64 <= rbp[i].uint64 ) {
        pc.uint32 += offset;
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    } else {
        ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
    }
}

/// float<-->int converters.
void func_f32TOf64(INSTR_PARAMS) {
	VMACHINE_OPCODE
    rA.float64 = rA.float32;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_f64TOf32(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.float32 = rA.float64;
	rA.uint32a[1] = 0;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_iTOf64(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.float64 = rA.int64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_iTOf32(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.float32 = rA.int64;
	rA.uint32a[1] = 0;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_f64TOi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.int64 = rA.float64;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_f32TOi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.int64 = rA.float32;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// function call ops
void func_call(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t offset = *pc.int32++;
    lr = ( uintptr_t )(pc.uint8);
	pc.uint32 += offset;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_callr(INSTR_PARAMS) {
	VMACHINE_OPCODE
    lr = ( uintptr_t )(pc.uint8);
	pc.uint8 = ( const uint8_t* )(rA.uintptr);
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ret(INSTR_PARAMS) {
	VMACHINE_OPCODE
	pc.uint8 = ( const uint8_t* )(lr);
	if( pc.uint8==NULL ) {
		func_halt(INSTR_ARGS);
	} else {
  		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_enter(INSTR_PARAMS) {
	VMACHINE_OPCODE
    const int_fast32_t i = *pc.int32++;
    sp -= sizeof fp;
    union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
    rsp->uintptr = fp; /// push bp
    fp = sp; /// mov bp, sp
	sp -= i * sizeof rA;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_leave(INSTR_PARAMS) {
	VMACHINE_OPCODE
    sp = fp; /// mov sp, bp
    const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
    sp += sizeof fp;
    fp = rsp->uintptr; /// pop bp
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_zero(INSTR_PARAMS) {
	VMACHINE_OPCODE
	rA.uint64 = 0;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_pushlr(INSTR_PARAMS) {
	VMACHINE_OPCODE
	sp -= sizeof rA;
    union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
    rsp->uintptr = lr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_poplr(INSTR_PARAMS) {
	VMACHINE_OPCODE
    union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	sp += sizeof rA;
    lr = rsp->uintptr;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_inc(INSTR_PARAMS) {
	VMACHINE_OPCODE
    rA.uint64++;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_dec(INSTR_PARAMS) {
	VMACHINE_OPCODE
    rA.uint64--;
    ( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
