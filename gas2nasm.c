/*	gas2nasm.c
	Copyright(C) 2004 H.Kawai, 2026 yi_xi_yun. All rights reserved.
*/

#include <windows.h>

typedef unsigned char UCHAR;
typedef unsigned int UINT;

static UCHAR *dest0_, *dest1_;

#define FLAG_t		0
#define FLAG_d		1
#define FLAG_b		2
#define FLAG_T		3
#define FLAG_D		4
#define FLAG_B		5
#define FLAG_a		6
#define FLAG_e		7

#define	NL			"\r\n"
#define	LEN_NL		2

#define SECTION_TEXT	0
#define SECTION_DATA	1
#define SECTION_BSS		2
#define SECTION_RDATA	3

struct STR_FLAGS {
	UCHAR opt[8];
};

struct stack_alloc {
	UCHAR ibuf[8 * 1024 * 1024];	
	UCHAR filename[1000];
	UCHAR obuf[8 * 1024 * 1024];
	UCHAR for_align[16];
};

UCHAR *readfile(UCHAR *name, UCHAR *b0, UCHAR *b1);
static void errout(UCHAR *s);
static void errout_s_NL(UCHAR *s, UCHAR *t);
static UCHAR *convmain(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1, struct STR_FLAGS flags);
unsigned int GO_strlen(const UCHAR *s);

#define GOLD_write_t	GOLD_write_b
#define GOLD_exit		ExitProcess


static char errflag = 0;

static UCHAR *param_p[6];

int main(int argc, char **argv)
{
	struct stack_alloc *pwork;
	UCHAR *p0, *f, *src1, *filename, i = 0;
	struct STR_FLAGS flags;
	int j;

	pwork = (struct stack_alloc *) malloc(sizeof (struct stack_alloc));

	for (j = 0; j < 8; j++)
		flags.opt[j] = 0;

	while (--argc) {
		p0 = *++argv;
		if (*p0 == '-') {
			while (*++p0) {
				if (*p0 == 'a')
					flags.opt[FLAG_a] = 1;
				if (*p0 == 'e')
					flags.opt[FLAG_e] = 1;
				if (*p0 == 'd') {
					j = FLAG_d;
	getparam:
					do {
						p0++;
					} while (*p0 != '\0' && *p0 <= ' ');
					if (*p0 == '\0') {
						p0 = *++argv;
						argc--;
						if (argc > 1)
							goto getparam;
						flags.opt[j] = 0;
						goto skip;
					}
					param_p[j] = p0;
					while ('0' <= *p0 && *p0 <= '9')
						p0++;
					flags.opt[j] = p0 - param_p[j];
					p0--; /* 为了下一次循环 */
					continue;
				}
				if (*p0 == 'b') {
					j = FLAG_b;
					goto getparam;
				}
				if (*p0 == 't') {
					j = FLAG_t;
					goto getparam;
				}
				if (*p0 == 'D') {
					j = FLAG_D;
					goto getparam;
				}
				if (*p0 == 'B') {
					j = FLAG_B;
					goto getparam;
				}
				if (*p0 == 'T') {
					j = FLAG_T;
					goto getparam;
				}
			}
		} else {
			filename = p0;
			if (i == 0)
				src1 = readfile(filename, pwork->ibuf, pwork->ibuf + sizeof (pwork->ibuf));
			i++;
		}
	}
skip:
	if (i != 2) {
		errout("gas2nasm" NL "Copyright(C) 2004 H.Kawai, 2026 yi_xi_yun." NL
			"usage : >gas2nasm input-file output-file" NL);
	}
	src1 = convmain(pwork->ibuf, src1, pwork->obuf, pwork->obuf + sizeof (pwork->obuf), flags);
	if (src1 == NULL)
		errout("output filebuf over!" NL);
	if (GOLD_write_t(filename, src1 - pwork->obuf, pwork->obuf))
		errout_s_NL("can't write file: ", filename);

	return errflag;
}

static void output(UINT l, UCHAR *s)
{
	if (l) {
		if (dest0_ + l >= dest1_)
			dest0_ = NULL;
		if (dest0_) {
			do {
				*dest0_++ = *s++;
			} while (--l);
		}
	}
	return;
}

void msgout0(int len, UCHAR *s)
{
	GOLD_write_t(NULL, len, s);
	return;
}


void msgout(UCHAR *s)
{
	GOLD_write_t(NULL, GO_strlen(s), s);
	return;
}

void errout(UCHAR *s)
{
	msgout(s);
	GOLD_exit(1);
}

void errout_s_NL(UCHAR *s, UCHAR *t)
{
	msgout(s);
	msgout(t);
	msgout(NL);
	GOLD_exit(1);
}

UCHAR *readfile(UCHAR *name, UCHAR *b0, UCHAR *b1)
{
	HANDLE h;
	int bytes, len = b1 - b0;
	h = CreateFileA((char *) name, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		errout_s_NL("can't open file: ", name);
	if (ReadFile(h, b0, len, &bytes, NULL))
	CloseHandle(h);
	if (len == bytes)
		errout("input filebuf over!" NL);
	return b0 + bytes;
}

/* for w32 */

int GOLD_write_b(const UCHAR *name, int len, const UCHAR *p0)
/* 以二进制模式输出到文件 */
{
	HANDLE h;
	int ll = 0;
	if (name)
		h = CreateFileA((char *) name, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
	else
		h = GetStdHandle(STD_OUTPUT_HANDLE);
	if (h == INVALID_HANDLE_VALUE)
		goto err;
	if (len)
		WriteFile(h, p0, len, &ll, NULL);
	if (name)
		CloseHandle(h);
	if (ll != len)
		goto err;
	return 0;
err:
	return 1;
}
unsigned int GO_strlen(const UCHAR *s)
{
	const UCHAR *t = s;
	while (*s)
		s++;
	return s - t;
}

/* 被各个 gas2nask 变体包含 */
/*		Copyright(C) 2004 H.Kawai, 2026 yi_xi_yun.   (KL-01) */
static UCHAR *checkparam(UCHAR *p);
static void convparam(UCHAR *p, int i);

static UCHAR *skipspace(UCHAR *p)
{
	while (*p <= ' ' && *p != '\n')
		p++;
	return p;
}

static UCHAR *getparam(UCHAR *p)
{
	if (*p != '\n') {
		do {
			p++;
		} while (*p != ',' && *p > ' ' && *p != ';');
	}
	return p;
}

static UCHAR *seek_token_end(UCHAR *s)
{
	while (*s != ':' && *s != '+' && *s > ' ' && *s != '(' && *s != '#' && *s != '*' && *s != '$'
			&& *s != ')' && *s != 0x22 && *s != ',' && *s != ';' && *s != '-')
		s++;
	return s;
}

//static UCHAR *opcmp(UCHAR *q, UCHAR *s)
//{
//	int i;
//	for (i = 0; i < 8; i++) {
//		if (q[i] == ' ' && (s[i] <= ' ' || s[i] == ';'))
//			goto match;
//		if (q[i] != s[i])
//			goto mismatch;
//	}
//	if (s[i] <= ' ' || s[i] == ';')
//		goto match;
//mismatch:
//	return NULL;
//match:
//	return skipspace(&s[i]);
//}

/*重写opcmp：取消8字符限制（为支持更长的指令） */
static UCHAR *opcmp(UCHAR *q, UCHAR *s){ 
	while(*q && *q!=' '){
		if(*q!=*s) goto mismatch;
		q++;s++;
	}
	if(*s<=' ' || *s==';')goto match;
mismatch:
	return NULL;
match:
	return skipspace(s);
}

/* NEW: 取消8字符限制后，寻找nasm指令不再是固定位置，要一个辅助函数 */
static UCHAR *tbl_nasm(UCHAR *p){
	while(*p && *p!=' ')p++;
	while(*p==' ')p++;
	return p;
}


static char my_strcmp(const char *s, const char *t)
{
	while (*s == *t) {
		s++;
		t++;
		if (*t == '\0' && (*s <= ' ' || *s == ';'))
			return 1;
	}
	return 0;
}

/* NEW: 处理lock/rep指令的辅助函数 */
static const char *get_prefix(UCHAR *s, int len)
{
	if (len == 4 && s[0]=='l' && s[1]=='o' && s[2]=='c' && s[3]=='k')
		return "LOCK ";
	if (len == 3 && s[0]=='r' && s[1]=='e' && s[2]=='p')
		return "REP ";
	if (len == 4 && s[0]=='r' && s[1]=='e' && s[2]=='p' && s[3]=='e')
		return "REPE ";
	if (len == 4 && s[0]=='r' && s[1]=='e' && s[2]=='p' && s[3]=='z')
		return "REPZ ";
	if (len == 5 && s[0]=='r' && s[1]=='e' && s[2]=='p' && s[3]=='n' && s[4]=='e')
		return "REPNE ";
	if (len == 5 && s[0]=='r' && s[1]=='e' && s[2]=='p' && s[3]=='n' && s[4]=='z')
		return "REPNZ ";
	return NULL;
}

/* NEW: 处理段超越前缀的辅助函数*/
static int is_seg_prefix(UCHAR *p){
	return p[0] == '%' && p[3] == ':' && p[2] == 's'
	    && (p[1] == 'c' || p[1] == 'd' || p[1] == 'e'
	     || p[1] == 'f' || p[1] == 'g' || p[1] == 's');
}


static UCHAR *convmain(UCHAR *src0, UCHAR *src1, UCHAR *dest0, UCHAR *dest1, struct STR_FLAGS flags)
{
	UCHAR *p, *q, *r, *s, *t;
	UCHAR intext = 0, c, flag, flag1, flag2;
	/*update:扩充指令表*/
	static UCHAR no_params[][32] = {
		"cld     CLD",
		"cltd    CDQ",
		"cmpsb   CMPSB",
		"cwtd    CWD",
		"cwtl    CWDE",
		"cbtw    CBW",
		"fabs    FABS",
		"fchs    FCHS",
		"fcos    FCOS",
		"fld1    FLD1",
		"fldz    FLDZ",
		"fsin    FSIN",
		"fsqrt   FSQRT",
		"fucom   FUCOM",
		"fucomp  FUCOMP",
		"fucompp FUCOMPP",
		"leave   LEAVE",
		"movsb   MOVSB",
		"movsl   MOVSD",
		"rep     REP",
		"repe    REPE",
		"repne   REPNE",
		"ret     RET",
		"sahf    SAHF",
		"scasb   SCASB",
		"stosb   STOSB",
		"stosl   STOSD",
		/*以下新增*/
		"nop     NOP",
		"lahf    LAHF",  
		"xlat    XLAT",
		"xlatb   XLATB",
		"pushad  PUSHAD",
		"pushal  PUSHAD",
		"popad   POPAD",
		"popal   POPAD",
		"pushfd  PUSHFD",
		"pushfl  PUSHFD",
		"popfd   POPFD",
		"popfl   POPFD",
		"lodsb   LODSB",
		"lodsl   LODSD",
		"scasl   SCASD",
		"cmpsl   CMPSD",
		"movsw   MOVSW",
		"stosw   STOSW",
		"scasw   SCASW",
		"cmpsw   CMPSW",
		"lodsw   LODSW",
		/*到此结束*/
		"\0"
	};
	static UCHAR one_param[][32] = {
		"0call    CALL",
		"1decb    DEC",
		"4decl    DEC",
		"2decw    DEC",
		"1divb    DIV",
		"4divl    DIV",
		"2divw    DIV",
		"8faddl   FADD",
		"4fadds   FADD",
		"8fdivl   FDIV",
		"4fdivs   FDIV",
		"8fdivrl  FDIVR",
		"4fdivrs  FDIVR",
		"4fidivl  FIDIV",
		"4fidivrl FIDIVR",
		"2filds   FILD",
		"4fildl   FILD",
		"8fildq   FILD",
		"4fimull  FIMUL",
		"2fistps  FISTP",
		"4fistpl  FISTP",
		"8fistpq  FISTP",
		"2fists   FIST",
		"4fistl   FIST",
		"3fld     FLD",
		"2fldcw   FLDCW",
		"8fldl    FLD",
		"4flds    FLD",
		"8fmull   FMUL",
		"4fmuls   FMUL",
		"2fnstcw  FNSTCW",
		"2fnstsw  FNSTSW",
		"8fstl    FST",
		"3fstp    FSTP",
		"8fstpl   FSTP",
		"4fstps   FSTP",
		"4fsts    FST",
		"8fsubl   FSUB",
		"8fsubrl  FSUBR",
		"4fsubrs  FSUBR",
		"3fucom   FUCOM",
		"3fucomp  FUCOMP",
		"3fxch    FXCH",
		"1idivb   IDIV",
		"4idivl   IDIV",
		"2idivw   IDIV",
		"1imulb   IMUL",
		"4imull   IMUL",
		"2imulw   IMUL",
		"1incb    INC",
		"4incl    INC",
		"2incw    INC",
		"0ja      JA",
		"0jae     JAE",
		"0jb      JB",
		"0jbe     JBE",
		"0je      JE",
		"0jg      JG",
		"0jge     JGE",
		"0jl      JL",
		"0jle     JLE",
		"0jmp     JMP",
		"0jne     JNE",
		"0jnp     JNP",
		"0jns     JNS",
		"0jp      JP",
		"0js      JS",
		/*以下新增*/
		"0jo      JO",
		"0jno     JNO",
		/*部分版本的GCC会生成如下命令，意思和上面等价*/
		"0jc      JB",
		"0jnc     JAE",
		"0jz      JE",
		"0jnz     JNE",
		"0jna     JBE",
		"0jnae    JB",
		"0jnb     JAE",
		"0jnbe    JA",
		"0jpe     JP",
		"0jpo     JNP",
		"0jng     JLE",
		"0jnge    JL",
		"0jnl     JGE",
		"0jnle    JG",
		"0jecxz   JECXZ",
		/*到此结束*/ 
		"0loop    LOOP",
		"1mulb    MUL",
		"4mull    MUL",
		"2mulw    MUL",
		"1negb    NEG",
		"4negl    NEG",
		"2negw    NEG",
		"1notb    NOT",
		"4notl    NOT",
		"2notw    NOT",
		"4popl    POP",
		"4pushl   PUSH",
		"4ret     RET",
		"1seta    SETA",
		"1setae   SETAE",
		"1setb    SETB",
		"1setbe   SETBE",
		"1sete    SETE",
		"1setg    SETG",
		"1setge   SETGE",
		"1setl    SETL",
		"1setle   SETLE",
		"1setne   SETNE",
		"1setnp   SETNP",
		"1setns   SETNS",
		"1sets    SETS",
		"1setp    SETP",
		/*以下新增*/
		"1seto    SETO",
		"1setno   SETNO",
		/*部分版本的GCC会生成如下命令，意思和上面等价*/
		"1setc    SETB",
		"1setnc   SETAE",
		"1setz    SETE",
		"1setnz   SETNE",
		"1setna   SETBE",
		"1setnae  SETB",
		"1setnb   SETAE",
		"1setnbe  SETA",
		"1setpe   SETP",
		"1setpo   SETNP",
		"1setng   SETLE",
		"1setnge  SETL",
		"1setnl   SETGE",
		"1setnle  SETG",	
		/*其他重要指令*/
		"4bswap  BSWAP",
		"4bswapl BSWAP",
		"2pushw  PUSH",
		"2popw   POP",
		"0loope  LOOPE",
		"0loopz  LOOPE",
		"0loopne LOOPNE",
		"0loopnz LOOPNE",
		"0jcxz   JCXZ",
		"8cmpxchg8b CMPXCHG8B",
		/*到此结束*/
		"\0"
	};
	static UCHAR one_shifts[][32] = {
		"1salb    SAL",
		"4sall    SAL",
		"2salw    SAL",
		"1sarb    SAR",
		"4sarl    SAR",
		"2sarw    SAR",
		"1shrb    SHR",
		"4shrl    SHR",
		"2shrw    SHR",
		"\0"
	};
	static UCHAR two_params[][32] = {
		"1addb    ADD",
		"4addl    ADD",
		"2addw    ADD",
		"1andb    AND",
		"4andl    AND",
		"2andw    AND",
		"1cmpb    CMP",
		"4cmpl    CMP",
		"2cmpw    CMP",
		"3fadd    FADD",
		"3faddp   FADDP",
		"3fdivp   FDIVRP",
		"3fdivrp  FDIVP",
		"3fmul    FMUL",
		"3fmulp   FMULP",
		"3fsubp   FSUBRP",
		"3fsubrp  FSUBP",
		"4imull   IMUL",
		"2imulw   IMUL",
		"1inb     IN",
		"4inl     IN",
		"2inw     IN",
		"4leal    LEA",
		"1movb    MOV",
		"4movl    MOV",
		"2movw    MOV",
		"1movsbl  MOVSX",
		"1movsbw  MOVSX",
		"2movswl  MOVSX",
		"1movzbl  MOVZX",
		"1movzbw  MOVZX",
		"2movzwl  MOVZX",
		"1orb     OR",
		"4orl     OR",
		"2orw     OR",
		"1outb    OUT",
		"4outl    OUT",
		"2outw    OUT",
		"1rolb    ROL",
		"4roll    ROL",
		"2rolw    ROL",
		"1salb    SAL",
		"4sall    SAL",
		"2salw    SAL",
		"1sarb    SAR",
		"4sarl    SAR",
		"2sarw    SAR",
		"4sbbl    SBB",
		"1shrb    SHR",
		"4shrl    SHR",
		"2shrw    SHR",
		"1subb    SUB",
		"4subl    SUB",
		"2subw    SUB",
		"1testb   TEST",
		"4testl   TEST",
		"2testw   TEST",
		"1xchgb   XCHG",
		"4xchgl   XCHG",
		"2xchgw   XCHG",
		"1xorb    XOR",
		"4xorl    XOR",
		"2xorw    XOR",
		/*以下新增*/
		"2btw    BT",
		"4btl    BT",
		"2btsw   BTS",
		"4btsl   BTS",
		"2btrw   BTR",
		"4btrl   BTR",
		"2btcw   BTC",
		"4btcl   BTC",
		"1sbbb    SBB",
		"2sbbw    SBB",
		"1shlb    SHL",
		"4shll    SHL",
		"2shlw    SHL",	
		"1adcb    ADC",
		"4adcl    ADC",
		"2adcw    ADC",
		"1rorb    ROR",
		"4rorl    ROR",
		"2rorw    ROR",
		"1xaddb   XADD",
		"2xaddw   XADD",
		"4xaddl   XADD",
		"1cmpxchgb CMPXCHG",
		"4cmpxchgl CMPXCHG",
		"2cmpxchgw CMPXCHG",
		"3movaps  MOVAPS",
		"3movups  MOVUPS",
		"3movdqa  MOVDQA",
		"3movdqu  MOVDQU",
		/*CMOVcc系列*/
		"4cmovol  CMOVO",
		"4cmovnol CMOVNO",
		"4cmovbl  CMOVB",
		"4cmovael CMOVAE",
		"4cmovel  CMOVE",
		"4cmovnel CMOVNE",
		"4cmovbel CMOVBE",
		"4cmoval  CMOVA",
		"4cmovsl  CMOVS",
		"4cmovnsl CMOVNS",
		"4cmovpl  CMOVP",
		"4cmovnpl CMOVNP",
		"4cmovll  CMOVL",
		"4cmovgel CMOVGE",
		"4cmovlel CMOVLE",
		"4cmovgl  CMOVG",	
		"4cmovcl  CMOVB",
		"4cmovncl CMOVAE",
		"4cmovzl  CMOVE",
		"4cmovnzl CMOVNE",
		"4cmovnal CMOVBE",
		"4cmovnael CMOVB",
		"4cmovnbl  CMOVAE",
		"4cmovnbel CMOVA",
		"4cmovpel  CMOVP",
		"4cmovpol  CMOVNP",
		"4cmovngel CMOVL",
		"4cmovnll  CMOVGE",
		"4cmovngl  CMOVLE",
		"4cmovnlel CMOVG",
		/*GCC也可能这么输出*/
		"4cmovo  CMOVO",
		"4cmovno CMOVNO",
		"4cmovb  CMOVB",
		"4cmovae CMOVAE",
		"4cmove  CMOVE",
		"4cmovne CMOVNE",
		"4cmovbe CMOVBE",
		"4cmova  CMOVA",
		"4cmovs  CMOVS",
		"4cmovns CMOVNS",
		"4cmovp   CMOVP",
		"4cmovnp  CMOVNP",
		"4cmovl   CMOVL",
		"4cmovge  CMOVGE",
		"4cmovle  CMOVLE",
		"4cmovg   CMOVG",	
		"4cmovc   CMOVB",
		"4cmovnc  CMOVAE",
		"4cmovz   CMOVE",
		"4cmovnz  CMOVNE",
		"4cmovna  CMOVBE",
		"4cmovnae  CMOVB",
		"4cmovnb   CMOVAE",
		"4cmovnbe  CMOVA",
		"4cmovpe   CMOVP",
		"4cmovpo   CMOVNP",
		"4cmovnge  CMOVL",
		"4cmovnl   CMOVGE",
		"4cmovng   CMOVLE",
		"4cmovnle  CMOVG",	
		/*到此结束*/
		"\0"
	};

	static UCHAR three_params[][32] = {
		"4imull   IMUL",
		"2imulw   IMUL",
		/*以下新增*/
		"2shldw   SHLD",
		"4shldl   SHLD",
		"2shrdw   SHRD",
		"4shrdl   SHRD",
		/*到此结束*/
		"\0"
	};

	static UCHAR fdivfsub[][32] = {
		"1fdiv    FDIV",
		"0fdivr   FDIV",
		"1fsub    FSUB",
		"0fsubr   FSUB",
		"\0"
	};

	int i, j;
	int currentsection=SECTION_TEXT; 

	dest0_ = dest0;
	dest1_ = dest1;

	output(sizeof("[BITS 32]")+LEN_NL-1, "[BITS 32]" NL);
	//update:以下不是标准nasm语法，故删去 
//	output(16 + LEN_NL, "[FORMAT \"WCOFF\"]" NL);					//从此开始 
//	output(18 + LEN_NL + 12 + LEN_NL + 10 + LEN_NL + 9 + LEN_NL
//			/* + 15 + LEN_NL + 15 + LEN_NL */, 
//		"[INSTRSET \"i486p\"]" NL
//		"[OPTIMIZE 1]" NL
//		"[OPTION 1]" NL
//		"[BITS 32]" NL
//	);																//到此结束 

	if (flags.opt[FLAG_T]) {
		output(21, "[SECTION .text align=");
		output(flags.opt[FLAG_T], param_p[FLAG_T]);
		output(1 + LEN_NL, "]" NL);
	}
	if (flags.opt[FLAG_D]) {
		output(21, "[SECTION .data align=");
		output(flags.opt[FLAG_D], param_p[FLAG_D]);
		output(1 + LEN_NL, "]" NL);
	}
	if (flags.opt[FLAG_B]) {
		output(20, "[SECTION .bss align=");
		output(flags.opt[FLAG_B], param_p[FLAG_B]);
		output(1 + LEN_NL, "]" NL);
	}

	if (flags.opt[FLAG_t]) {
		output(15 + LEN_NL + 7, "[SECTION .text]" NL "\tALIGN\t");
		output(flags.opt[FLAG_t], param_p[FLAG_t]);
		output(LEN_NL, NL);
	}
	if (flags.opt[FLAG_d]) {
		output(16 + LEN_NL + 7, "[SECTION .data]" NL "\tALIGNB\t");
		output(flags.opt[FLAG_d], param_p[FLAG_d]);
		output(LEN_NL, NL);
	}
	if (flags.opt[FLAG_b]) {
		output(15 + LEN_NL + 7, "[SECTION .bss]" NL "\tALIGNB\t");
		output(flags.opt[FLAG_b], param_p[FLAG_b]);
		output(LEN_NL, NL);
	}

	/* EXTERN提取 */
	if (src0 > &src1[-1])
		goto no_src;
	if (src1[-1] != '\n') {
		errflag = 1;
		goto no_src;
	}
	/* 算法：以 token 为分隔。登记以 _ 开头的 token */
	r = src0;
	p = dest0_ + (dest1 - dest0_) / 2;
	q = p;
	flag2 = 1;
	do {
		r = skipspace(r);
		s = seek_token_end(r);
		if (r[0] == '.') {
			if (r[1] == 't' && r[2] == 'e' && r[3] == 'x' && r[4] == 't' && r[5] <= ' ') {
				flag2 = 1;
				goto ext_skip2;
			}
			if (r[1] == 'd' && r[2] == 'a' && r[3] == 't' && r[4] == 'a' && r[5] <= ' ') {
				flag2 = 1;
				goto ext_skip2;
			}
			if (r[1] == 'b' && r[2] == 's' && r[3] == 's' && r[4] <= ' ') {
				flag2 = 1;
				goto ext_skip2;
			}
			if (r[1] == 's' && r[2] == 'e' && r[3] == 'c' && r[4] == 't' && r[5] == 'i' && r[6] == 'o' && r[7] == 'n' && r[8] <= ' ') {
				flag2 = 1;	//这里要改成1
				goto ext_skip2;
			}
			/*NEW*/
			/*避免.comm/.lcomm中的内容被识别为外部符号*/
			/*从此开始*/
			if ((r[1]=='c' && r[2]=='o' && r[3]=='m' && r[4]=='m' && r[5] <= ' ') ||
			    (r[1]=='l' && r[2]=='c' && r[3]=='o' && r[4]=='m' && r[5]=='m' && r[6] <= ' ')) {
			    UCHAR *sym;
			    int symlen;
			    if (r[1] == 'c')
			        sym = skipspace(r + 5);
			    else
			        sym = skipspace(r + 6);
			    if (*sym == '_') {
			        s=sym;
					while (*s != ':' && *s != '+' && *s > ' ' && *s != '(' && *s != '#' && *s != '*' && *s != '$' && *s != ')' && *s != 0x22 && *s != ',' && *s != ';' && *s != '-')
						s++; 
			        symlen = s - sym;
			        if (symlen > 0 && symlen <= 255) {
			            if (q > p) {
			                t = p;
			                do {
			                    t += 2;
			                    if (t[-2] != symlen) {
			ext_next_common:
			                        t += t[-2];
			                        continue;
			                    }
			                    for (i = symlen - 1; i >= 0; i--) {
			                        if (sym[i] != t[i])
			                            goto ext_next_common;
			                    }
			                    t[-1] = 0x01;	//已有记录 
			                    goto ext_skip2;
			                } while (t < q);
			            }
			            if (q + symlen + 2 > dest1)
			                goto err;
			            q[0] = symlen;
			            q[1] = 0x01;	//新记录 
			            q += 2;
			            for (i = 0; i < symlen; i++)
			                *q++ = sym[i];
			        }
			    }
			    goto ext_skip2;
			}
			/*到此结束*/
		}
		if (r[0] == '#')
			goto ext_skip2;
		if (r[0] == '/')
			goto ext_skip2;
		if (flag2 == 0)
			goto ext_skip2;
		if (s - r != 0 && *s == ':' && *r == '_') {
			/* 标签定义检测 */
			if (s - r > 255)
				goto ext_skip0;
			c = s - r;
			if (c == 0)
				goto ext_skip0;
			if (q > p) {
				t = p;
				do {
					t += 2;
					if (t[-2] != c) {
	ext_next_label2:
						t += t[-2];
						continue;
					}
					for (i = c - 1; i >= 0; i--) {
						if (r[i] != t[i])
							goto ext_next_label2;
					}
					t[-1] = 0x01; /* internal */
					goto ext_skip0; /* 二重登録回避 */
				} while (t < q);
				if (q + c + 2 > dest1)
					goto err;
				q[0] = c; /* len */ 
				q[1] = 0x01; /* internal */
				q += 2;
				do {
					*q++ = *r++;
				} while (--c);
			}
		}
ext_skip0:
		for (;;) {
			if (*s == 0x22) {
				do {
					s++;
					if (*s == '\\' && s[1] == 0x22)
						s += 2;
					if (*s == '\n')
						break;
				} while (*s != 0x22);
			}
			if (*s == ';')
				break;
			if (*s == '#')
				break;
			if (*s == '\r')
				break;
			if (*s == '\n')
				break;
			r = skipspace(s + 1);
			s = seek_token_end(r);
			if (*r == '_') {
				if (s - r > 255)
					goto ext_skip1; /* 太长了 */
				c = s - r;
				if (q > p) {
					t = p;
					do {
						t += 2;
						if (t[-2] != c) {
		ext_next_label:
							t += t[-2];
							continue;
						}
						for (i = c - 1; i >= 0; i--) {
							if (r[i] != t[i])
								goto ext_next_label;
						}
						goto ext_skip1; /* 二重登録回避 */
					} while (t < q);
				}
				if (q + c + 2 > dest1)
					goto err;
				q[0] = c; /* len */ 
				q[1] = 0; /* external */
				q += 2;
				do {
					*q++ = *r++;
				} while (--c);
			}
ext_skip1:
			;
		}
ext_skip2:
		while (*s != '\n')
			s++;
		r = s + 1;
	} while (r < src1);
	if (q > p) {
		do {
			p += 2;
			if (p[-1] == 0x00) {
				if (dest0_ + 8 + p[-2] + LEN_NL > p)
					goto err;
				output(8, "\tEXTERN\t");
				output(p[-2], p);
				output(LEN_NL, NL);
			}
			p += p[-2];
		} while (p < q);
	}

	/* 一般变换 */
	flag2 = 1;
	do {
		/* 可以假定行尾总是有 '\n' */
		src0 = skipspace(src0);

		/* 伪指令 */
		if (*src0 == '.') {
			if (src0[1] == 'f' && src0[2] == 'i' && src0[3] == 'l' && src0[4] == 'e' && src0[5] <= ' ') {
				/* .file */
				//update：跳过，因标准nasm没有[FILE] 
				goto skip;										//这里之后的部分不会被执行 
				src0 = skipspace(src0 + 5);
				if (*src0 != 0x22) {
	err_skip:
					msgout("skip:");
					p = src0;
					while (*p++ != '\n');
					msgout0(p - src0, src0);
					errflag = 1;
					src0 = p;
					continue;
				}
				p = src0;
				do {
					p++;
				} while (*p != 0x22 && *p != '\n');
				output(6, "[FILE ");
				output(p - src0, src0);
				src0 = p;
				output(2 + LEN_NL, "\"]" NL);
	skip:
				while (*src0 != '\n')
					src0++;
				src0++;
				continue;
			}
			if (src0[1] == 't' && src0[2] == 'e' && src0[3] == 'x' && src0[4] == 't' && src0[5] <= ' ') {
				/* .text */
				intext = 1;
				flag2 = 1;
				currentsection=SECTION_TEXT;
				output(15 + LEN_NL, "[SECTION .text]" NL);
				goto skip;
			}
			if (src0[1] == 'd' && src0[2] == 'a' && src0[3] == 't' && src0[4] == 'a' && src0[5] <= ' ') {
				/* .data */
				intext = 0;
				flag2 = 1;
				currentsection=SECTION_DATA;
				output(15 + LEN_NL, "[SECTION .data]" NL);
				goto skip;
			}
			if (src0[1] == 'b' && src0[2] == 's' && src0[3] == 's' && src0[4] <= ' ') {
				/* .bss */
				intext = 0;
				flag2 = 1;
				currentsection=SECTION_BSS;
				output(14 + LEN_NL, "[SECTION .bss]" NL);
				goto skip;
			}
			if (src0[1]=='r' && src0[2]=='d' && src0[3]=='a' && src0[4]=='t' && src0[5]=='a' && src0[6] <= ' ') {		//这里！ 
				/* .rdata */
		        intext = 0;
				flag2 = 1;
				currentsection=SECTION_RDATA;
		        output(16 + LEN_NL, "[SECTION .rdata]" NL);
		        goto skip;
		    }
			if (src0[1] == 'g' && src0[2] == 'l' && src0[3] == 'o' && src0[4] == 'b' && src0[5] == 'l' && src0[6] <= ' ') {
				/* .globl */
				src0 += 6;
				output(8, "\tGLOBAL\t");
				for (;;) {
					src0 = skipspace(src0);
					p = getparam(src0);
					output(p - src0, src0);
					if (*p != ',')
						break;
					output(1, ",");
					src0 = p + 1;
				}
				output(LEN_NL, NL);
				goto skip;
			}
			if (src0[1] == 's' && src0[2] == 'e' && src0[3] == 'c' && src0[4] == 't' && src0[5] == 'i' &&
				src0[6] == 'o' && src0[7] == 'n' && src0[8] <= ' ') {
				/* .section */
				
				src0 = skipspace(src0 + 8);
			    p = getparam(src0);
			
			    /* .text 及 .text.* */
			    if (p - src0 >= 5 && src0[0]=='.' && src0[1]=='t' && src0[2]=='e'
			        && src0[3]=='x' && src0[4]=='t') {
			        intext = 1; flag2 = 1;
			        currentsection=SECTION_TEXT;
			        output(15 + LEN_NL, "[SECTION .text]" NL);
			        goto skip;
			    }
			    /* .data 及 .data.* */
			    if (p - src0 >= 5 && src0[0]=='.' && src0[1]=='d' && src0[2]=='a'
			        && src0[3]=='t' && src0[4]=='a') {
			        intext = 0; flag2 = 1;
			        currentsection=SECTION_DATA;
			        output(15 + LEN_NL, "[SECTION .data]" NL);
			        goto skip;
			    }
			    /* .bss 及 .bss.* */
			    if (p - src0 >= 4 && src0[0]=='.' && src0[1]=='b' && src0[2]=='s'
			        && src0[3]=='s') {
			        intext = 0; flag2 = 1;
			        currentsection=SECTION_BSS;
			        output(14 + LEN_NL, "[SECTION .bss]" NL);
			        goto skip;
			    }
			    /* .rdata 及 .rdata$* / .rdata.* */
			    if (p - src0 >= 6 && src0[0]=='.' && src0[1]=='r' && src0[2]=='d'
			        && src0[3]=='a' && src0[4]=='t' && src0[5]=='a') {
			        intext = 0; flag2 = 1;
			        currentsection=SECTION_RDATA;
			        output(sizeof("[SECTION .rdata]" NL) - 1, "[SECTION .rdata]" NL);
			        goto skip;
			    }
			    flag2 = 1;
				goto skip;
			}
			/* NEW: 支持.comm/.lcomm */
			
			/*新增：.comm*/
			/*这个在使用静态变量的场景常出现*/
			if (src0[1]=='c' && src0[2]=='o' && src0[3]=='m' && src0[4]=='m' && src0[5] <= ' ') {
			    src0 = skipspace(src0 + 5);
			    p = getparam(src0);              /* name */
			    output(8, "\tCOMMON\t");
			    output(p - src0, src0);
			
			    src0 = skipspace(p + 1);         /* 跳过逗号 */
			    p = getparam(src0);              /* size */
			    output(1, " ");
			    output(p - src0, src0);
			
//			    if (*p == ',') {                 /* 第三个对齐参数nasm不支持，忽略*/
//			        src0 = skipspace(p + 1);
//			        p = getparam(src0);
//			        output(1, ":");
//			        output(p - src0, src0);
//			    }
			    output(LEN_NL, NL);
			    goto skip;
			}
			
			/*新增：.lcomm*/
			/*nasm没有类似语法，需要手动用bss段模拟*/
			/* [gas2nasm-mod] .lcomm name, size[, align] -> 切 .bss + name: resb size + 恢复原段 */
			if (src0[1]=='l' && src0[2]=='c' && src0[3]=='o' && src0[4]=='m' && src0[5]=='m' && src0[6] <= ' ') {
			    UCHAR *name_p, *size_p, *align_p = NULL;
			    int name_len, size_len, align_len = 0, savedsection;
			
			    src0 = skipspace(src0 + 6);
			    name_p = src0;  p = getparam(src0);  name_len = p - name_p;
			
			    src0 = skipspace(p + 1);
			    size_p = src0;  p = getparam(src0);  size_len = p - size_p;
			
			    if (*p == ',') {
			        src0 = skipspace(p + 1);
			        align_p = src0;  p = getparam(src0);  align_len = p - align_p;
			    }
			
			    savedsection=currentsection;
			    output(sizeof("[SECTION .bss]" NL)-1, "[SECTION .bss]" NL);
			    /* 对齐：仅当对齐值不是 1 时输出 ALIGNB，避免空操作 */
			    if (align_p != NULL && !(align_len == 1 && *align_p == '1')) {
			        output(8, "\tALIGNB\t");
			        output(align_len, align_p);
			        output(LEN_NL, NL);
			    }
			    output(name_len, name_p);
			    output(2, ":" NL);
			    output(6, "\tRESB\t");
			    output(size_len, size_p);
			    output(LEN_NL, NL);
			    /* 恢复原段 */
			    if (savedsection == SECTION_TEXT)
			        output(sizeof("[SECTION .text]" NL) - 1, "[SECTION .text]" NL);
			    else if (savedsection == SECTION_DATA)
			        output(sizeof("[SECTION .data]" NL) - 1, "[SECTION .data]" NL);
			    else if (savedsection == SECTION_BSS)
			        output(sizeof("[SECTION .bss]" NL) - 1, "[SECTION .bss]" NL);
			    else if (savedsection == SECTION_RDATA)
			        output(sizeof("[SECTION .rdata]" NL) - 1, "[SECTION .rdata]" NL);
			    goto skip;
			}
			
			
			if (src0[1] == 'd' && src0[2] == 'e' && src0[3] == 'f' && src0[4] <= ' ')
				goto skip; /* 跳过 .def */
			if (src0[1] == 'p' && src0[2] == '2' && src0[3] == 'a' && src0[4] == 'l' && src0[5] == 'i' && src0[6] == 'g' &&
					src0[7] == 'n' && src0[8] <= ' ')
				goto skip; /* 跳过 .p2align */
			if (flag2 == 0)
				goto skip;
			if (src0[1] == 'b' && src0[2] == 'a' && src0[3] == 'l' && src0[4] == 'i' && src0[5] == 'g' && src0[6] == 'n'  && src0[7] <= ' ') {
				/* .balign */
				src0 = skipspace(src0 + 7);
				p = getparam(src0);
				if (intext != 0 && flags.opt[FLAG_a] != 0 && (p - src0) == 1 && *src0 == '2')
					goto skip; 
				if (intext)
					output(7, "\tALIGN\t");
				else
					output(8, "\tALIGNB\t");
				output(p - src0, src0);
				src0 = p;
				output(LEN_NL, NL);
				goto skip;
			}
			if (src0[1] == 's' && src0[2] == 'p' && src0[3] == 'a' && src0[4] == 'c' && src0[5] == 'e' && src0[6] <= ' ') {
				/* .space */
				/* yi_xi_yun注：非.bss段实际上不应该用RESB而是用TIMES N DB 0 
				但实际上因非.bss段是物理上写入文件的，nasm规定此时使用RESB等价于用0填充 
				故这种写法理论上正确，只是会被nasm报警告*/
				src0 = skipspace(src0 + 6);
				p = getparam(src0);
				output(6, "\tRESB\t");
				output(p - src0, src0);
				src0 = p;
				output(LEN_NL, NL);
				goto skip;
			}
			if (src0[1] == 'l' && src0[2] == 'o' && src0[3] == 'n' && src0[4] == 'g' && src0[5] <= ' ') {
				/* .long */
				src0 += 5;
				output(4, "\tDD\t");
	dd_dw_db:
				for (;;) {
					src0 = skipspace(src0);
					p = src0;
					if (*src0 == '\n')
						break;
					do {
						p++;
					} while (*p != ',' && *p != ';' && *p != '\r' && *p != '\n');
					output(p - src0, src0);
					if (*p != ',')
						break;
					output(1, ",");
					src0 = p + 1;
				}
				output(LEN_NL, NL);
				goto skip;
			}
			if (src0[1] == 'b' && src0[2] == 'y' && src0[3] == 't' && src0[4] == 'e' && src0[5] <= ' ') {
				/* .byte */
				src0 += 5;
				output(4, "\tDB\t");
				goto dd_dw_db;
			}
			if (src0[1] == 'w' && src0[2] == 'o' && src0[3] == 'r' && src0[4] == 'd' && src0[5] <= ' ') {
				/* .word */
				src0 += 5;
				output(4, "\tDW\t");
				goto dd_dw_db;
			}
			if (src0[1] == 'a' && src0[2] == 's' && src0[3] == 'c' && src0[4] == 'i' && src0[5] == 'i' && src0[6] <= ' ') {
				/* .ascii */
				src0 += 6;
				output(4, "\tDB\t");
				flag1 = 0;
				for (;;) {
					src0 = skipspace(src0);
					p = src0;
					if (*p++ != 0x22)
						break;
					flag = 0;
					for (;;) {
						c = *p++;
						if (c == 0x22)
							break;
						if (c == '\r')
							break; /* 虽是错误，但处理起来麻烦，故忽略 */
						if (c == '\n')
							break; /* 虽是错误，但处理起来麻烦，故忽略 */
						if (c != '\\') {
							if (flag == 0) {
								if (flag1)
									output(1, ",");
								output(1, "\x22");
								flag = 1;
							}
							output(1, &c);
							flag1 = 1;
							continue;
						}
						c = *p++;
						if (c == '\r')
							break; /* 虽是错误，但处理起来麻烦，故忽略 */
						if (c == '\n')
							break; /* 虽是错误，但处理起来麻烦，故忽略 */
						if (flag) {
							output(1, "\x22");
							flag = 0;
						}
						if (flag1)
							output(1, ",");
						if ('0' <= c && c <= '7') {
							c -= '0';
							if ('0' <= *p && *p <= '7')
								c = (c << 3) | (*p++ - '0');
							if ('0' <= *p && *p <= '7')
								c = (c << 3) | (*p++ - '0');
						}
						output(2, "0x");
						output(1, &"0123456789ABCDEF"[(c >> 4) & 0x0f]);
						output(1, &"0123456789ABCDEF"[c & 0x0f]);
						flag1 = 1;
					}
					if (flag) {
						output(1, "\x22");
					//	flag = 0;
					}
					src0 = skipspace(p);
					if (*src0 == '\n')
						break;
					if (*src0 == ';')
						break;
					if (*src0++ != ',')
						break; /* 虽是错误，但处理起来麻烦，故忽略 */
				}
				output(LEN_NL, NL);
				goto skip;
			}
			/* 无匹配项 */
			goto err_skip;
		}

		if (src0[0] == '#')
			goto skip;
		if (src0[0] == '/')
			goto skip;

		/* 通常命令 */
		if (flag2 == 0)
			goto skip;
retry:
		if (*src0 == '\n') {
			src0++;
			output(LEN_NL, NL);
			continue;
		}
		p = src0;
		do {
			p++;
		} while (*p > ' ' && *p != ';');
		if (p[-1] == ':') {
			/* 标签定义 */
			output(p - src0, src0);
			src0 = skipspace(p);
			goto retry;
		}
		output(1, "\t");

		/*NEW : 检测前缀*/
		const char *pfx = get_prefix(src0, p - src0);
		if (pfx != NULL) {
			UCHAR *nxt = skipspace(p);
			if (*nxt != '\n' && *nxt != ';' && *nxt != '\0') {
				int plen = 0;
				while (pfx[plen])
					plen++;
				output(plen, (UCHAR *)pfx);
				src0 = nxt;
				p = src0;
				do {
					p++;
				} while (*p > ' ' && *p != ';');
			}
		}	
		/*到此结束*/
		
		if (p - src0 > 16)
			goto unknown;
			
		/* 无参数指令 */
		for (i = 0; q = &no_params[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[0], src0)) {
				if (*r == '\n' || *r == ';') { /* 有参数就跳过 */
					output(strlen(tbl_nasm(q)),tbl_nasm(q)); //取消8字符限制 
					output(LEN_NL, NL);
					goto skip;
				}
			}
		}
		//execcmd的内联展开，可能与作者的OSASK操作系统有关但总之标准的gas2nasm用不到 
		if (flags.opt[FLAG_e]) {
			if (r = opcmp("call ", src0)) {
				if (my_strcmp(r, "_lib_execcmd0")) {
					output(41 + LEN_NL * 4,
						"PUSH EBX" NL "LEA EBX,[ESP+4]" NL "CALL 0xc7:0" NL "POP EBX" NL
					);
					goto skip;
				}
				if (my_strcmp(r, "_lib_execcmd1")) {
					output(78 + LEN_NL * 7,
						"POP EAX" NL "PUSH EBX" NL "LEA EBX,[ESP+4]" NL "CALL 0xc7:0" NL
						"POP EBX" NL "PUSH EAX" NL "MOV EAX,[SS:ESP+EAX-8]" NL
					);
					goto skip;
				}
				if (my_strcmp(r, "_lib_execcmd2")) {
					output(76 + LEN_NL * 7,
						"POP EAX" NL "PUSH EBX" NL "LEA EBX,[ESP+4]" NL "CALL 0xc7:0" NL
						"POP EBX" NL "PUSH EAX" NL "MOV EAX,[SS:ESP+EAX]" NL
					);
					goto skip;
				}
			}
		}

		/* 单参数指令 */
		for (i = 0; q = &one_param[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[1], src0)) {
				if (*r != '\n' && *r != ';') { /* 如果没有参数则跳过 */
					r = checkparam(r); /* 逗号或分号，读到\n */
					if (r == NULL)
						break;
					if (*r == ',')
						break;
					output(strlen(tbl_nasm(q+1)),tbl_nasm(q+1)); //取消8字符限制
					output(1, "\t");
					convparam(p, *q - '0');
					output(LEN_NL, NL);
					goto skip;
				}
			}
		}

		/* 单参数的移位指令 */
		for (i = 0; q = &one_shifts[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[1], src0)) {
				if (*r != '\n' && *r != ';') { /* 如果没有参数则跳过 */
					r = checkparam(r); /* 逗号或分号，读到\n */
					if (r == NULL)
						break;
					if (*r == ',')
						break;
					output(strlen(tbl_nasm(q+1)),tbl_nasm(q+1)); //取消8字符限制 
					output(1, "\t");
					convparam(p, *q - '0');
					output(2 + LEN_NL, ",1" NL);
					goto skip;
				}
			}
		}

		/* 双参数指令（交换类型） */
		for (i = 0; q = &two_params[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[1], src0)) {
				if (*r != '\n' && *r != ';') { /* 如果没有参数则跳过 */
					r = checkparam(p);
					if (r == NULL)
						break;
					if (*r != ',')
						break;
					s = checkparam(r + 1);
					if (s == NULL)
						break;
					if (*s == ',')
						break;
					output(strlen(tbl_nasm(q+1)),tbl_nasm(q+1)); //取消8字符限制
					output(1, "\t");
					convparam(r + 1, *q - '0');
					output(1, ",");
					convparam(p, *q - '0');
					output(LEN_NL, NL);
					goto skip;
				}
			}
		}

		/* 三参数指令（逆序类型） */
		for (i = 0; q = &three_params[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[1], src0)) {
				if (*r != '\n' && *r != ';') { /* 如果没有参数则跳过 */
					r = checkparam(r);
					if (r == NULL)
						break;
					if (*r != ',')
						break;
					s = checkparam(r + 1);
					if (s == NULL)
						break;
					if (*s != ',')
						break;
					t = checkparam(s + 1);
					if (t == NULL)
						break;
					if (*t == ',')
						break;
					output(strlen(tbl_nasm(q+1)),tbl_nasm(q+1)); //取消8字符限制
					output(1, "\t");
					convparam(s + 1, *q - '0');
					output(1, ",");
					convparam(r + 1, *q - '0');
					output(1, ",");
					convparam(p, *q - '0');
					output(LEN_NL, NL);
					goto skip;
				}
			}
		}

		/* 双参数指令（交换类型，FDIV/FSUB专用） */
		for (i = 0; q = &fdivfsub[i][0], *q != '\0'; i++) {
			if (r = opcmp(&q[1], src0)) {
				if (*r != '\n' && *r != ';') { /* 如果没有参数则跳过 */
					r = checkparam(r);
					if (r == NULL)
						break;
					if (*r != ',')
						break;
					s = checkparam(r + 1);
					if (s == NULL)
						break;
					if (*s == ',')
						break;
					t = skipspace(p);
					c = '0';
					output(strlen(&q[9]), &q[9]);
					if (t[0] == '%' && t[1] == 's' && t[2] == 't' && t[3] != '(')
						c = '1';
					if (c == *q)
						output(1, "R");
					output(1, "\t");
					convparam(r + 1, 3);
					output(1, ",");
					convparam(p, 3);
					output(LEN_NL, NL);
					goto skip;
				}
			}
		}

unknown:
		/* 不符合 */
		output(LEN_NL, NL);
		goto err_skip;

	} while (src0 < src1);

no_src:
	return dest0_;
err:
	return NULL;
}

static UCHAR *checkparam(UCHAR *p)
{
	p = skipspace(p);
	/*NEW*/
	/*若有段超越前缀，去除并将剩余部分按mem解析*/
	if(is_seg_prefix(p)){
		p=skipspace(p+4);
		goto mem;
	}
	
	if (*p == '%') { /* reg */
		p = skipspace(p + 1);
		if (*p == ',')
			goto err;
		if (*p == '\n')
			goto err;
		if (*p == ';')
			goto err;
		p = getparam(p);
		p = skipspace(p);
		goto fin;
	}

	if (*p == '$') { /* imm */
		p = skipspace(p + 1);
		if (*p == ',')
			goto err;
		if (*p == '\n')
			goto err;
		if (*p == ';')
			goto err;
		do {
			p++;
		} while (*p != '\n' && *p != ',' && *p != ';');
		goto fin;
	}

	if (*p == '\n')
		goto err;
	if (*p == ',')
		goto err;
	if (*p == ';')
		goto err;
		
mem:
	/* mem */
	if (*p != '(') {
		do {
			p++;
			if (*p == '\n')
				goto fin;
			if (*p == ',')
				goto fin;
			if (*p == ';')
				goto fin;
		} while (*p != '(');
	}
	p = skipspace(p + 1);
	if (*p == '%') { /* base */
		p = skipspace(p + 1);
		if (*p == ',')
			goto err;
		if (*p == ')')
			goto err;
		do {
			if (*p++ == '\n')
				goto err;
		} while (*p != ',' && *p != ')');
	}
	if (*p == ')') {
fin2:	
		p++;
		goto fin;
	}
	if (*p != ',')
		goto err;
	p = skipspace(p + 1);
	if (*p == ')')
		goto fin2;
	if (*p != '1' && *p != '%')
		goto err;
	if (*p == '%') {
		p = skipspace(p + 1);
		if (*p == ',')
			goto err;
		if (*p == ')')
			goto err;
		do {
			if (*p++ == '\n')
				goto err;
		} while (*p != ',' && *p != ')');
	}
	if (*p == '1')
		p = skipspace(p + 1);
	if (*p == ')')
		goto fin2;
	do {
		if (*p++ == '\n')
			goto err;
	} while (*p != ')');
	goto fin2;

err:
	p = NULL;
fin:
	return p;
}

static void convparam(UCHAR *p, int i)
{
	UCHAR c, *q;
retry:
	p = skipspace(p);
	/*NEW*/
	/*段超越前缀*/
	if(is_seg_prefix(p)){
		UCHAR s1 = p[1], s2 = p[2];
		p += 4;
		if (i == 1)      output(6, "BYTE [");
		else if (i == 2) output(6, "WORD [");
		else if (i == 3) output(1, "[");
		else if (i == 4) output(7, "DWORD [");
		else if (i == 8) output(7, "QWORD [");
		else if (i == 9) output(7, "TWORD [");
		if ('a' <= s1 && s1 <= 'z') s1 -= 32;
		if ('a' <= s2 && s2 <= 'z') s2 -= 32;
		output(1, &s1);
		output(1, &s2);
		output(1, ":");
		goto mem2;
	}
	if (*p == '%') { /* reg */
		p = skipspace(p + 1);
		if (p[0] == 's' && p[1] == 't' && p[2] != '(') {
			if (dest0_ + 3 >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			p += 2;
			dest0_[0] = 'S';
			dest0_[1] = 'T';
			dest0_[2] = '0';
			dest0_ += 3;
		}
		for (;;) {
			c = *p++;
			if (c == ',')
				goto fin;
			if (c == ';')
				goto fin;
			if (c <= ' ')
				goto fin;
			if ('a' <= c && c <= 'z')
				c += 'A' - 'a';
			if (c == '(')
				continue;
			if (c == ')')
				continue;
			if (dest0_ >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			*dest0_++ = c;
		}
	}
	if (*p == '$') { /* imm */
		p = skipspace(p + 1);
imm:
		for (;;) {
			c = *p++;
			if (c == ',')
				goto fin;
			if (c == ';')
				goto fin;
			if (c == '\n')
				goto fin;
			if (c == '\r')
				goto fin;
			if (dest0_ >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			*dest0_++ = c;
		}
	}
	if (i == 0) { /* call、jmp 等 */
		i += 4;
		if (*p == '*') {
			p++;
			goto retry;
		}
		q = p;
		for (;;) {
			if (*q == ',')
				break;
			if (*q == ';')
				break;
			if (*q == '\n')
				break;
			if (*q == '(')
				goto mem;
			q++;
		}
		goto imm;
	}
mem:
	/* mem */
	if (i == 1)
		output(6, "BYTE [");
	if (i == 2)
		output(6, "WORD [");
	if (i == 3)
		output(1, "[");
	if (i == 4)
		output(7, "DWORD [");
	if (i == 8)
		output(7, "QWORD [");
	if (i == 9)
		output(7, "TWORD [");
mem2: 
	c = 0;
	if (*p != '(') {
		do {
			c = *p++;
			if (c == ',')
				goto fin2;
			if (c == ';')
				goto fin2;
			if (c == '\n')
				goto fin2;
			if (c == '\r')
				goto fin2;
			if (dest0_ >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			*dest0_++ = c;
		} while (*p != '(');
	}
	p = skipspace(p + 1);
	if (*p == '%') { /* base */
		p = skipspace(p + 1);
		if (c)
			output(1, "+");
		do {
			c = *p++;
			if ('a' <= c && c <= 'z')
				c += 'A' - 'a';
			if (dest0_ >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			*dest0_++ = c;
		} while (*p != ',' && *p != ')');
	}
	if (*p == ')')
		goto fin2;
	p = skipspace(p + 1);
	if (*p == ')')
		goto fin2;
	if (*p == '%') { /* index */
		p = skipspace(p + 1);
		if (c)
			output(1, "+");
		do {
			c = *p++;
			if ('a' <= c && c <= 'z')
				c += 'A' - 'a';
			if (dest0_ >= dest1_)
				dest0_ = NULL;
			if (dest0_ == NULL)
				goto fin;
			*dest0_++ = c;
		} while (*p != ',' && *p != ')');
		output(1, "*");
		c = 1;
	}
	if (*p == '1')
		p = skipspace(p + 1);
	if (*p == ')') {
fin3:
		if (c == 1)
			output(1, "1");
		goto fin2;
	}
	p = skipspace(p + 1);
	if (*p == ')')
		goto fin3;
	do {
		if (dest0_ >= dest1_)
			dest0_ = NULL;
		if (dest0_ == NULL)
			goto fin;
		*dest0_++ = *p++;
	} while (*p != ')');
fin2:
	output(1, "]");
fin:
	return;
}
