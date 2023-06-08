#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_ov(void);

void (*exception_handlers[32])(void) = {
    [0 ... 31] = handle_reserved,
    [0] = handle_int,
    [2 ... 3] = handle_tlb,
    [12] = handle_ov,
#if !defined(LAB) || LAB >= 4
    [1] = handle_mod,
    [8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf) {
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_ov(struct Trapframe *tf)
{
	curenv->env_ov_cnt++;
	u_int va = tf->cp0_epc;
	Pte *pte;
	struct Page *pp = page_lookup(curenv->env_pgdir, va, &pte);
	// int *kva = page2kva(pp);
	// int order = *kva;
	// pgdir_walk(curenv->env_pgdir, va, &pte);
	u_int pa = PTE_ADDR(*pte) + (va & 0xfff); 
	int *kva = KADDR(pa);
	int order = *kva;
	// printk("%d\n", *kva);
	if (order & (1 << 29))
	{
		printk("addi ov handled\n");
		int t = ((order & (0x1f << 16)) >> 16);
		int s = ((order & (0x1f << 21)) >> 21);
		int imm = (order & 0xffff);
		tf->regs[t] = (tf->regs[s] / 2 + imm / 2);
		tf->cp0_epc += 4;
	}
	else
	{
		if (order & (1 << 1))
		{
			printk("sub ov handled\n");
			(*kva) |= 1;
		}
		else
		{
			printk("add ov handled\n");
			(*kva) |= 1;
		}
	}	
}

