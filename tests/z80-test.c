//------------------------------------------------------------------------------
//  z80-test.c
//------------------------------------------------------------------------------
#ifndef _DEBUG
#define _DEBUG
#endif
#define CHIPS_IMPL
#include "chips/z80.h"
#include <stdio.h>

z80 cpu;
uint8_t mem[1<<16] = { 0 };

void tick(z80* cpu) {
    if (cpu->CTRL & Z80_MREQ) {
        if (cpu->CTRL & Z80_RD) {
            /* memory read */
            cpu->DATA = mem[cpu->ADDR];
        }
        else if (cpu->CTRL & Z80_WR) {
            /* memory write */
            mem[cpu->ADDR] = cpu->DATA;
        }
    }
    else if (cpu->CTRL & Z80_IORQ) {
        if (cpu->CTRL & Z80_RD) {
            /* IN */
        }
        else if (cpu->CTRL & Z80_WR) {
            /* OUT */
        }
    }
}

void init() {
    z80_desc desc = {
        .tick_func = tick,
        .tick_context = 0
    };
    z80_init(&cpu, &desc);
}

void copy(uint16_t addr, uint8_t* bytes, size_t num) {
    assert((addr + num) <= sizeof(mem));
    memcpy(&mem[addr], bytes, num);
}

uint32_t step() {
    return z80_step(&cpu);
}

bool flags(uint8_t expected) {
    /* don't check undocumented flags */
    return (cpu.F & ~(Z80_YF|Z80_XF)) == expected;
}

uint32_t num_tests = 0;
#define T(x) { assert(x); num_tests++; }

/* LD A,R; LD A,I */
void LD_A_RI() {
    puts(">>> LD A,R; LD A,I");
    uint8_t prog[] = {
        0xED, 0x57,         // LD A,I
        0x97,               // SUB A
        0xED, 0x5F,         // LD A,R
    };
    copy(0x0000, prog, sizeof(prog));
    init();
    cpu.iff1 = true;
    cpu.iff2 = true;
    cpu.R = 0x34;
    cpu.I = 0x01;
    cpu.F = Z80_CF;
    T(9 == step()); T(0x01 == cpu.A); T(flags(Z80_PF|Z80_CF));
    T(4 == step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(9 == step()); T(0x39 == cpu.A); T(flags(Z80_PF));
}

/* LD I,A; LD R,A */
void LD_IR_A() {
    puts(">>> LD I,A; LD R,A");
    uint8_t prog[] = {
        0x3E, 0x45,     // LD A,0x45
        0xED, 0x47,     // LD I,A
        0xED, 0x4F,     // LD R,A
    };
    copy(0x0000, prog, sizeof(prog));
    init();
    T(7==step()); T(0x45 == cpu.A);
    T(9==step()); T(0x45 == cpu.I);
    T(9==step()); T(0x45 == cpu.R);
}

/* LD r,s; LD r,n */
void LD_r_sn() {
    puts(">>> LD r,s; LD r,n");
    uint8_t prog[] = {
        0x3E, 0x12,     // LD A,0x12
        0x47,           // LD B,A
        0x4F,           // LD C,A
        0x57,           // LD D,A
        0x5F,           // LD E,A
        0x67,           // LD H,A
        0x6F,           // LD L,A
        0x7F,           // LD A,A

        0x06, 0x13,     // LD B,0x13
        0x40,           // LD B,B
        0x48,           // LD C,B
        0x50,           // LD D,B
        0x58,           // LD E,B
        0x60,           // LD H,B
        0x68,           // LD L,B
        0x78,           // LD A,B

        0x0E, 0x14,     // LD C,0x14
        0x41,           // LD B,C
        0x49,           // LD C,C
        0x51,           // LD D,C
        0x59,           // LD E,C
        0x61,           // LD H,C
        0x69,           // LD L,C
        0x79,           // LD A,C

        0x16, 0x15,     // LD D,0x15
        0x42,           // LD B,D
        0x4A,           // LD C,D
        0x52,           // LD D,D
        0x5A,           // LD E,D
        0x62,           // LD H,D
        0x6A,           // LD L,D
        0x7A,           // LD A,D

        0x1E, 0x16,     // LD E,0x16
        0x43,           // LD B,E
        0x4B,           // LD C,E
        0x53,           // LD D,E
        0x5B,           // LD E,E
        0x63,           // LD H,E
        0x6B,           // LD L,E
        0x7B,           // LD A,E

        0x26, 0x17,     // LD H,0x17
        0x44,           // LD B,H
        0x4C,           // LD C,H
        0x54,           // LD D,H
        0x5C,           // LD E,H
        0x64,           // LD H,H
        0x6C,           // LD L,H
        0x7C,           // LD A,H

        0x2E, 0x18,     // LD L,0x18
        0x45,           // LD B,L
        0x4D,           // LD C,L
        0x55,           // LD D,L
        0x5D,           // LD E,L
        0x65,           // LD H,L
        0x6D,           // LD L,L
        0x7D,           // LD A,L
    };
    init();
    copy(0x0000, prog, sizeof(prog));

    T(7==step()); T(0x12==cpu.A);
    T(4==step()); T(0x12==cpu.B);
    T(4==step()); T(0x12==cpu.C);
    T(4==step()); T(0x12==cpu.D);
    T(4==step()); T(0x12==cpu.E);
    T(4==step()); T(0x12==cpu.H);
    T(4==step()); T(0x12==cpu.L);
    T(4==step()); T(0x12==cpu.A);
    T(7==step()); T(0x13==cpu.B);
    T(4==step()); T(0x13==cpu.B);
    T(4==step()); T(0x13==cpu.C);
    T(4==step()); T(0x13==cpu.D);
    T(4==step()); T(0x13==cpu.E);
    T(4==step()); T(0x13==cpu.H);
    T(4==step()); T(0x13==cpu.L);
    T(4==step()); T(0x13==cpu.A);
    T(7==step()); T(0x14==cpu.C);
    T(4==step()); T(0x14==cpu.B);
    T(4==step()); T(0x14==cpu.C);
    T(4==step()); T(0x14==cpu.D);
    T(4==step()); T(0x14==cpu.E);
    T(4==step()); T(0x14==cpu.H);
    T(4==step()); T(0x14==cpu.L);
    T(4==step()); T(0x14==cpu.A);
    T(7==step()); T(0x15==cpu.D);
    T(4==step()); T(0x15==cpu.B);
    T(4==step()); T(0x15==cpu.C);
    T(4==step()); T(0x15==cpu.D);
    T(4==step()); T(0x15==cpu.E);
    T(4==step()); T(0x15==cpu.H);
    T(4==step()); T(0x15==cpu.L);
    T(4==step()); T(0x15==cpu.A);
    T(7==step()); T(0x16==cpu.E);
    T(4==step()); T(0x16==cpu.B);
    T(4==step()); T(0x16==cpu.C);
    T(4==step()); T(0x16==cpu.D);
    T(4==step()); T(0x16==cpu.E);
    T(4==step()); T(0x16==cpu.H);
    T(4==step()); T(0x16==cpu.L);
    T(4==step()); T(0x16==cpu.A);
    T(7==step()); T(0x17==cpu.H);
    T(4==step()); T(0x17==cpu.B);
    T(4==step()); T(0x17==cpu.C);
    T(4==step()); T(0x17==cpu.D);
    T(4==step()); T(0x17==cpu.E);
    T(4==step()); T(0x17==cpu.H);
    T(4==step()); T(0x17==cpu.L);
    T(4==step()); T(0x17==cpu.A);
    T(7==step()); T(0x18==cpu.L);
    T(4==step()); T(0x18==cpu.B);
    T(4==step()); T(0x18==cpu.C);
    T(4==step()); T(0x18==cpu.D);
    T(4==step()); T(0x18==cpu.E);
    T(4==step()); T(0x18==cpu.H);
    T(4==step()); T(0x18==cpu.L);
    T(4==step()); T(0x18==cpu.A);
}

/* LD r,(HL) */
void LD_r_iHLi() {
    puts(">>> LD r,(HL)");
    uint8_t prog[] = {
        0x21, 0x00, 0x10,   // LD HL,0x1000
        0x3E, 0x33,         // LD A,0x33
        0x77,               // LD (HL),A
        0x3E, 0x22,         // LD A,0x22
        0x46,               // LD B,(HL)
        0x4E,               // LD C,(HL)
        0x56,               // LD D,(HL)
        0x5E,               // LD E,(HL)
        0x66,               // LD H,(HL)
        0x26, 0x10,         // LD H,0x10
        0x6E,               // LD L,(HL)
        0x2E, 0x00,         // LD L,0x00
        0x7E,               // LD A,(HL)
    };
    init();
    copy(0x0000, prog, sizeof(prog));

    T(10==step()); T(0x1000 == cpu.HL);
    T(7==step()); T(0x33 == cpu.A);
    T(7==step()); T(0x33 == mem[0x1000]);
    T(7==step()); T(0x22 == cpu.A);
    T(7==step()); T(0x33 == cpu.B);
    T(7==step()); T(0x33 == cpu.C);
    T(7==step()); T(0x33 == cpu.D);
    T(7==step()); T(0x33 == cpu.E);
    T(7==step()); T(0x33 == cpu.H);
    T(7==step()); T(0x10 == cpu.H);
    T(7==step()); T(0x33 == cpu.L);
    T(7==step()); T(0x00 == cpu.L);
    T(7==step()); T(0x33 == cpu.A);       
}

/* LD r,(IX|IY+d) */
void LD_r_iIXIYi() {
    puts("FIXME FIXME FIXME >>> LD r,(IX+d); LD r,(IY+d)");
}

/* LD (HL),r */
void LD_iHLi_r() {
    puts(">>> LD (HL),r");
    uint8_t prog[] = {
        0x21, 0x00, 0x10,   // LD HL,0x1000
        0x3E, 0x12,         // LD A,0x12
        0x77,               // LD (HL),A
        0x06, 0x13,         // LD B,0x13
        0x70,               // LD (HL),B
        0x0E, 0x14,         // LD C,0x14
        0x71,               // LD (HL),C
        0x16, 0x15,         // LD D,0x15
        0x72,               // LD (HL),D
        0x1E, 0x16,         // LD E,0x16
        0x73,               // LD (HL),E
        0x74,               // LD (HL),H
        0x75,               // LD (HL),L
    };
    init();
    copy(0x0000, prog, sizeof(prog));

    T(10==step()); T(0x1000 == cpu.HL);
    T(7==step()); T(0x12 == cpu.A);
    T(7==step()); T(0x12 == mem[0x1000]);
    T(7==step()); T(0x13 == cpu.B);
    T(7==step()); T(0x13 == mem[0x1000]);
    T(7==step()); T(0x14 == cpu.C);
    T(7==step()); T(0x14 == mem[0x1000]);
    T(7==step()); T(0x15 == cpu.D);
    T(7==step()); T(0x15 == mem[0x1000]);
    T(7==step()); T(0x16 == cpu.E);
    T(7==step()); T(0x16 == mem[0x1000]);
    T(7==step()); T(0x10 == mem[0x1000]);
    T(7==step()); T(0x00 == mem[0x1000]);
}

/* LD (IX|IY+d),r */
void LD_iIXIYi_r() {
    puts("FIXME FIXME FIXME >>> LD (IX+d),r; LD (IY+d)");
}

/* LD (HL),n */
void LD_iHLi_n() {
    puts(">>> LD (HL),n");
    uint8_t prog[] = {
        0x21, 0x00, 0x20,   // LD HL,0x2000
        0x36, 0x33,         // LD (HL),0x33
        0x21, 0x00, 0x10,   // LD HL,0x1000
        0x36, 0x65,         // LD (HL),0x65
    };
    init();
    copy(0x0000, prog, sizeof(prog));

    T(10==step()); T(0x2000 == cpu.HL);
    T(10==step()); T(0x33 == mem[0x2000]);
    T(10==step()); T(0x1000 == cpu.HL);
    T(10==step()); T(0x65 == mem[0x1000]);
}

/* LD (IX|IY+d),n */
void LD_iIXIYi_n() {
    puts("FIXME FIXME FIXME >>> LD (IX+d),n; LD (IY+d),n");
    /* FIXME */
}

/* LD A,(BC); LD A,(DE); LD A,(nn) */
void LD_A_iBCDEnni() {
    puts(">>> LD A,(BC); LD A,(DE); LD A,(nn)");
    uint8_t data[] = {
        0x11, 0x22, 0x33
    };
    copy(0x1000, data, sizeof(data));
    uint8_t prog[] = {
        0x01, 0x00, 0x10,   // LD BC,0x1000
        0x11, 0x01, 0x10,   // LD DE,0x1001
        0x0A,               // LD A,(BC)
        0x1A,               // LD A,(DE)
        0x3A, 0x02, 0x10,   // LD A,(0x1002)
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(10==step()); T(0x1000 == cpu.BC);
    T(10==step()); T(0x1001 == cpu.DE);
    T(7 ==step()); T(0x11 == cpu.A);
    T(7 ==step()); T(0x22 == cpu.A);
    T(13==step()); T(0x33 == cpu.A);
}

/* LD (BC),A; LD (DE),A; LD (nn),A */
void LD_iBCDEnni_A() {
    puts(">>> LD (BC),A; LD (DE),A; LD (nn),A");
    uint8_t prog[] = {
        0x01, 0x00, 0x10,   // LD BC,0x1000
        0x11, 0x01, 0x10,   // LD DE,0x1001
        0x3E, 0x77,         // LD A,0x77
        0x02,               // LD (BC),A
        0x12,               // LD (DE),A
        0x32, 0x02, 0x10,   // LD (0x1002),A
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(10==step()); T(0x1000 == cpu.BC);
    T(10==step()); T(0x1001 == cpu.DE);
    T(7 ==step()); T(0x77 == cpu.A);
    T(7 ==step()); T(0x77 == mem[0x1000]);
    T(7 ==step()); T(0x77 == mem[0x1001]);
    T(13==step()); T(0x77 == mem[0x1002]);
}

/* LD dd,nn; LD IX,nn; LD IY,nn */
void LD_ddIXIY_nn() {
    puts("FIXME FIXME FIXME >>> LD dd,nn; LD IX,nn; LD IY,nn");
}

/* LD HL,(nn); LD dd,(nn); LD IX,(nn); LD IY,(nn) */
void LD_HLddIXIY_inni() {
    puts("FIXME FIXME FIXME >>> LD dd,(nn); LD IX,(nn); LD IY,(nn)");
}

/* LD (nn),HL; LD (nn),dd; LD (nn),IX; LD (nn),IY */
void LD_inni_HLddIXIY() {
    puts("FIXME FIXME FIXME >>> LD (nn),dd; LD (nn),IX; LD (nn),IY");
}

/* LD SP,HL; LD SP,IX; LD SP,IY */
void LD_SP_HLIXIY() {
    puts("FIXME FIXME FIXME >>> LD SP,HL; LD SP,IX; LD SP,IY");
}

/* PUSH qq; PUSH IX; PUSH IY; POP qq; POP IX; POP IY */
void PUSH_POP_qqIXIY() {
    puts("FIXME FIXME FIXME >>> PUSH qq; PUSH IX; PUSH IY; POP qq; POP IX; POP IY");
}

/* EX DE,HL; EX AF,AF'; EX (SP),HL; EX (SP),IX; EX (SP),IY */
void EX() {
    puts("FIXME FIXME FIXME >>> EX DE,HL; EX AF,AF'; EX (SP),HL; EX (SP),IX; EX (SP),IY");
}

/* ADD A,r; ADD A,n */
void ADD_A_rn() {
    puts(">>> ADD A,r; ADD A,n");
    uint8_t prog[] = {
        0x3E, 0x0F,     // LD A,0x0F
        0x87,           // ADD A,A
        0x06, 0xE0,     // LD B,0xE0
        0x80,           // ADD A,B
        0x3E, 0x81,     // LD A,0x81
        0x0E, 0x80,     // LD C,0x80
        0x81,           // ADD A,C
        0x16, 0xFF,     // LD D,0xFF
        0x82,           // ADD A,D
        0x1E, 0x40,     // LD E,0x40
        0x83,           // ADD A,E
        0x26, 0x80,     // LD H,0x80
        0x84,           // ADD A,H
        0x2E, 0x33,     // LD L,0x33
        0x85,           // ADD A,L
        0xC6, 0x44,     // ADD A,0x44
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(7==step()); T(0x0F == cpu.A); T(flags(0));
    T(4==step()); T(0x1E == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xE0 == cpu.B);
    T(4==step()); T(0xFE == cpu.A); T(flags(Z80_SF));
    T(7==step()); T(0x81 == cpu.A);
    T(7==step()); T(0x80 == cpu.C);
    T(4==step()); T(0x01 == cpu.A); T(flags(Z80_VF|Z80_CF));
    T(7==step()); T(0xFF == cpu.D);
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_HF|Z80_CF));
    T(7==step()); T(0x40 == cpu.E);
    T(4==step()); T(0x40 == cpu.A); T(flags(0));
    T(7==step()); T(0x80 == cpu.H);
    T(4==step()); T(0xC0 == cpu.A); T(flags(Z80_SF));
    T(7==step()); T(0x33 == cpu.L);
    T(4==step()); T(0xF3 == cpu.A); T(flags(Z80_SF));
    T(7==step()); T(0x37 == cpu.A); T(flags(Z80_CF));
}

/* ADD A,(HL); ADD A,(IX+d); ADD A,(IY+d) */
void ADD_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> ADD A,(HL); ADD A,(IX+d); ADD A,(IY+d)");
}

/* ADC A,r; ADC A,n */
void ADC_A_rn() {
    puts(">>> ADC A,r; ADD A,n");
    uint8_t prog[] = {
        0x3E, 0x00,         // LD A,0x00
        0x06, 0x41,         // LD B,0x41
        0x0E, 0x61,         // LD C,0x61
        0x16, 0x81,         // LD D,0x81
        0x1E, 0x41,         // LD E,0x41
        0x26, 0x61,         // LD H,0x61
        0x2E, 0x81,         // LD L,0x81
        0x8F,               // ADC A,A
        0x88,               // ADC A,B
        0x89,               // ADC A,C
        0x8A,               // ADC A,D
        0x8B,               // ADC A,E
        0x8C,               // ADC A,H
        0x8D,               // ADC A,L
        0xCE, 0x01,         // ADC A,0x01
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(7==step()); T(0x00 == cpu.A);
    T(7==step()); T(0x41 == cpu.B);
    T(7==step()); T(0x61 == cpu.C);
    T(7==step()); T(0x81 == cpu.D);
    T(7==step()); T(0x41 == cpu.E);
    T(7==step()); T(0x61 == cpu.H);
    T(7==step()); T(0x81 == cpu.L);
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF));
    T(4==step()); T(0x41 == cpu.A); T(flags(0));
    T(4==step()); T(0xA2 == cpu.A); T(flags(Z80_SF|Z80_VF));
    T(4==step()); T(0x23 == cpu.A); T(flags(Z80_VF|Z80_CF));
    T(4==step()); T(0x65 == cpu.A); T(flags(0));
    T(4==step()); T(0xC6 == cpu.A); T(flags(Z80_SF|Z80_VF));
    T(4==step()); T(0x47 == cpu.A); T(flags(Z80_VF|Z80_CF));
    T(7==step()); T(0x49 == cpu.A); T(flags(0));
}

/* ADC A,(HL); ADC A,(IX+d); ADC A,(IY+d) */
void ADC_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> ADC A,(HL); ADC A,(IX+d); ADC A,(IY+d)");
}

/* SUB A,r; SUB A,n */
void SUB_A_rn() {
    puts(">>> SUB A,r; SUB A,n");
    uint8_t prog[] = {
        0x3E, 0x04,     // LD A,0x04
        0x06, 0x01,     // LD B,0x01
        0x0E, 0xF8,     // LD C,0xF8
        0x16, 0x0F,     // LD D,0x0F
        0x1E, 0x79,     // LD E,0x79
        0x26, 0xC0,     // LD H,0xC0
        0x2E, 0xBF,     // LD L,0xBF
        0x97,           // SUB A,A
        0x90,           // SUB A,B
        0x91,           // SUB A,C
        0x92,           // SUB A,D
        0x93,           // SUB A,E
        0x94,           // SUB A,H
        0x95,           // SUB A,L
        0xD6, 0x01,     // SUB A,0x01
        0xD6, 0xFE,     // SUB A,0xFE
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(7==step()); T(0x04 == cpu.A);
    T(7==step()); T(0x01 == cpu.B);
    T(7==step()); T(0xF8 == cpu.C);
    T(7==step()); T(0x0F == cpu.D);
    T(7==step()); T(0x79 == cpu.E);
    T(7==step()); T(0xC0 == cpu.H);
    T(7==step()); T(0xBF == cpu.L);
    T(4==step()); T(0x0 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(4==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x07 == cpu.A); T(flags(Z80_NF));
    T(4==step()); T(0xF8 == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x7F == cpu.A); T(flags(Z80_HF|Z80_VF|Z80_NF));
    T(4==step()); T(0xBF == cpu.A); T(flags(Z80_SF|Z80_VF|Z80_NF|Z80_CF));
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(7==step()); T(0x01 == cpu.A); T(flags(Z80_NF));
}

/* SUB A,(HL); SUB A,(IX+d); SUB A,(IY+d) */
void SUB_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> SUB A,(HL); SUB A,(IX+d); SUB A,(IY+d)");
}

/* CP A,r; CP A,n */
void CP_A_rn() {
    puts(">>> CP A,r; CP A,n");
    uint8_t prog[] = {
        0x3E, 0x04,     // LD A,0x04
        0x06, 0x05,     // LD B,0x05
        0x0E, 0x03,     // LD C,0x03
        0x16, 0xff,     // LD D,0xff
        0x1E, 0xaa,     // LD E,0xaa
        0x26, 0x80,     // LD H,0x80
        0x2E, 0x7f,     // LD L,0x7f
        0xBF,           // CP A
        0xB8,           // CP B
        0xB9,           // CP C
        0xBA,           // CP D
        0xBB,           // CP E
        0xBC,           // CP H
        0xBD,           // CP L
        0xFE, 0x04,     // CP 0x04
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    T(7==step()); T(0x04 == cpu.A);
    T(7==step()); T(0x05 == cpu.B);
    T(7==step()); T(0x03 == cpu.C);
    T(7==step()); T(0xff == cpu.D);
    T(7==step()); T(0xaa == cpu.E);
    T(7==step()); T(0x80 == cpu.H);
    T(7==step()); T(0x7f == cpu.L);
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_NF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_SF|Z80_VF|Z80_NF|Z80_CF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(7==step()); T(0x04 == cpu.A); T(flags(Z80_ZF|Z80_NF));
}

/* CP A,(HL); CP A,(IX+d); CP A,(IY+d) */
void CP_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> CP A,(HL); CP A,(IX+d); CP A,(IY+d)");
}

/* SBC A,r; SBC A,n */
void SBC_A_rn() {
    puts(">>> SBC A,r; SBC A,n");
    uint8_t prog[] = {
        0x3E, 0x04,     // LD A,0x04
        0x06, 0x01,     // LD B,0x01
        0x0E, 0xF8,     // LD C,0xF8
        0x16, 0x0F,     // LD D,0x0F
        0x1E, 0x79,     // LD E,0x79
        0x26, 0xC0,     // LD H,0xC0
        0x2E, 0xBF,     // LD L,0xBF
        0x97,           // SUB A,A
        0x98,           // SBC A,B
        0x99,           // SBC A,C
        0x9A,           // SBC A,D
        0x9B,           // SBC A,E
        0x9C,           // SBC A,H
        0x9D,           // SBC A,L
        0xDE, 0x01,     // SBC A,0x01
        0xDE, 0xFE,     // SBC A,0xFE
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    for (int i = 0; i < 7; i++) {
        step();
    }
    T(4==step()); T(0x0 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(4==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x06 == cpu.A); T(flags(Z80_NF));
    T(4==step()); T(0xF7 == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x7D == cpu.A); T(flags(Z80_HF|Z80_VF|Z80_NF));
    T(4==step()); T(0xBD == cpu.A); T(flags(Z80_SF|Z80_VF|Z80_NF|Z80_CF));
    T(4==step()); T(0xFD == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(7==step()); T(0xFB == cpu.A); T(flags(Z80_SF|Z80_NF));
    T(7==step()); T(0xFD == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
}

/* SBC A,(HL); SBC A,(IX+d); SBC A,(IY+d) */
void SBC_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> SBC A,(HL); SBC A,(IX+d); SBC A,(IY+d)");
}

/* OR A,r; OR A,n */
void OR_A_rn() {
    puts(">>> OR A,r; OR A,n");
    uint8_t prog[] = {
        0x97,           // SUB A
        0x06, 0x01,     // LD B,0x01
        0x0E, 0x02,     // LD C,0x02
        0x16, 0x04,     // LD D,0x04
        0x1E, 0x08,     // LD E,0x08
        0x26, 0x10,     // LD H,0x10
        0x2E, 0x20,     // LD L,0x20
        0xB7,           // OR A
        0xB0,           // OR B
        0xB1,           // OR C
        0xB2,           // OR D
        0xB3,           // OR E
        0xB4,           // OR H
        0xB5,           // OR L
        0xF6, 0x40,     // OR 0x40
        0xF6, 0x80,     // OR 0x80
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_PF));
    T(4==step()); T(0x01 == cpu.A); T(flags(0));
    T(4==step()); T(0x03 == cpu.A); T(flags(Z80_PF));
    T(4==step()); T(0x07 == cpu.A); T(flags(0));
    T(4==step()); T(0x0F == cpu.A); T(flags(Z80_PF));
    T(4==step()); T(0x1F == cpu.A); T(flags(0));
    T(4==step()); T(0x3F == cpu.A); T(flags(Z80_PF));
    T(7==step()); T(0x7F == cpu.A); T(flags(0));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
}

/* XOR A,r; XOR A,n */
void XOR_A_rn() {
    puts(">>> XOR A,r; XOR A,n");
    uint8_t prog[] = {
        0x97,           // SUB A
        0x06, 0x01,     // LD B,0x01
        0x0E, 0x03,     // LD C,0x03
        0x16, 0x07,     // LD D,0x07
        0x1E, 0x0F,     // LD E,0x0F
        0x26, 0x1F,     // LD H,0x1F
        0x2E, 0x3F,     // LD L,0x3F
        0xAF,           // XOR A
        0xA8,           // XOR B
        0xA9,           // XOR C
        0xAA,           // XOR D
        0xAB,           // XOR E
        0xAC,           // XOR H
        0xAD,           // XOR L
        0xEE, 0x7F,     // XOR 0x7F
        0xEE, 0xFF,     // XOR 0xFF
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_PF));
    T(4==step()); T(0x01 == cpu.A); T(flags(0));
    T(4==step()); T(0x02 == cpu.A); T(flags(0));
    T(4==step()); T(0x05 == cpu.A); T(flags(Z80_PF));
    T(4==step()); T(0x0A == cpu.A); T(flags(Z80_PF));
    T(4==step()); T(0x15 == cpu.A); T(flags(0));
    T(4==step()); T(0x2A == cpu.A); T(flags(0));
    T(7==step()); T(0x55 == cpu.A); T(flags(Z80_PF));
    T(7==step()); T(0xAA == cpu.A); T(flags(Z80_SF|Z80_PF));
}

/* OR A,(HL); OR A,(IX+d); OR A,(IY+d) */
/* XOR A,(HL); XOR A,(IX+d); XOR A,(IY+d) */
void OR_XOR_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> OR/XOR A,(HL); OR/XOR A,(IX+d); OR/XOR A,(IY+d)");
}

/* AND A,r; AND A,n */
void AND_A_rn() {
    puts(">>> AND A,r; AND A,n");
    uint8_t prog[] = {
        0x3E, 0xFF,             // LD A,0xFF
        0x06, 0x01,             // LD B,0x01
        0x0E, 0x03,             // LD C,0x02
        0x16, 0x04,             // LD D,0x04
        0x1E, 0x08,             // LD E,0x08
        0x26, 0x10,             // LD H,0x10
        0x2E, 0x20,             // LD L,0x20
        0xA0,                   // AND B
        0xF6, 0xFF,             // OR 0xFF
        0xA1,                   // AND C
        0xF6, 0xFF,             // OR 0xFF
        0xA2,                   // AND D
        0xF6, 0xFF,             // OR 0xFF
        0xA3,                   // AND E
        0xF6, 0xFF,             // OR 0xFF
        0xA4,                   // AND H
        0xF6, 0xFF,             // OR 0xFF
        0xA5,                   // AND L
        0xF6, 0xFF,             // OR 0xFF
        0xE6, 0x40,             // AND 0x40
        0xF6, 0xFF,             // OR 0xFF
        0xE6, 0xAA,             // AND 0xAA
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(4==step()); T(0x01 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(4==step()); T(0x03 == cpu.A); T(flags(Z80_HF|Z80_PF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(4==step()); T(0x04 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(4==step()); T(0x08 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(4==step()); T(0x10 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(4==step()); T(0x20 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(7==step()); T(0x40 == cpu.A); T(flags(Z80_HF));
    T(7==step()); T(0xFF == cpu.A); T(flags(Z80_SF|Z80_PF));
    T(7==step()); T(0xAA == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_PF));
}

/* AND A,(HL); AND A,(IX+d); AND A,(IY+d) */
void AND_A_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> AND A,(HL); AND A,(IX+d); AND A,(IY+d)");
}

/* INC r; DEC r */
void INC_DEC_r() {
    puts(">>> INC r; DEC r");
    uint8_t prog[] = {
        0x3e, 0x00,         // LD A,0x00
        0x06, 0xFF,         // LD B,0xFF
        0x0e, 0x0F,         // LD C,0x0F
        0x16, 0x0E,         // LD D,0x0E
        0x1E, 0x7F,         // LD E,0x7F
        0x26, 0x3E,         // LD H,0x3E
        0x2E, 0x23,         // LD L,0x23
        0x3C,               // INC A
        0x3D,               // DEC A
        0x04,               // INC B
        0x05,               // DEC B
        0x0C,               // INC C
        0x0D,               // DEC C
        0x14,               // INC D
        0x15,               // DEC D
        0xFE, 0x01,         // CP 0x01  // set carry flag (should be preserved)
        0x1C,               // INC E
        0x1D,               // DEC E
        0x24,               // INC H
        0x25,               // DEC H
        0x2C,               // INC L
        0x2D,               // DEC L
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(4==step()); T(0x01 == cpu.A); T(flags(0));
    T(4==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_NF));
    T(4==step()); T(0x00 == cpu.B); T(flags(Z80_ZF|Z80_HF));
    T(4==step()); T(0xFF == cpu.B); T(flags(Z80_SF|Z80_HF|Z80_NF));
    T(4==step()); T(0x10 == cpu.C); T(flags(Z80_HF));
    T(4==step()); T(0x0F == cpu.C); T(flags(Z80_HF|Z80_NF));
    T(4==step()); T(0x0F == cpu.D); T(flags(0));
    T(4==step()); T(0x0E == cpu.D); T(flags(Z80_NF));
    T(7==step()); T(0x00 == cpu.A); T(flags(Z80_SF|Z80_HF|Z80_NF|Z80_CF));
    T(4==step()); T(0x80 == cpu.E); T(flags(Z80_SF|Z80_HF|Z80_VF|Z80_CF));
    T(4==step()); T(0x7F == cpu.E); T(flags(Z80_HF|Z80_VF|Z80_NF|Z80_CF));
    T(4==step()); T(0x3F == cpu.H); T(flags(Z80_CF));
    T(4==step()); T(0x3E == cpu.H); T(flags(Z80_NF|Z80_CF));
    T(4==step()); T(0x24 == cpu.L); T(flags(Z80_CF));
    T(4==step()); T(0x23 == cpu.L); T(flags(Z80_NF|Z80_CF));
}

/* INC (HL); INC (IX+d); INC (IY+d); DEC (HL); DEC (IX+d); DEC (IY+d) */
void INC_DEC_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> INC/DEC (HL)/(IX+d)/(IY+d)");
}

/* INC ss; INC IX; INC IY; DEC ss; DEC IX; DEC IY */
void INC_DEC_ssIXIY() {
    puts("FIXME FIXME FIXME >>> INC/DEC ss/IX/IY");
}

/* RLCA; RLA; RRCA; RRA */
void RLCA_RLA_RRCA_RRA() {
    puts(">>> RLCA; RLA; RRCA; RRA");
    uint8_t prog[] = {
        0x3E, 0xA0,     // LD A,0xA0
        0x07,           // RLCA
        0x07,           // RLCA
        0x0F,           // RRCA
        0x0F,           // RRCA
        0x17,           // RLA
        0x17,           // RLA
        0x1F,           // RRA
        0x1F,           // RRA
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    cpu.F = 0xff;
    T(7==step()); T(0xA0 == cpu.A);
    T(4==step()); T(0x41 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF|Z80_CF));
    T(4==step()); T(0x82 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF));
    T(4==step()); T(0x41 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF));
    T(4==step()); T(0xA0 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF|Z80_CF));
    T(4==step()); T(0x41 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF|Z80_CF));
    T(4==step()); T(0x83 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF));
    T(4==step()); T(0x41 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF|Z80_CF));
    T(4==step()); T(0xA0 == cpu.A); T(flags(Z80_SF|Z80_ZF|Z80_VF|Z80_CF));
}

/* RLC r; RL r; RRC r; RR r */
void RLC_RL_RRC_RR_r() {
    puts(">>> RLC r; RL r; RRC r; RR r");
    uint8_t prog[] = {
        0x3E, 0x01,     // LD A,0x01
        0x06, 0xFF,     // LD B,0xFF
        0x0E, 0x03,     // LD C,0x03
        0x16, 0xFE,     // LD D,0xFE
        0x1E, 0x11,     // LD E,0x11
        0x26, 0x3F,     // LD H,0x3F
        0x2E, 0x70,     // LD L,0x70

        0xCB, 0x0F,     // RRC A
        0xCB, 0x07,     // RLC A
        0xCB, 0x08,     // RRC B
        0xCB, 0x00,     // RLC B
        0xCB, 0x01,     // RLC C
        0xCB, 0x09,     // RRC C
        0xCB, 0x02,     // RLC D
        0xCB, 0x0A,     // RRC D
        0xCB, 0x0B,     // RRC E
        0xCB, 0x03,     // RLC E
        0xCB, 0x04,     // RLC H
        0xCB, 0x0C,     // RCC H
        0xCB, 0x05,     // RLC L
        0xCB, 0x0D,     // RRC L

        0xCB, 0x1F,     // RR A
        0xCB, 0x17,     // RL A
        0xCB, 0x18,     // RR B
        0xCB, 0x10,     // RL B
        0xCB, 0x11,     // RL C
        0xCB, 0x19,     // RR C
        0xCB, 0x12,     // RL D
        0xCB, 0x1A,     // RR D
        0xCB, 0x1B,     // RR E
        0xCB, 0x13,     // RL E
        0xCB, 0x14,     // RL H
        0xCB, 0x1C,     // RR H
        0xCB, 0x15,     // RL L
        0xCB, 0x1D,     // RR L
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(8==step()); T(0x80 == cpu.A); T(flags(Z80_SF|Z80_CF));
    T(8==step()); T(0x01 == cpu.A); T(flags(Z80_CF));
    T(8==step()); T(0xFF == cpu.B); T(flags(Z80_SF|Z80_PF|Z80_CF));
    T(8==step()); T(0xFF == cpu.B); T(flags(Z80_SF|Z80_PF|Z80_CF));
    T(8==step()); T(0x06 == cpu.C); T(flags(Z80_PF));
    T(8==step()); T(0x03 == cpu.C); T(flags(Z80_PF));
    T(8==step()); T(0xFD == cpu.D); T(flags(Z80_SF|Z80_CF));
    T(8==step()); T(0xFE == cpu.D); T(flags(Z80_SF|Z80_CF));
    T(8==step()); T(0x88 == cpu.E); T(flags(Z80_SF|Z80_PF|Z80_CF));
    T(8==step()); T(0x11 == cpu.E); T(flags(Z80_PF|Z80_CF));
    T(8==step()); T(0x7E == cpu.H); T(flags(Z80_PF));
    T(8==step()); T(0x3F == cpu.H); T(flags(Z80_PF));
    T(8==step()); T(0xE0 == cpu.L); T(flags(Z80_SF));
    T(8==step()); T(0x70 == cpu.L); T(flags(0));
    T(8==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_PF|Z80_CF));
    T(8==step()); T(0x01 == cpu.A); T(flags(0));
    T(8==step()); T(0x7F == cpu.B); T(flags(Z80_CF));
    T(8==step()); T(0xFF == cpu.B); T(flags(Z80_SF|Z80_PF));
    T(8==step()); T(0x06 == cpu.C); T(flags(Z80_PF));
    T(8==step()); T(0x03 == cpu.C); T(flags(Z80_PF));
    T(8==step()); T(0xFC == cpu.D); T(flags(Z80_SF|Z80_PF|Z80_CF));
    T(8==step()); T(0xFE == cpu.D); T(flags(Z80_SF));
    T(8==step()); T(0x08 == cpu.E); T(flags(Z80_CF));
    T(8==step()); T(0x11 == cpu.E); T(flags(Z80_PF));
    T(8==step()); T(0x7E == cpu.H); T(flags(Z80_PF));
    T(8==step()); T(0x3F == cpu.H); T(flags(Z80_PF));
    T(8==step()); T(0xE0 == cpu.L); T(flags(Z80_SF));
    T(8==step()); T(0x70 == cpu.L); T(flags(0));
}

/* RRC/RLC/RR/RL (HL)/(IX+d)/(IY+d) */
void RRC_RLC_RR_RL_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> RLC/RL/RRC/RR (HL)/(IX+d)/(IY+d)");
}

/* SLA r */
void SLA_r() {
    puts(">>> SLA r");
    uint8_t prog[] = {
        0x3E, 0x01,         // LD A,0x01
        0x06, 0x80,         // LD B,0x80
        0x0E, 0xAA,         // LD C,0xAA
        0x16, 0xFE,         // LD D,0xFE
        0x1E, 0x7F,         // LD E,0x7F
        0x26, 0x11,         // LD H,0x11
        0x2E, 0x00,         // LD L,0x00
        0xCB, 0x27,         // SLA A
        0xCB, 0x20,         // SLA B
        0xCB, 0x21,         // SLA C
        0xCB, 0x22,         // SLA D
        0xCB, 0x23,         // SLA E
        0xCB, 0x24,         // SLA H
        0xCB, 0x25,         // SLA L
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(8==step()); T(0x02 == cpu.A); T(flags(0));
    T(8==step()); T(0x00 == cpu.B); T(flags(Z80_ZF|Z80_PF|Z80_CF));
    T(8==step()); T(0x54 == cpu.C); T(flags(Z80_CF));
    T(8==step()); T(0xFC == cpu.D); T(flags(Z80_SF|Z80_PF|Z80_CF));
    T(8==step()); T(0xFE == cpu.E); T(flags(Z80_SF));
    T(8==step()); T(0x22 == cpu.H); T(flags(Z80_PF));
    T(8==step()); T(0x00 == cpu.L); T(flags(Z80_ZF|Z80_PF));
}

/* SLA (HL)/(IX+d)/(IY+d) */
void SLA_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> SLA (HL)/(IX+d)/(IY+d)");
}

/* SRA r */
void SRA_r() {
    puts(">>> SRA r");
    uint8_t prog[] = {
        0x3E, 0x01,         // LD A,0x01
        0x06, 0x80,         // LD B,0x80
        0x0E, 0xAA,         // LD C,0xAA
        0x16, 0xFE,         // LD D,0xFE
        0x1E, 0x7F,         // LD E,0x7F
        0x26, 0x11,         // LD H,0x11
        0x2E, 0x00,         // LD L,0x00
        0xCB, 0x2F,         // SRA A
        0xCB, 0x28,         // SRA B
        0xCB, 0x29,         // SRA C
        0xCB, 0x2A,         // SRA D
        0xCB, 0x2B,         // SRA E
        0xCB, 0x2C,         // SRA H
        0xCB, 0x2D,         // SRA L
    };
    copy(0x0000, prog, sizeof(prog));
    init();
    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(8==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_PF|Z80_CF));
    T(8==step()); T(0xC0 == cpu.B); T(flags(Z80_SF|Z80_PF));
    T(8==step()); T(0xD5 == cpu.C); T(flags(Z80_SF));
    T(8==step()); T(0xFF == cpu.D); T(flags(Z80_SF|Z80_PF));
    T(8==step()); T(0x3F == cpu.E); T(flags(Z80_PF|Z80_CF));
    T(8==step()); T(0x08 == cpu.H); T(flags(Z80_CF));
    T(8==step()); T(0x00 == cpu.L); T(flags(Z80_ZF|Z80_PF));
}

/* SRA (HL)/(IX+d)/(IY+d) */
void SRA_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> SRA (HL)/(IX+d)/(IY+d)");
}

/* SRL r */
void SRL_r() {
    puts(">>> SRL r");
    uint8_t prog[] = {
        0x3E, 0x01,         // LD A,0x01
        0x06, 0x80,         // LD B,0x80
        0x0E, 0xAA,         // LD C,0xAA
        0x16, 0xFE,         // LD D,0xFE
        0x1E, 0x7F,         // LD E,0x7F
        0x26, 0x11,         // LD H,0x11
        0x2E, 0x00,         // LD L,0x00
        0xCB, 0x3F,         // SRL A
        0xCB, 0x38,         // SRL B
        0xCB, 0x39,         // SRL C
        0xCB, 0x3A,         // SRL D
        0xCB, 0x3B,         // SRL E
        0xCB, 0x3C,         // SRL H
        0xCB, 0x3D,         // SRL L
    };
    copy(0x0000, prog, sizeof(prog));
    init();

    // skip loads
    for (int i = 0; i < 7; i++) {
        step();
    }
    T(8==step()); T(0x00 == cpu.A); T(flags(Z80_ZF|Z80_PF|Z80_CF));
    T(8==step()); T(0x40 == cpu.B); T(flags(0));
    T(8==step()); T(0x55 == cpu.C); T(flags(Z80_PF));
    T(8==step()); T(0x7F == cpu.D); T(flags(0));
    T(8==step()); T(0x3F == cpu.E); T(flags(Z80_PF|Z80_CF));
    T(8==step()); T(0x08 == cpu.H); T(flags(Z80_CF));
    T(8==step()); T(0x00 == cpu.L); T(flags(Z80_ZF|Z80_PF))
}

/* SRL (HL)/(IX+d)/(IY+d) */
void SRL_iHLIXIYi() {
    puts("FIXME FIXME FIXME >>> SRL (HL)/(IX+d)/(IY+d)");
}

/* RLD; RRD */
void RLD_RRD() {
    puts(">>> RLD; RRD");
    uint8_t prog[] = {
        0x3E, 0x12,         // LD A,0x12
        0x21, 0x00, 0x10,   // LD HL,0x1000
        0x36, 0x34,         // LD (HL),0x34
        0xED, 0x67,         // RRD
        0xED, 0x6F,         // RLD
        0x7E,               // LD A,(HL)
        0x3E, 0xFE,         // LD A,0xFE
        0x36, 0x00,         // LD (HL),0x00
        0xED, 0x6F,         // RLD
        0xED, 0x67,         // RRD
        0x7E,               // LD A,(HL)
        0x3E, 0x01,         // LD A,0x01
        0x36, 0x00,         // LD (HL),0x00
        0xED, 0x6F,         // RLD
        0xED, 0x67,         // RRD
        0x7E
    };
    copy(0x0000, prog, sizeof(prog));
    init();
    T(7 ==step()); T(0x12 == cpu.A);
    T(10==step()); T(0x1000 == cpu.HL);
    T(10==step()); T(0x34 == mem[0x1000]);
    T(18==step()); T(0x14 == cpu.A); T(0x23 == mem[0x1000]);
    T(18==step()); T(0x12 == cpu.A); T(0x34 == mem[0x1000]);
    T(7 ==step()); T(0x34 == cpu.A);
    T(7 ==step()); T(0xFE == cpu.A);
    T(10==step()); T(0x00 == mem[0x1000]);
    T(18==step()); T(0xF0 == cpu.A); T(0x0E == mem[0x1000]); T(flags(Z80_SF|Z80_PF));
    T(18==step()); T(0xFE == cpu.A); T(0x00 == mem[0x1000]); T(flags(Z80_SF));
    T(7 ==step()); T(0x00 == cpu.A);
    T(7 ==step()); T(0x01 == cpu.A);
    T(10 ==step()); T(0x00 == mem[0x1000]);
    cpu.F |= Z80_CF;
    T(18==step()); T(0x00 == cpu.A); T(0x01 == mem[0x1000]); T(flags(Z80_ZF|Z80_PF|Z80_CF));
    T(18==step()); T(0x01 == cpu.A); T(0x00 == mem[0x1000]); T(flags(Z80_CF));
    T(7 ==step()); T(0x00 == cpu.A);
}

int main() {
    LD_A_RI();
    LD_IR_A();
    LD_r_sn();
    LD_r_iHLi();
    LD_r_iIXIYi();
    LD_iHLi_r();
    LD_iIXIYi_r();
    LD_iHLi_n();
    LD_iIXIYi_n();
    LD_A_iBCDEnni();
    LD_iBCDEnni_A();
    LD_ddIXIY_nn();
    LD_HLddIXIY_inni();
    LD_inni_HLddIXIY();
    LD_SP_HLIXIY();
    PUSH_POP_qqIXIY();
    EX();
    ADD_A_rn();
    ADD_A_iHLIXIYi();
    ADC_A_rn();
    ADC_A_iHLIXIYi();
    SUB_A_rn();
    SUB_A_iHLIXIYi();
    CP_A_rn();
    CP_A_iHLIXIYi();
    SBC_A_rn();
    SBC_A_iHLIXIYi();
    OR_A_rn();
    XOR_A_rn();
    OR_XOR_A_iHLIXIYi();
    AND_A_rn();
    AND_A_iHLIXIYi();
    INC_DEC_r();
    INC_DEC_iHLIXIYi();
    INC_DEC_ssIXIY();
    RLCA_RLA_RRCA_RRA();
    RLC_RL_RRC_RR_r();
    RRC_RLC_RR_RL_iHLIXIYi();
    SLA_r();
    SLA_iHLIXIYi();
    SRA_r();
    SRA_iHLIXIYi();
    SRL_r();
    SRL_iHLIXIYi();
    RLD_RRD();
    printf("%d tests run ok.\n", num_tests);
    return 0;
}
