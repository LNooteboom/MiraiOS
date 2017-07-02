#ifndef LAPICDEFS_H
#define LAPICDEFS_H

#define LAPIC_ID	0x20
#define LAPIC_TPR	0x80
#define LAPIC_EOI	0xB0
#define LAPIC_SPURIOUS	0xF0
#define LAPIC_ICRL	0x300
#define LAPIC_ICRH	0x310

#define LAPIC_LVT_TMR	0x320
#define LAPIC_LVT_THERM	0x330
#define LAPIC_LVT_PERF	0x340
#define LAPIC_LVT_LINT0	0x350
#define LAPIC_LVT_LINT1	0x360

#define LAPIC_TIC		0x380
#define LAPIC_TCC		0x390
#define LAPIC_DIV		0x3E0

#define LAPIC_MASK		(1 << 16)
#define LAPIC_NMI		(4 << 8)

#endif