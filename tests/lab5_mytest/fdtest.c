#include <lib.h>

int main()
{
	int r;
	int fdnum;
	char buf[512];
	int n;

	if ((r = open("/newmotd", O_RDWR)) < 0)
	{
		user_panic("open /newmotd: %d\n", r);
	}
	debugf("open /newmotd is good\n");
	fdnum = r;

	if ((r = read(fdnum, buf, 10)) < 0)
	{
		user_panic("read /newmotd: %d\n", r);
	}
	debugf("read /newmotd is good\n");
	debugf("read buf is %s\n", buf);

	debugf("before fork, fd is %d\n", fdnum);
	n = fork();
	if (n == 0)
	{
		char sonbuf[512];
		if ((r = read(fdnum, sonbuf, 10)) < 0)
		{
			user_panic("son's read /newmotd: %d\n", r);
		}
		fdnum = r;
		debugf("son's read /newmotd is good\n");
		debugf("son's read buf is %s\n", sonbuf);
		debugf("after son read, fd is %d\n", fdnum);
	}
	else
	{	
		char fatherbuf[512];
		if ((r = read(fdnum, fatherbuf, 10)) < 0)
		{
			user_panic("father's read /newmotd: %d\n", r);
		}
		fdnum = r;
		debugf("father's read /newmotd is good\n");
		debugf("father's read buf is %s\n", fatherbuf);
		debugf("after father read, fd is %d\n", fdnum);
	}
	
	return 0;
}
