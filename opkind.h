/* opcode kind code include file */

#define NOP 1
#define RESET 2
#define RTE 3
#define RTR 4
#define RTS 5
#define STOP 6
#define TRAPV 7
#define LINK 8
#define SWAP 9
#define UNLK 10
#define MOVEu 11
#define TRAP 12
#define EXT 13
#define ASLm 14
#define ASRm 15
#define BCHGi 16
#define BCLRi 17
#define BSETi 18
#define BTSTi 19
#define JMP 20
#define JSR 21
#define LSLm 22
#define LSRm 23
#define MOVEc 24
#define MOVEts 25
#define MOVEfs 26
#define NBCD 27
#define PEA 28
#define ROLm 29
#define RORm 30
#define ROXLm 31
#define ROXRm 32
#define TAS 33
#define ABCDr 34
#define ABCDm 35
#define SBCDr 36
#define SBCDm 37
#define MOVEMf 38
#define MOVEMt 39
#define DBcc 40
#define ADDI 41
#define ANDI 42
#define CLR 43
#define CMPI 44
#define EORI 45
#define NEG 46
#define NEGX 47
#define NOT 48
#define ORI  49
#define SUBI 50
#define TST 51
#define ADDXr 52
#define ADDXm 53
#define CMPM 54
#define MOVEP 55
#define SUBXr 56
#define SUBXm 57
#define BCHGr 58
#define BCLRr 59
#define BSETr 60
#define BTSTr 61
#define CHK 62
#define DIVS 63
#define DIVU 64
#define LEA 65
#define MULS 66
#define MULU 67
#define ASLr 68
#define ASRr 69
#define LSLr 70
#define LSRr 71
#define ROLr 72
#define RORr 73
#define ROXLr 74
#define ROXRr 75
#define ADDA 76
#define CMPA 77
#define SUBA 78
#define Scc  79
#define MOVEA 80
#define ADDQ 81
#define CMP 82
#define EOR 83
#define EXG 84
#define MOVEQ 85
#define SUBQ 86
#define ADD 87
#define AND 88
#define Bcc 89
#define OR 90
#define SUB 91
#define MOVE 92
#define BRA 93
#define BSR 94
#define LINEA 100
#define LINEF 101

/* "STOP" instructions, which are known not to return control to the
   following statement. */
#define STPINST(x) (((x)==BRA)||((x)==JMP)||((x)==RTS)||((x)==RTE)||((x)==RTR))

/* unsecure instructions, which sometimes don't return control to the
   following statement, but most often do. */
#define USCINST(x) (((x)==TRAP)||((x)==STOP)||(x)==BSR)||((x)==JSR)||((x)==DBcc)||((x)==Bcc)||((x)==LINEA)||((x==LINEF))

/* secure instructions, which are known to return control to the
   following statement. */
#define SECINST(x) (!(STPINST(x)||USCINST))

/* control transfer instructions, which can create references to text sections*/
#define TRCTL(x) ( ((x)==BRA)||((x)==JMP)||((x)==BSR)||((x)==JSR)||((x)==DBcc)||((x)==Bcc) )
