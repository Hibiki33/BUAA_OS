// User-level IPC library routines

#include <env.h>
#include <lib.h>
#include <mmu.h>

#define RTC 0x15000000
#define refresh 0x0000
#define reads 0x0010
#define readus 0x0020

u_int get_time(u_int *us)
{
	u_int temp = 1;
	u_int rs;
	u_int rus;
	panic_on(syscall_write_dev(&temp, RTC + refresh, 4));
	panic_on(syscall_read_dev(&rs, RTC + reads, 4));
	panic_on(syscall_read_dev(&rus, RTC +readus, 4));
	*us = rus;
	return rs;
}

void usleep(u_int us)
{
	u_int enter_us;
	u_int enter_s = get_time(&enter_us);
	while (1)
	{
		u_int present_us;
		u_int present_s = get_time(&present_us);
		int delta_time = (int)present_s - (int)enter_s;
		delta_time = delta_time * 1000000 + (int)present_us - (int)enter_us;
		if (delta_time >= us)
		{
			return;
		}
		else
		{
			syscall_yield();
		}
	}
}

// Send val to whom.  This function keeps trying until
// it succeeds.  It should panic() on any error other than
// -E_IPC_NOT_RECV.
//
// Hint: use syscall_yield() to be CPU-friendly.
void ipc_send(u_int whom, u_int val, const void *srcva, u_int perm) {
	int r;
	while ((r = syscall_ipc_try_send(whom, val, srcva, perm)) == -E_IPC_NOT_RECV) {
		syscall_yield();
	}
	user_assert(r == 0);
}

// Receive a value.  Return the value and store the caller's envid
// in *whom.
//
// Hint: use env to discover the value and who sent it.
u_int ipc_recv(u_int *whom, void *dstva, u_int *perm) {
	int r = syscall_ipc_recv(dstva);
	if (r != 0) {
		user_panic("syscall_ipc_recv err: %d", r);
	}

	if (whom) {
		*whom = env->env_ipc_from;
	}

	if (perm) {
		*perm = env->env_ipc_perm;
	}

	return env->env_ipc_value;
}
