
SECTION .rodata

addressText: db 'At: ', 0
PFAddr: db 'Attempted to access ', 0
errorCode: db 'Error code: ', 0

excList:
dq excDE
dq excDB
dq excNMI
dq excBP
dq excOF
dq excBR
dq excUD
dq excNM
dq excDF
dq excCSO
dq excTS
dq excNP
dq excSS
dq excGP
dq excPF
dq undefinedInterrupt
dq excMF
dq excAC
dq excMC
dq excXM
dq excVE

DEmsg:	db 'Division error', endl
DBmsg:	db 'Debug error', endl
BPmsg:	db 'Breakpoint reached', endl
OFmsg:	db 'Overflow', endl
BRmsg:	db 'BOUND Range exceeded', endl
UDmsg:	db 'Invalid opcode detected', endl
NMmsg:	db 'Coprocessor not available', endl
DFmsg:	db 'Double Fault', endl
CSOmsg:	db 'Coprocessor segment overrun', endl
TSmsg:	db 'Invalid TSS', endl
NPmsg:	db 'Segment not present', endl
SSmsg:	db 'Stack fault', endl
GPmsg:	db 'General protection fault', endl
PFmsg:	db 'Page fault', endl
MFmsg:	db 'Coprocessor error', endl
ACmsg:	db 'Alignment check', endl
MCmsg:	db 'Machine check', endl
XMmsg:	db 'SIMD floating point error', endl
VEmsg:	db 'Virtualization exception', endl