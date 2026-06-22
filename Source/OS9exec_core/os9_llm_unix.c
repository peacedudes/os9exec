/*
 * os9_llm_unix.c
 * Low-level magic stub for UNIX (macOS arm64 / Linux).
 * The Classic Mac OS os9_llm.c is replaced by this file on UNIX targets.
 * The actual 68k execution engine will replace llm_os9_go() once the
 * 64-bit alignment issues are resolved.
 */

#include "os9exec_incl.h"

ulong llm_os9_go(regs_type *rp)
{
    /* placeholder — real 68k execution goes here */
    return 0;
}

void llm_os9_copyback(regs_type *rp)
{
    /* no-op on UNIX: register copyback for bus-error reporting */
}

Boolean llm_fpu_present(void)     { return false; }
Boolean llm_vm_enabled(void)      { return false; }
Boolean llm_has_cache(void)       { return false; }
Boolean llm_runs_in_usermode(void){ return false; }

void Flush68kCodeRange(void *address, ulong size) { (void)address; (void)size; }
void LockMemRange     (void *address, ulong size) { (void)address; (void)size; }
void UnlockMemRange   (void *address, ulong size) { (void)address; (void)size; }

OSErr lowlevel_prepare(void)  { return 0; }
void  lowlevel_release(void)  {}
