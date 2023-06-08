#include <env.h>
#include <pmap.h>
#include <printk.h>

/* Overview:
 *   Implement a round-robin scheduling to select a runnable env and schedule it using 'env_run'.
 *
 * Post-Condition:
 *   If 'yield' is set (non-zero), 'curenv' should not be scheduled again unless it is the only
 *   runnable env.
 *
 * Hints:
 *   1. The variable 'count' used for counting slices should be defined as 'static'.
 *   2. Use variable 'env_sched_list', which contains and only contains all runnable envs.
 *   3. You shouldn't use any 'return' statement because this function is 'noreturn'.
 */
void schedule(int yield) {
	static int count = 0; // remaining time slices of current env
	struct Env *e = curenv;
	static int user_time[5];

	int users[5] = {0, 0, 0, 0, 0};
	struct Env *var;
	TAILQ_FOREACH(var, &env_sched_list, env_sched_link) 
	{
		if (var->env_status == ENV_RUNNABLE)
		{
			users[var->env_user] = 1;
		}
	}

	if (yield || count <= 0 || e == NULL || e->env_status != ENV_RUNNABLE)
	{
		if (e != NULL)
		{
			TAILQ_REMOVE(&env_sched_list, e, env_sched_link);
			if (e->env_status == ENV_RUNNABLE)
			{
				TAILQ_INSERT_TAIL(&env_sched_list, e, env_sched_link);
				user_time[e->env_user] += e->env_pri;
			}
		}
		if (TAILQ_EMPTY(&env_sched_list))
		{
			panic("no runnable envs");
		}
		// e = TAILQ_FIRST(&env_sched_list);
		int user = -1;
		int min = 100000000;
		for (int i = 0; i < 5; i++)
		{
			if (users[i] == 0)
			{
				continue;
			}
			if (min > user_time[i])
			{
				min = user_time[i];
				user = i;
			}
		}	

		TAILQ_FOREACH(var, &env_sched_list, env_sched_link)
		{
			if (var->env_user == user)
			{
				e = var;
				break;
			}
		}
		count = e->env_pri;
	}
	count--;
	env_run(e);
}
