 /*
  * UAE - The Un*x Amiga Emulatore
  *
  * MC68881 emulation
  *
  * Copyright 1996 Herman ten Brugge
  * adapted by luz/bfo
  */



/*
 *  CVS:
 *    $Author$
 *    $Date$
 *    $Revision$
 *    $Source$
 *    $State$
 *    $Name$ (Tag)
 *    $Locker$ (who has reserved checkout)
 *  Log:
 *    $Log$
 *
 */


#include <math.h>
#include <fenv.h>
#include <float.h>
#include <stdint.h>
#include <string.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include "config.h"
//#include "options.h"
#include "memory.h"
//#include "custom.h"
#include "readcpu.h"
#include "newcpu.h"
//#include "ersatz.h"

#if 1

	#define	DEBUG_FPP	0
//	#define	DEBUG_FPP	1

/* single   : S  8*E 23*F */
/* double   : S 11*E 52*F */
/* extended : S 15*E 64*F */
/* E = 0 & F = 0 -> 0 */
/* E = MAX & F = 0 -> Infin */
/* E = MAX & F # 0 -> NotANumber */
/* E = biased by 127 (single) ,1023 (double) ,16383 (extended) */

/* The 68k's single and double formats ARE the host's IEEE 754 ones, so values
   cross as bit patterns: exactly, with infinities, NaNs and denormals intact.
   Rebuilding them with frexp/ldexp lost all three, and from_double's "round to
   nearest" added half an ULP to a value that already had exactly 53 bits -- the
   add itself rounded to even, so every odd mantissa was stored one ULP high
   (3/10 as $3FD3333333333334). Host and 68k agree on byte order within these
   integers on every host we build for; memcpy is the defined way to move the
   bits across. */
static __inline__ double to_single (uae_u32 value)
{
    float f;
    uint32_t bits= value;

    memcpy( &f, &bits, sizeof(f) );
    return f;
}

static __inline__ uae_u32 from_single (double src)
/* Narrowed as the 68881 does, to nearest and ties to even; a value beyond the
   largest single becomes an infinity rather than a float conversion C leaves
   undefined. */
{
    float    f;
    uint32_t bits;

    if (isfinite( src ) && fabs( src )>(double)FLT_MAX) {
        /* FLT_MAX's mantissa is odd, so its halfway point rounds up */
        double half= ldexp( 1.0, FLT_MAX_EXP-FLT_MANT_DIG-1 );
        f= fabs( src )>=(double)FLT_MAX+half ? (float)INFINITY : FLT_MAX;
        if (src<0) f= -f;
    }
    else f= (float)src;
    memcpy( &bits, &f, sizeof(bits) );
    return bits;
}

/* FPSR's condition code byte for a result: N the sign bit (so -0 and -inf set
   it), Z zero, I infinity, NAN not-a-number. Only N and Z (and N by "< 0") were
   ever set, so a program could not tell that a result had overflowed to an
   infinity: math881 copies these four into the CPU's CCR, where I lands on V,
   and its caller's TRAPV is how BASIC09 reports an overflow or a division by
   zero -- with the FPU in use it reported nothing and printed a garbage value. */
/* The whole FPSR as an operation leaves it: its condition codes, the quotient
   byte (which only FMOD and FREM change: "Quotient Byte: Not affected") and
   the accrued exception byte (sticky) kept, the exception byte cleared, as
   exceptions are not emulated. Every operation used to replace all four
   bytes, wiping what FMOD/FREM and earlier operations had left (review). */
static __inline__ uae_u32 fpsr_keep (uae_u32 cc)
{
    return (regs.fpsr & 0x00FF00FF) | cc;
}

static __inline__ uae_u32 fpsr_cc (double v)
{
    return fpsr_keep ((signbit (v) ? 0x8000000 : 0) |
                      (v == 0      ? 0x4000000 : 0) |
                      (isinf (v)   ? 0x2000000 : 0) |
                      (isnan (v)   ? 0x1000000 : 0));
}

/* FMOD/FREM: the quotient byte -- its sign and its seven low bits -- of the
   quotient rounded toward zero (FMOD) or to nearest (FREM) */
static __inline__ void fpsr_quotient (double dst, double src, int nearest)
{
    double q = dst / src;
    uae_u32 byte;
    if (isnan (q) || isinf (q)) return;
    q = nearest ? nearbyint (q) : trunc (q);
    byte = ((uae_u32) fmod (fabs (q), 128.0)) | (signbit (q) ? 0x80 : 0);
    regs.fpsr = (regs.fpsr & ~0x00FF0000u) | (byte << 16);
}

static __inline__ double to_exten(uae_u32 wrd1, uae_u32 wrd2, uae_u32 wrd3)
{
    double frac;

    if ((wrd1 & 0x7fff0000) == 0 && wrd2 == 0 && wrd3 == 0)
	return 0.0;
    if ((wrd1 & 0x7fff0000) == 0x7fff0000) {  /* infinity (no fraction) or NaN */
	if ((wrd2 & 0x7fffffff) == 0 && wrd3 == 0)
	    return (wrd1 & 0x80000000) ? -INFINITY : INFINITY;
	return NAN;
    }
    frac = (double) wrd2 / 2147483648.0 +
	(double) wrd3 / 9223372036854775808.0;
    if (wrd1 & 0x80000000)
	frac = -frac;
    return ldexp (frac, ((wrd1 >> 16) & 0x7fff) - 16383);
}

static __inline__ void from_exten(double src, uae_u32 * wrd1, uae_u32 * wrd2, uae_u32 * wrd3)
{
    int expon;
    double frac;

    if (isnan( src )) {
	*wrd1 = 0x7fff0000; *wrd2 = 0xffffffff; *wrd3 = 0xffffffff;
	return;
    }
    if (isinf( src )) {
	*wrd1 = src < 0 ? 0xffff0000 : 0x7fff0000; *wrd2 = 0; *wrd3 = 0;
	return;
    }
    if (src == 0.0) {
	*wrd1 = 0;
	*wrd2 = 0;
	*wrd3 = 0;
	return;
    }
    if (src < 0) {
	*wrd1 = 0x80000000;
	src = -src;
    } else {
	*wrd1 = 0;
    }
    frac = frexp (src, &expon);
    frac += 0.5 / 18446744073709551616.0;
    if (frac >= 1.0) {
	frac /= 2.0;
	expon++;
    }
    *wrd1 |= (((expon + 16383 - 1) & 0x7fff) << 16);
    *wrd2 = (uae_u32) (frac * 4294967296.0);
    *wrd3 = (uae_u32) (frac * 18446744073709551616.0 - *wrd2 * 4294967296.0);
}

static __inline__ double to_double(uae_u32 wrd1, uae_u32 wrd2)
{
    double   d;
    uint64_t bits= ((uint64_t)wrd1<<32) | wrd2;

    memcpy( &d, &bits, sizeof(d) );
    return d;
}

static __inline__ void from_double(double src, uae_u32 * wrd1, uae_u32 * wrd2)
{
    uint64_t bits;

    memcpy( &bits, &src, sizeof(bits) );
    *wrd1 = (uae_u32)(bits >> 32);
    *wrd2 = (uae_u32) bits;
}

static __inline__ double to_pack(uae_u32 wrd1, uae_u32 wrd2, uae_u32 wrd3)
{
    double d = 0.0;	/* a string sscanf cannot read left it uninitialised */
    char *cp;
    char str[100];

    /* SE and both Y bits set with exponent $FFF: an infinity when the
       fraction is zero, a NaN otherwise (M68000 PRM, table 1-7). Read as
       digits these became letters, and garbage. */
    if ((wrd1 & 0x7FFF0000) == 0x7FFF0000) {
	if (wrd2 == 0 && wrd3 == 0)
	    return (wrd1 & 0x80000000) ? -INFINITY : INFINITY;
	return NAN;
    }

    cp = str;
    if (wrd1 & 0x80000000)
	*cp++ = '-';
    *cp++ = (wrd1 & 0xf) + '0';
    *cp++ = '.';
    *cp++ = ((wrd2 >> 28) & 0xf) + '0';
    *cp++ = ((wrd2 >> 24) & 0xf) + '0';
    *cp++ = ((wrd2 >> 20) & 0xf) + '0';
    *cp++ = ((wrd2 >> 16) & 0xf) + '0';
    *cp++ = ((wrd2 >> 12) & 0xf) + '0';
    *cp++ = ((wrd2 >> 8) & 0xf) + '0';
    *cp++ = ((wrd2 >> 4) & 0xf) + '0';
    *cp++ = ((wrd2 >> 0) & 0xf) + '0';
    *cp++ = ((wrd3 >> 28) & 0xf) + '0';
    *cp++ = ((wrd3 >> 24) & 0xf) + '0';
    *cp++ = ((wrd3 >> 20) & 0xf) + '0';
    *cp++ = ((wrd3 >> 16) & 0xf) + '0';
    *cp++ = ((wrd3 >> 12) & 0xf) + '0';
    *cp++ = ((wrd3 >> 8) & 0xf) + '0';
    *cp++ = ((wrd3 >> 4) & 0xf) + '0';
    *cp++ = ((wrd3 >> 0) & 0xf) + '0';
    *cp++ = 'E';
    if (wrd1 & 0x40000000)
	*cp++ = '-';
    *cp++ = ((wrd1 >> 24) & 0xf) + '0';
    *cp++ = ((wrd1 >> 20) & 0xf) + '0';
    *cp++ = ((wrd1 >> 16) & 0xf) + '0';
    *cp = 0;
    sscanf(str, "%le", &d);
    return d;
}

static __inline__ void from_pack(double src, uae_u32 * wrd1, uae_u32 * wrd2, uae_u32 * wrd3)
{
    int i;
    int t;
    char *cp;
    char str[100];

    /* an infinity or a NaN has its own packed form (see to_pack); printed
       with %e it was "inf" or "nan", packed as if the letters were digits */
    if (isinf (src) || isnan (src)) {
	*wrd1 = (signbit (src) ? 0x80000000 : 0) | 0x7FFF0000;
	*wrd2 = isnan (src) ? 0x40000000 : 0;	/* a quiet NaN's fraction */
	*wrd3 = 0;
	return;
    }
    sprintf(str, "%.16e", src);
    cp = str;
    *wrd1 = *wrd2 = *wrd3 = 0;
    if (*cp == '-') {
	cp++;
	*wrd1 = 0x80000000;
    }
    if (*cp == '+')
	cp++;
    *wrd1 |= (*cp++ - '0');
    if (*cp == '.')
	cp++;
    for (i = 0; i < 8; i++) {
	*wrd2 <<= 4;
	if (*cp >= '0' && *cp <= '9')
	    *wrd2 |= *cp++ - '0';
    }
    for (i = 0; i < 8; i++) {
	*wrd3 <<= 4;
	if (*cp >= '0' && *cp <= '9')
	    *wrd3 |= *cp++ - '0';
    }
    if (*cp == 'e' || *cp == 'E') {
	cp++;
	if (*cp == '-') {
	    cp++;
	    *wrd1 |= 0x40000000;
	}
	if (*cp == '+')
	    cp++;
	t = 0;
	for (i = 0; i < 3; i++) {
	    if (*cp >= '0' && *cp <= '9')
		t = (t << 4) | (*cp++ - '0');
	}
	*wrd1 |= t << 16;
    }
}

static __inline__ int get_fp_value (uae_u32 opcode, uae_u16 extra, double *src)
{
    uaecptr tmppc;
    uae_u16 tmp;
    int size;
    int mode;
    int reg;
    uae_u32 ad = 0;
    static int sz1[8] =
    {4, 4, 12, 12, 2, 8, 1, 0};
    static int sz2[8] =
    {4, 4, 12, 12, 2, 8, 2, 0};

    if ((extra & 0x4000) == 0) {
	*src = regs.fp[(extra >> 10) & 7];
	return 1;
    }
    mode = (opcode >> 3) & 7;
    reg = opcode & 7;
    size = (extra >> 10) & 7;
    switch (mode) {
    case 0:
	switch (size) {
	case 6:
	    *src = (double) (uae_s8) m68k_dreg (regs, reg);
	    break;
	case 4:
	    *src = (double) (uae_s16) m68k_dreg (regs, reg);
	    break;
	case 0:
	    *src = (double) (uae_s32) m68k_dreg (regs, reg);
	    break;
	case 1:
	    *src = to_single(m68k_dreg (regs, reg));
	    break;
	default:
	    return 0;
	}
	return 1;
    case 1:
	return 0;
    case 2:
	ad = m68k_areg (regs, reg);
	break;
    case 3:
	ad = m68k_areg (regs, reg);
	m68k_areg (regs, reg) += reg == 7 ? sz2[size] : sz1[size];
	break;
    case 4:
	m68k_areg (regs, reg) -= reg == 7 ? sz2[size] : sz1[size];
	ad = m68k_areg (regs, reg);
	break;
    case 5:
	ad = m68k_areg (regs, reg) + (uae_s32) (uae_s16) next_iword();
	break;
    case 6:
	ad = get_disp_ea_020 (m68k_areg (regs, reg), next_iword());
	break;
    case 7:
	switch (reg) {
	case 0:
	    ad = (uae_s32) (uae_s16) next_iword();
	    break;
	case 1:
	    ad = next_ilong();
	    break;
	case 2:
	    ad = m68k_getpc ();
	    ad += (uae_s32) (uae_s16) next_iword();
	    break;
	case 3:
	    tmppc = m68k_getpc ();
	    tmp = next_iword();
	    ad = get_disp_ea_020 (tmppc, tmp);
	    break;
	case 4:
	    ad = m68k_getpc ();
	    m68k_setpc (ad + sz2[size]);
	    break;
	default:
	    return 0;
	}
    }
    switch (size) {
    case 0:
	*src = (double) (uae_s32) get_long (ad);
	break;
    case 1:
	*src = to_single(get_long (ad));
	break;
    case 2:{
	    uae_u32 wrd1, wrd2, wrd3;
	    wrd1 = get_long (ad);
	    ad += 4;
	    wrd2 = get_long (ad);
	    ad += 4;
	    wrd3 = get_long (ad);
	    *src = to_exten(wrd1, wrd2, wrd3);
	}
	break;
    case 3:{
	    uae_u32 wrd1, wrd2, wrd3;
	    wrd1 = get_long (ad);
	    ad += 4;
	    wrd2 = get_long (ad);
	    ad += 4;
	    wrd3 = get_long (ad);
	    *src = to_pack(wrd1, wrd2, wrd3);
	}
	break;
    case 4:
	*src = (double) (uae_s16) get_word(ad);
	break;
    case 5:{
	    uae_u32 wrd1, wrd2;
	    wrd1 = get_long (ad);
	    ad += 4;
	    wrd2 = get_long (ad);
	    *src = to_double(wrd1, wrd2);
	}
	break;
    case 6:
	/* a byte immediate is the LOW byte of its extension word: FMOVE.B #5
	   read the high one and loaded 0 (pre-release review) */
	*src = (double) (uae_s8) get_byte(mode == 7 && reg == 4 ? ad + 1 : ad);
	break;
    default:
	return 0;
    }
    return 1;
}

/* A float stored as an integer (FMOVE to a byte, word or long). The 68881
   rounds by FPCR's mode -- RND, bits 5-4: to nearest, toward zero, toward
   minus infinity, toward plus infinity -- and a value the format cannot hold
   is an operand error that stores its largest integer of that sign; a NaN is
   taken by its sign here. A C cast truncated whatever the mode, and past the
   range was undefined behaviour. */
/* FPCR's rounding mode for the arithmetic of one FPU instruction: the host
   FPU is set to it for the instruction and put back after. It was always to
   nearest, whatever a program set. Hosts without all four modes (WebAssembly
   has only to nearest) keep rounding to nearest. */
#if defined FE_TONEAREST && defined FE_TOWARDZERO && defined FE_DOWNWARD && defined FE_UPWARD
  static int fpp_round_enter (void)
  {
      static const int mode[4] = { FE_TONEAREST, FE_TOWARDZERO, FE_DOWNWARD, FE_UPWARD };
      int sv = fegetround ();
      fesetround (mode[(regs.fpcr >> 4) & 3]);
      return sv;
  }
  static void fpp_round_leave (int sv) { fesetround (sv); }
#else
  static int  fpp_round_enter (void)   { return 0; }
  static void fpp_round_leave (int sv) { (void) sv; }
#endif

/* <value> rounded to an integer the way FPCR's mode control says: nearest
   (even), toward zero, toward minus or toward plus infinity */
static double fpp_round (double value)
{
    switch ((regs.fpcr >> 4) & 3) {
    case 0:  return nearbyint (value);
    case 1:  return trunc (value);
    case 2:  return floor (value);
    default: return ceil (value);
    }
}

static uae_s32 fpp_to_int (double value, double lo, double hi)
{
    double r;
    if (isnan (value)) return (uae_s32) (signbit (value) ? lo : hi);
    r = fpp_round (value);
    return (uae_s32) (r < lo ? lo : r > hi ? hi : r);
}

#define FPP_BYTE(v) fpp_to_int ((v), -128.0, 127.0)
#define FPP_WORD(v) fpp_to_int ((v), -32768.0, 32767.0)
#define FPP_LONG(v) fpp_to_int ((v), -2147483648.0, 2147483647.0)

static __inline__ int put_fp_value (double value, uae_u32 opcode, uae_u16 extra)
{
    uae_u16 tmp;
    uaecptr tmppc;
    int size;
    int mode;
    int reg;
    uae_u32 ad;
    static int sz1[8] =
    {4, 4, 12, 12, 2, 8, 1, 0};
    static int sz2[8] =
    {4, 4, 12, 12, 2, 8, 2, 0};

    if ((extra & 0x4000) == 0) {
	regs.fp[(extra >> 10) & 7] = value;
	return 1;
    }
    mode = (opcode >> 3) & 7;
    reg = opcode & 7;
    size = (extra >> 10) & 7;
    ad = -1;
    switch (mode) {
    case 0:
	switch (size) {
	case 6:
	    m68k_dreg (regs, reg) = ((FPP_BYTE (value) & 0xff)
				    | (m68k_dreg (regs, reg) & ~0xff));
	    break;
	case 4:
	    m68k_dreg (regs, reg) = ((FPP_WORD (value) & 0xffff)
				    | (m68k_dreg (regs, reg) & ~0xffff));
	    break;
	case 0:
	    m68k_dreg (regs, reg) = FPP_LONG (value);
	    break;
	case 1:
	    m68k_dreg (regs, reg) = from_single(value);
	    break;
	default:
	    return 0;
	}
	return 1;
    case 1:
	return 0;
    case 2:
	ad = m68k_areg (regs, reg);
	break;
    case 3:
	ad = m68k_areg (regs, reg);
	m68k_areg (regs, reg) += reg == 7 ? sz2[size] : sz1[size];
	break;
    case 4:
	m68k_areg (regs, reg) -= reg == 7 ? sz2[size] : sz1[size];
	ad = m68k_areg (regs, reg);
	break;
    case 5:
	ad = m68k_areg (regs, reg) + (uae_s32) (uae_s16) next_iword();
	break;
    case 6:
	ad = get_disp_ea_020 (m68k_areg (regs, reg), next_iword());
	break;
    case 7:
	switch (reg) {
	case 0:
	    ad = (uae_s32) (uae_s16) next_iword();
	    break;
	case 1:
	    ad = next_ilong();
	    break;
	case 2:
	    ad = m68k_getpc ();
	    ad += (uae_s32) (uae_s16) next_iword();
	    break;
	case 3:
	    tmppc = m68k_getpc ();
	    tmp = next_iword();
	    ad = get_disp_ea_020 (tmppc, tmp);
	    break;
	case 4:
	    ad = m68k_getpc ();
	    m68k_setpc (ad + sz2[size]);
	    break;
	default:
	    return 0;
	}
    }
    switch (size) {
    case 0:
	put_long (ad, FPP_LONG (value));
	break;
    case 1:
	put_long (ad, from_single(value));
	break;
    case 2:
	{
	    uae_u32 wrd1, wrd2, wrd3;
	    from_exten(value, &wrd1, &wrd2, &wrd3);
	    put_long (ad, wrd1);
	    ad += 4;
	    put_long (ad, wrd2);
	    ad += 4;
	    put_long (ad, wrd3);
	}
	break;
    case 3:
	{
	    uae_u32 wrd1, wrd2, wrd3;
	    from_pack(value, &wrd1, &wrd2, &wrd3);
	    put_long (ad, wrd1);
	    ad += 4;
	    put_long (ad, wrd2);
	    ad += 4;
	    put_long (ad, wrd3);
	}
	break;
    case 4:
	put_word(ad, (uae_s16) FPP_WORD (value));
	break;
    case 5:{
	    uae_u32 wrd1, wrd2;
	    from_double(value, &wrd1, &wrd2);
	    put_long (ad, wrd1);
	    ad += 4;
	    put_long (ad, wrd2);
	}
	break;
    case 6:
	put_byte(ad, (uae_s8) FPP_BYTE (value));
	break;
    default:
	return 0;
    }
    return 1;
}

static __inline__ int get_fp_ad(uae_u32 opcode, uae_u32 * ad)
{
    uae_u16 tmp;
    uaecptr tmppc;
    int mode;
    int reg;

    mode = (opcode >> 3) & 7;
    reg = opcode & 7;
    switch (mode) {
    case 0:
    case 1:
	return 0;
    case 2:
	*ad = m68k_areg (regs, reg);
	break;
    case 3:
	*ad = m68k_areg (regs, reg);
	break;
    case 4:
	*ad = m68k_areg (regs, reg);
	break;
    case 5:
	*ad = m68k_areg (regs, reg) + (uae_s32) (uae_s16) next_iword();
	break;
    case 6:
	*ad = get_disp_ea_020 (m68k_areg (regs, reg), next_iword());
	break;
    case 7:
	switch (reg) {
	case 0:
	    *ad = (uae_s32) (uae_s16) next_iword();
	    break;
	case 1:
	    *ad = next_ilong();
	    break;
	case 2:
	    *ad = m68k_getpc ();
	    *ad += (uae_s32) (uae_s16) next_iword();
	    break;
	case 3:
	    tmppc = m68k_getpc ();
	    tmp = next_iword();
	    *ad = get_disp_ea_020 (tmppc, tmp);
	    break;
	default:
	    return 0;
	}
    }
    return 1;
}

static __inline__ int fpp_cond(uae_u32 opcode, int contition)
{
    int N = (regs.fpsr & 0x8000000) != 0;
    int Z = (regs.fpsr & 0x4000000) != 0;
    /* int I = (regs.fpsr & 0x2000000) != 0; */
    int NotANumber = (regs.fpsr & 0x1000000) != 0;

    switch (contition) {
    case 0x00:
	return 0;
    case 0x01:
	return Z;
    case 0x02:
	return !(NotANumber || Z || N);
    case 0x03:
	return Z || !(NotANumber || N);
    case 0x04:
	return N && !(NotANumber || Z);
    case 0x05:
	return Z || (N && !NotANumber);
    case 0x06:
	return !(NotANumber || Z);
    case 0x07:
	return !NotANumber;
    case 0x08:
	return NotANumber;
    case 0x09:
	return NotANumber || Z;
    case 0x0a:
	return NotANumber || !(N || Z);
    case 0x0b:
	return NotANumber || Z || !N;
    case 0x0c:
	return NotANumber || (N && !Z);
    case 0x0d:
	return NotANumber || Z || N;
    case 0x0e:
	return !Z;
    case 0x0f:
	return 1;
    case 0x10:
	return 0;
    case 0x11:
	return Z;
    case 0x12:
	return !(NotANumber || Z || N);
    case 0x13:
	return Z || !(NotANumber || N);
    case 0x14:
	return N && !(NotANumber || Z);
    case 0x15:
	return Z || (N && !NotANumber);
    case 0x16:
	return !(NotANumber || Z);
    case 0x17:
	return !NotANumber;
    case 0x18:
	return NotANumber;
    case 0x19:
	return NotANumber || Z;
    case 0x1a:
	return NotANumber || !(N || Z);
    case 0x1b:
	return NotANumber || Z || !N;
    case 0x1c:		/* NGE: NAN or (N and not Z), as ULT ($0C) -- PRM table 3-23 */
	return NotANumber || (N && !Z);
    case 0x1d:
	return NotANumber || Z || N;
    case 0x1e:
	return !Z;
    case 0x1f:
	return 1;
    }
    return -1;
}

void fdbcc_opp(uae_u32 opcode, uae_u16 extra)
{
    uaecptr pc = (uae_u32) m68k_getpc ();
    uae_s32 disp = (uae_s32) (uae_s16) next_iword();
    int cc;
    cc= fpp_cond(opcode, extra & 0x3f);

#if DEBUG_FPP
    upe_printf("fdbcc_opp (%08X %04X %d) at %08lx\n", opcode,extra & 0x3f,cc, m68k_getpc () );
    // fflush(stdout);
#endif

    if (cc == -1) {
	m68k_setpc (pc - 4);
	op_illg (opcode);
    } else if (!cc) {
	int reg = opcode & 0x7;

	m68k_dreg (regs, reg) = ((m68k_dreg (regs, reg) & ~0xffff)
				| ((m68k_dreg (regs, reg) - 1) & 0xffff));
	if ((m68k_dreg (regs, reg) & 0xffff) == 0xffff)
	    m68k_setpc (pc + disp);
    }
}

void fscc_opp(uae_u32 opcode, uae_u16 extra)
{
    uae_u32 ad;
    int cc;

#if DEBUG_FPP
    upe_printf("fscc_opp at %08lx\n", m68k_getpc ());
    // fflush(stdout);
#endif
    cc = fpp_cond(opcode, extra & 0x3f);
    if (cc == -1) {
	m68k_setpc (m68k_getpc () - 4);
	op_illg (opcode);
    } else if ((opcode & 0x38) == 0) {
	m68k_dreg (regs, opcode & 7) = (m68k_dreg (regs, opcode & 7) & ~0xff) |
	    (cc ? 0xff : 0x00);
    } else {
	/* (An)+ and -(An): get_fp_ad leaves the address register alone, so the
	   byte went to An itself and An never moved. A byte step is 1, 2 for
	   the stack pointer. */
	int     mode = (opcode >> 3) & 7, an = opcode & 7;
	uae_u32 step = (an == 7) ? 2 : 1;
	if (mode == 4) m68k_areg (regs, an) -= step;
	if (get_fp_ad(opcode, &ad) == 0) {
	    m68k_setpc (m68k_getpc () - 4);
	    op_illg (opcode);
	} else {
	    put_byte(ad, cc ? 0xff : 0x00);
	    if (mode == 3) m68k_areg (regs, an) += step;
	}
    }
}

void ftrapcc_opp(uae_u32 opcode, uaecptr oldpc)
{
    int cc;

#if DEBUG_FPP
    upe_printf("ftrapcc_opp at %08lx\n", m68k_getpc ());
    // fflush(stdout);
#endif
    cc = fpp_cond(opcode, opcode & 0x3f);
    if (cc == -1) {
	m68k_setpc (oldpc);
	op_illg (opcode);
	return;		/* illegal, and not a trap as well */
    }
    if (cc)
	Exception(7, oldpc - 2);
}

void fbcc_opp(uae_u32 opcode, uaecptr pc, uae_u32 extra)
{
    int cc;
    cc= fpp_cond(opcode, opcode & 0x3f); /* extra instead of opcode (bfo) !! */

#if DEBUG_FPP
    upe_printf("fbcc_opp (%04X %04X %d) at %08lx\n", opcode,opcode & 0x3f,cc, m68k_getpc () );
    // fflush(stdout);
#endif

    if (cc == -1) {
	m68k_setpc (pc);
	op_illg (opcode);
    } else if (cc) {
	if ((opcode & 0x40) == 0)
	    extra = (uae_s32) (uae_s16) extra;
	m68k_setpc (pc + extra);
    }
}

void fsave_opp(uae_u32 opcode)
{
    uae_u32 ad;
    int incr = (opcode & 0x38) == 0x20 ? -1 : 1;
    int i;

#if DEBUG_FPP
    upe_printf("fsave_opp at %08lx\n", m68k_getpc ());
    // fflush(stdout);
#endif
    if (get_fp_ad(opcode, &ad) == 0) {
	m68k_setpc (m68k_getpc () - 2);
	op_illg (opcode);
	return;
    }
    if (incr < 0) {
	ad -= 4;
	put_long (ad, 0x70000000);
	for (i = 0; i < 5; i++) {
	    ad -= 4;
	    put_long (ad, 0x00000000);
	}
	ad -= 4;
	put_long (ad, 0x1f180000);
    } else {
	put_long (ad, 0x1f180000);
	ad += 4;
	for (i = 0; i < 5; i++) {
	    put_long (ad, 0x00000000);
	    ad += 4;
	}
	put_long (ad, 0x70000000);
	ad += 4;
    }
    if ((opcode & 0x38) == 0x18)
	m68k_areg (regs, opcode & 7) = ad;
    if ((opcode & 0x38) == 0x20)
	m68k_areg (regs, opcode & 7) = ad;
}

void frestore_opp(uae_u32 opcode)
{
    uae_u32 ad;
    uae_u32 d;
    int incr = (opcode & 0x38) == 0x20 ? -1 : 1;

#if DEBUG_FPP
    upe_printf("frestore_opp at %08lx\n", m68k_getpc ());
    // fflush(stdout);
#endif
    if (get_fp_ad(opcode, &ad) == 0) {
	m68k_setpc (m68k_getpc () - 2);
	op_illg (opcode);
	return;
    }
    if (incr < 0) {
	ad -= 4;
	d = get_long (ad);
	if ((d & 0xff000000) != 0) {
	    if ((d & 0x00ff0000) == 0x00180000)
		ad -= 6 * 4;
	    else if ((d & 0x00ff0000) == 0x00380000)
		ad -= 14 * 4;
	    else if ((d & 0x00ff0000) == 0x00b40000)
		ad -= 45 * 4;
	}
    } else {
	d = get_long (ad);
	ad += 4;
	if ((d & 0xff000000) != 0) {
	    if ((d & 0x00ff0000) == 0x00180000)
		ad += 6 * 4;
	    else if ((d & 0x00ff0000) == 0x00380000)
		ad += 14 * 4;
	    else if ((d & 0x00ff0000) == 0x00b40000)
		ad += 45 * 4;
	}
    }
    if ((opcode & 0x38) == 0x18)
	m68k_areg (regs, opcode & 7) = ad;
    if ((opcode & 0x38) == 0x20)
	m68k_areg (regs, opcode & 7) = ad;
}

void fpp_opp(uae_u32 opcode, uae_u16 extra)
{
    int reg;
    double src;
    int svRound;	/* the host's rounding mode, while FPCR's is in force */

	#if DEBUG_FPP
	  char*  p;
	  unsigned short v;
	  double sav;
	  int mode, sreg;
	  char* r;
	  
	  v= (extra >> 13) & 0x7;
      upe_printf( "FPP %04lx %04x %1x at %08lx", 
             opcode & 0xffff, extra & 0xffff, v, m68k_getpc() - 4 );
    
      if (v!=0 &&
    	  v!=2 &&
    	  v!=3) upe_printf( "\n" );
    	
      // fflush(stdout);
	#endif

    switch ((extra >> 13) & 0x7) {
    case 3:
	if (put_fp_value (regs.fp[(extra >> 7) & 7], opcode, extra) == 0) {
	    m68k_setpc (m68k_getpc () - 4);
	    op_illg (opcode);
	}
	
	#if DEBUG_FPP
      if ((extra & 0x4000) == 0) {
		  sreg= (extra >> 10) & 7;
		  r= "fp";
	  }
	  else {
		  sreg=  opcode & 7;
		  mode= (opcode >> 3) & 7;
		  switch (mode) {
			case 0 : r= "d"; break;
			case 1 : r= "?"; break;
			case 2 : 
			case 3 : 
			case 4 : 
			case 5 : 
			case 6 : 
			case 7 : r= "a"; break;
			default: r= "?";
		  }
	  }
	
	  reg = (extra >> 7) & 7;
	  p= "FMOVE";
      upe_printf(" %-7s fp%d,%s%d  %f\n", p, reg, r,sreg,
                                      regs.fp[reg] );
      // fflush(stdout);
	#endif
	return;
	
    case 4:
    case 5:
	if ((opcode & 0x38) == 0) {
	    if (extra & 0x2000) {
		if (extra & 0x1000)
		    m68k_dreg (regs, opcode & 7) = regs.fpcr;
		if (extra & 0x0800)
		    m68k_dreg (regs, opcode & 7) = regs.fpsr;
		if (extra & 0x0400)
		    m68k_dreg (regs, opcode & 7) = regs.fpiar;
	    } else {
		if (extra & 0x1000)
		    regs.fpcr = m68k_dreg (regs, opcode & 7);
		if (extra & 0x0800)
		    regs.fpsr = m68k_dreg (regs, opcode & 7);
		if (extra & 0x0400)
		    regs.fpiar = m68k_dreg (regs, opcode & 7);
	    }
	/* == 8, not == 1: this tests the 68k addressing MODE field, bits 5..3, so
	 * (opcode & 0x38) is always a multiple of 8 and could never equal 1 -- the
	 * whole An-direct branch below was dead code. Mode 1 (An direct) therefore
	 * fell through to the memory-EA path below, which treated the address
	 * register as a memory address. Affects FMOVE.L of FPcr/FPsr/FPIAR to/from
	 * an address register (e.g. `fmove.l fpiar,a0`). */
	} else if ((opcode & 0x38) == 8) {
	    if (extra & 0x2000) {
		if (extra & 0x1000)
		    m68k_areg (regs, opcode & 7) = regs.fpcr;
		if (extra & 0x0800)
		    m68k_areg (regs, opcode & 7) = regs.fpsr;
		if (extra & 0x0400)
		    m68k_areg (regs, opcode & 7) = regs.fpiar;
	    } else {
		if (extra & 0x1000)
		    regs.fpcr = m68k_areg (regs, opcode & 7);
		if (extra & 0x0800)
		    regs.fpsr = m68k_areg (regs, opcode & 7);
		if (extra & 0x0400)
		    regs.fpiar = m68k_areg (regs, opcode & 7);
	    }
	} else if ((opcode & 0x3f) == 0x3c) {
	    if ((extra & 0x2000) == 0) {
		if (extra & 0x1000)
		    regs.fpcr = next_ilong();
		if (extra & 0x0800)
		    regs.fpsr = next_ilong();
		if (extra & 0x0400)
		    regs.fpiar = next_ilong();
	    }
	} else if (extra & 0x2000) {
	    /* FMOVEM FPP->memory */
	    uae_u32 ad;
	    int incr = 0;

	    if (get_fp_ad(opcode, &ad) == 0) {
		m68k_setpc (m68k_getpc () - 4);
		op_illg (opcode);
		return;
	    }
	    if ((opcode & 0x38) == 0x20) {
		if (extra & 0x1000)
		    incr += 4;
		if (extra & 0x0800)
		    incr += 4;
		if (extra & 0x0400)
		    incr += 4;
	    }
	    ad -= incr;
	    if (extra & 0x1000) {
		put_long (ad, regs.fpcr);
		ad += 4;
	    }
	    if (extra & 0x0800) {
		put_long (ad, regs.fpsr);
		ad += 4;
	    }
	    if (extra & 0x0400) {
		put_long (ad, regs.fpiar);
		ad += 4;
	    }
	    ad -= incr;
	    if ((opcode & 0x38) == 0x18)
		m68k_areg (regs, opcode & 7) = ad;
	    if ((opcode & 0x38) == 0x20)
		m68k_areg (regs, opcode & 7) = ad;
	} else {
	    /* FMOVEM memory->FPP */
	    uae_u32 ad;

	    if (get_fp_ad(opcode, &ad) == 0) {
		m68k_setpc (m68k_getpc () - 4);
		op_illg (opcode);
		return;
	    }
	    ad = (opcode & 0x38) == 0x20 ? ad - 12 : ad;
	    if (extra & 0x1000) {
		regs.fpcr = get_long (ad);
		ad += 4;
	    }
	    if (extra & 0x0800) {
		regs.fpsr = get_long (ad);
		ad += 4;
	    }
	    if (extra & 0x0400) {
		regs.fpiar = get_long (ad);
		ad += 4;
	    }
	    if ((opcode & 0x38) == 0x18)
		m68k_areg (regs, opcode & 7) = ad;
	    if ((opcode & 0x38) == 0x20)
		m68k_areg (regs, opcode & 7) = ad - 12;
	}
	return;
	
    case 6:
    case 7:
	{
	    uae_u32 ad, list = 0;
	    int incr = 0;
	    if (extra & 0x2000) {
		/* FMOVEM FPP->memory */
		if (get_fp_ad(opcode, &ad) == 0) {
		    m68k_setpc (m68k_getpc () - 4);
		    op_illg (opcode);
		    return;
		}
		switch ((extra >> 11) & 3) {
		case 0:	/* static pred */
		    list = extra & 0xff;
		    incr = -1;
		    break;
		case 1:	/* dynamic pred */
		    list = m68k_dreg (regs, (extra >> 4) & 3) & 0xff;
		    incr = -1;
		    break;
		case 2:	/* static postinc */
		    list = extra & 0xff;
		    incr = 1;
		    break;
		case 3:	/* dynamic postinc */
		    list = m68k_dreg (regs, (extra >> 4) & 3) & 0xff;
		    incr = 1;
		    break;
		}
		while (list) {
		    uae_u32 wrd1, wrd2, wrd3;
		    if (incr < 0) {
			from_exten(regs.fp[fpp_movem_index2[list]],
				   &wrd1, &wrd2, &wrd3);
			ad -= 4;
			put_long (ad, wrd3);
			ad -= 4;
			put_long (ad, wrd2);
			ad -= 4;
			put_long (ad, wrd1);
		    } else {
			from_exten(regs.fp[fpp_movem_index1[list]],
				   &wrd1, &wrd2, &wrd3);
			put_long (ad, wrd1);
			ad += 4;
			put_long (ad, wrd2);
			ad += 4;
			put_long (ad, wrd3);
			ad += 4;
		    }
		    list = fpp_movem_next[list];
		}
		if ((opcode & 0x38) == 0x18)
		    m68k_areg (regs, opcode & 7) = ad;
		if ((opcode & 0x38) == 0x20)
		    m68k_areg (regs, opcode & 7) = ad;
	    } else {
		/* FMOVEM memory->FPP */
		if (get_fp_ad(opcode, &ad) == 0) {
		    m68k_setpc (m68k_getpc () - 4);
		    op_illg (opcode);
		    return;
		}
		switch ((extra >> 11) & 3) {
		case 0:	/* static pred */
		    list = extra & 0xff;
		    incr = -1;
		    break;
		case 1:	/* dynamic pred */
		    list = m68k_dreg (regs, (extra >> 4) & 3) & 0xff;
		    incr = -1;
		    break;
		case 2:	/* static postinc */
		    list = extra & 0xff;
		    incr = 1;
		    break;
		case 3:	/* dynamic postinc */
		    list = m68k_dreg (regs, (extra >> 4) & 3) & 0xff;
		    incr = 1;
		    break;
		}
		while (list) {
		    uae_u32 wrd1, wrd2, wrd3;
		    if (incr < 0) {
			ad -= 4;
			wrd3 = get_long (ad);
			ad -= 4;
			wrd2 = get_long (ad);
			ad -= 4;
			wrd1 = get_long (ad);
			regs.fp[fpp_movem_index2[list]] = to_exten (wrd1, wrd2, wrd3);
		    } else {
			wrd1 = get_long (ad);
			ad += 4;
			wrd2 = get_long (ad);
			ad += 4;
			wrd3 = get_long (ad);
			ad += 4;
			regs.fp[fpp_movem_index1[list]] = to_exten (wrd1, wrd2, wrd3);
		    }
		    list = fpp_movem_next[list];
		}
		if ((opcode & 0x38) == 0x18)
		    m68k_areg (regs, opcode & 7) = ad;
		if ((opcode & 0x38) == 0x20)
		    m68k_areg (regs, opcode & 7) = ad;
	    }
	}
	return;
	
    case 0:
    case 2:
	reg = (extra >> 7) & 7;
	if ((extra & 0xfc00) == 0x5c00) {
	    switch (extra & 0x7f) {
	    case 0x00:
		regs.fp[reg] = 4.0 * atan(1.0);
		break;
	    case 0x0b:
		regs.fp[reg] = log10 (2.0);
		break;
	    case 0x0c:
		regs.fp[reg] = exp (1.0);
		break;
	    case 0x0d:
		regs.fp[reg] = log (exp (1.0)) / log (2.0);
		break;
	    case 0x0e:
		regs.fp[reg] = log (exp (1.0)) / log (10.0);
		break;
	    case 0x0f:
		regs.fp[reg] = 0.0;
		break;
	    case 0x30:
		regs.fp[reg] = log (2.0);
		break;
	    case 0x31:
		regs.fp[reg] = log (10.0);
		break;
	    case 0x32:
		regs.fp[reg] = 1.0e0;
		break;
	    case 0x33:
		regs.fp[reg] = 1.0e1;
		break;
	    case 0x34:
		regs.fp[reg] = 1.0e2;
		break;
	    case 0x35:
		regs.fp[reg] = 1.0e4;
		break;
	    case 0x36:
		regs.fp[reg] = 1.0e8;
		break;
	    case 0x37:
		regs.fp[reg] = 1.0e16;
		break;
	    case 0x38:
		regs.fp[reg] = 1.0e32;
		break;
	    case 0x39:
		regs.fp[reg] = 1.0e64;
		break;
	    case 0x3a:
		regs.fp[reg] = 1.0e128;
		break;
	    case 0x3b:
		regs.fp[reg] = 1.0e256;
		break;
#if 0
	    case 0x3c:
		regs.fp[reg] = 1.0e512;
		break;
	    case 0x3d:
		regs.fp[reg] = 1.0e1024;
		break;
	    case 0x3e:
		regs.fp[reg] = 1.0e2048;
		break;
	    case 0x3f:
		regs.fp[reg] = 1.0e4096;
		break;
#endif
	    default:
		m68k_setpc (m68k_getpc () - 4);
		op_illg (opcode);
		break;
	    }

		#if DEBUG_FPP
			upe_printf( " const %f\n", regs.fp[reg] );
		#endif
	    return;
	}
	if (get_fp_value (opcode, extra, &src) == 0) {
	    m68k_setpc (m68k_getpc () - 4);
	    op_illg (opcode);
	    
		#if DEBUG_FPP
			upe_printf( " ?\n" );
		#endif
	    return;
	}
		
	#if DEBUG_FPP
		sav= regs.fp[reg];
	#endif

	svRound = fpp_round_enter ();
	switch (extra & 0x7f) {
	case 0x00:		/* FMOVE */
	case 0x40:  /* Explicit rounding. This is just a quick fix. Same
		     * for all other cases that have three choices */
	case 0x44:   
	    regs.fp[reg] = src;
	    /* Brian King was here.  <ea> to register needs FPSR updated.
	     * See page 3-73 in Motorola 68K programmers reference manual.
	     * %%%FPU */
	    if ((extra & 0x44) == 0x40)
		regs.fp[reg] = (float)regs.fp[reg];
		
	//  MAKE_FPSR (regs.fp[reg]);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	/* FINT, FINTRZ, FMOD and FREM went through an (int): wrong for a
	   value past 2^31 (undefined behaviour, and a wrong answer on every
	   host), for inf and NaN, and -- (int) truncating toward zero -- for
	   rounding a negative value, FINT(-2.3) giving -1. The 68881 keeps
	   the result a float: FINT rounds "using the current rounding mode"
	   (FINT, M68000 PRM), FINTRZ toward zero; FMOD's quotient is rounded
	   toward zero, FREM's to nearest, which is C's fmod and remainder.
	   Arithmetic itself still rounds to nearest whatever FPCR says. */
	case 0x01:		/* FINT */
	    regs.fp[reg] = fpp_round (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x02:		/* FSINH */
	    regs.fp[reg] = sinh (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x03:		/* FINTRZ */
	    regs.fp[reg] = trunc (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x04:		/* FSQRT */
	    regs.fp[reg] = sqrt (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x06:		/* FLOGNP1 */
	    regs.fp[reg] = log (src + 1.0);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x08:		/* FETOXM1 */
	    regs.fp[reg] = exp (src) - 1.0;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x09:		/* FTANH */
	    regs.fp[reg] = tanh (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x0a:		/* FATAN */
	    regs.fp[reg] = atan (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x0c:		/* FASIN */
	    regs.fp[reg] = asin (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x0d:		/* FATANH */
#if 1				/* The BeBox doesn't have atanh, and it isn't in the HPUX libm either */
	    regs.fp[reg] = log ((1 + src) / (1 - src)) / 2;
#else
	    regs.fp[reg] = atanh (src);
#endif
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x0e:		/* FSIN */
	    regs.fp[reg] = sin (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x0f:		/* FTAN */
	    regs.fp[reg] = tan (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x10:		/* FETOX */
	    regs.fp[reg] = exp (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x11:		/* FTWOTOX */
	    regs.fp[reg] = pow(2.0, src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x12:		/* FTENTOX */
	    regs.fp[reg] = pow(10.0, src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x14:		/* FLOGN */
	    regs.fp[reg] = log (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x15:		/* FLOG10 */
	    regs.fp[reg] = log10 (src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x16:		/* FLOG2 */
	    regs.fp[reg] = log (src) / log (2.0);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x18:		/* FABS */
	    regs.fp[reg] = src < 0 ? -src : src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x19:		/* FCOSH */
	    regs.fp[reg] = cosh(src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x1a:		/* FNEG */
	    regs.fp[reg] = -src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x1c:		/* FACOS */
	    regs.fp[reg] = acos(src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x1d:		/* FCOS */
	    regs.fp[reg] = cos(src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x1e:		/* FGETEXP */
	    {
		/* zero answers itself, an infinity a NaN (FGETEXP, M68000 PRM);
		   frexp gave -1 for zero and garbage for infinity */
		int expon;
		if (src == 0.0 || isnan (src)) regs.fp[reg] = src;
		else if (isinf (src))          regs.fp[reg] = NAN;
		else { frexp (src, &expon); regs.fp[reg] = (double) (expon - 1); }
		regs.fpsr = fpsr_cc (regs.fp[reg]);
	    }
	    break;
	case 0x1f:		/* FGETMAN */
	    {
		int expon;
		/* as FGETEXP: zero answers itself, an infinity a NaN (PRM) */
		if (src == 0.0 || isnan (src)) regs.fp[reg] = src;
		else if (isinf (src))          regs.fp[reg] = NAN;
		else regs.fp[reg] = frexp (src, &expon) * 2.0;
		regs.fpsr = fpsr_cc (regs.fp[reg]);
	    }
	    break;
	case 0x20:		/* FDIV */
	    regs.fp[reg] /= src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x21:		/* FMOD */
	    fpsr_quotient (regs.fp[reg], src, 0);
	    regs.fp[reg] = fmod (regs.fp[reg], src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x22:		/* FADD */
	    regs.fp[reg] += src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x23:		/* FMUL */
	    regs.fp[reg] *= src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x24:		/* FSGLDIV */
	    regs.fp[reg] /= src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x25:		/* FREM */
	    fpsr_quotient (regs.fp[reg], src, 1);
	    regs.fp[reg] = remainder (regs.fp[reg], src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x26:		/* FSCALE */
	    /* adds the source, chopped toward zero, to the exponent (FSCALE,
	       M68000 PRM): exact, where exp(log(2)*n) was not -- FSCALE #3 of
	       1.0 gave 7.999999999999998. An infinite scale is a NaN; past
	       2^14 it always over- or underflows, so a clamp loses nothing. */
	    if (isnan (src) || isinf (src)) regs.fp[reg] = NAN;
	    else {
		double n = trunc (src);
		if (n >  20000.0) n =  20000.0;
		if (n < -20000.0) n = -20000.0;
		regs.fp[reg] = ldexp (regs.fp[reg], (int) n);
	    }
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x27:		/* FSGLMUL */
	    regs.fp[reg] *= src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x28:		/* FSUB */
	    regs.fp[reg] -= src;
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x30:		/* FSINCOS */
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	    regs.fp[reg] = sin (src);
	    regs.fp[extra & 7] = cos(src);
	    regs.fpsr = fpsr_cc (regs.fp[reg]);
	    break;
	case 0x38:		/* FCMP */
	    /* From the comparison, not from the difference: +inf - +inf is a
	       NaN where the operands are equal, and two finite values whose
	       difference overflows are not an infinity. */
	    {
		double dst = regs.fp[reg];
		if (isnan (dst) || isnan (src))
		    regs.fpsr = fpsr_keep (0x1000000);
		else if (dst == src)
		    regs.fpsr = fpsr_keep (0x4000000 | (signbit (dst) && signbit (src) ? 0x8000000 : 0));
		else /* "The infinity bit is always cleared by the FCMP instruction" */
		    regs.fpsr = fpsr_keep (dst < src ? 0x8000000 : 0);
	    }
	    break;
	case 0x3a:		/* FTST */
	    regs.fpsr = fpsr_cc (src);
	    break;
	default:
	    m68k_setpc (m68k_getpc () - 4);
	    op_illg (opcode);
	    break;
	}
	fpp_round_leave (svRound);
	
	#if DEBUG_FPP
	switch (extra & 0x7f) {
		case 0x00: p= "FMOVE"; 	 break;
		case 0x01: p= "FINT"; 	 break;
		case 0x02: p= "FSINH"; 	 break;
		case 0x03: p= "FINTRZ";  break;
		case 0x04: p= "FSQRT"; 	 break;
		case 0x06: p= "FLOGNP1"; break;
		case 0x08: p= "FETOXM1"; break;
		case 0x09: p= "FTANH"; 	 break;
		case 0x0a: p= "FATAN"; 	 break;
		case 0x0c: p= "FASIN"; 	 break;
		case 0x0d: p= "FATANH";  break;
		case 0x0e: p= "FSIN"; 	 break;
		case 0x0f: p= "FTAN"; 	 break;
		case 0x10: p= "FETOX"; 	 break;
		case 0x11: p= "FTWOTOX"; break;
		case 0x12: p= "FTENTOX"; break;
		case 0x14: p= "FLOGN"; 	 break;
		case 0x15: p= "FLOG10";  break;
		case 0x16: p= "FLOG2"; 	 break;
		case 0x18: p= "FABS"; 	 break;
		case 0x19: p= "FCOSH"; 	 break;
		case 0x1a: p= "FNEG"; 	 break;
		case 0x1c: p= "FACOS"; 	 break;
		case 0x1d: p= "FCOS"; 	 break;
		case 0x1e: p= "FGETEXP"; break;
		case 0x1f: p= "FGETMAN"; break;
		case 0x20: p= "FDIV"; 	 break;
		case 0x21: p= "FMOD"; 	 break;
		case 0x22: p= "FADD"; 	 break;
		case 0x23: p= "FMUL"; 	 break;
		case 0x24: p= "FSGLDIV"; break;
		case 0x25: p= "FREM"; 	 break;
		case 0x26: p= "FSCALE";  break;
		case 0x27: p= "FSGLMUL"; break;
		case 0x28: p= "FSUB"; 	 break;
		case 0x30: p= "FSINCOS";
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37: 				 break;
		case 0x38: p= "FCMP"; 	 break;
		case 0x3a: p= "FTST"; 	 break;
		default:   p= "????";
	}
	
    if ((extra & 0x4000) == 0) {
		sreg= (extra >> 10) & 7;
		r= "fp";
	}
	else {
		sreg=  opcode & 7;
		mode= (opcode >> 3) & 7;
		switch (mode) {
			case 0 : r= "d"; break;
			case 1 : r= "?"; break;
			case 2 : 
			case 3 : 
			case 4 : 
			case 5 : 
			case 6 : 
			case 7 : r= "a"; break;
			default: r= "?";
		}
	}
	
    upe_printf(" %-7s %s%d,fp%d  %f %f (%f)\n", p, r,sreg, reg,
                                            src,regs.fp[reg], sav );
    // fflush(stdout);
	#endif

	return;
    }
    m68k_setpc (m68k_getpc () - 4);
    op_illg (opcode);
}

#endif

