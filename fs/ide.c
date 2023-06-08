/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

#define ssd_diskno 0

u_int ssd_list[32];
u_int ssd_bitmap[32];
u_int ssd_times[32];
char *space[512];

void ssd_init()
{

}

int ssd_read(u_int logic_no, void *dst)
{
	for (int i = 0; i < 32; i++)
	{
		if (ssd_bitmap[i] && ssd_list[i] == logic_no)
		{
			ide_read(ssd_diskno, i, dst, 1);	
			return 0;
		}
	}
	return -1;
}

void ssd_write(u_int logic_no, void *src)
{
	for (int i = 0; i < 32; i++)
	{
        	if (ssd_bitmap[i] && ssd_list[i] == logic_no)
                {
                	ide_write(ssd_diskno, i, space, 1);
			ssd_list[i] = 0;
			ssd_bitmap[i] = 0;
			ssd_times[i] += 1;
			break;
        	}
 	}

	u_int min = 100000000;
	u_int min_phy_no = 0;
	for (int i = 0; i < 32; i++)
	{
		if (ssd_bitmap[i])
		{
			continue;
		}
		if (min > ssd_times[i])
		{
			min = ssd_times[i];
			min_phy_no = i;
		}
	}

	if (min < 5)
	{
		ssd_list[min_phy_no] = logic_no;
		ssd_bitmap[min_phy_no] = 1;
		ide_write(ssd_diskno, min_phy_no, src, 1);
	}
	else
	{
		u_int _min = 100000000;
		u_int _min_phy_no = 0;
		for (int i = 0; i < 32; i++)
		{
			if (!ssd_bitmap[i])
			{
				continue;
			}
			if (_min > ssd_times[i])
			{
				_min = ssd_times[i];
				_min_phy_no = i;
			}
		}

		char *tmp[512];
		ide_read(ssd_diskno, _min_phy_no, tmp, 1);
		ide_write(ssd_diskno, min_phy_no, tmp, 1);

		ssd_list[min_phy_no] = ssd_list[_min_phy_no];
		ssd_bitmap[min_phy_no] = 1;

		ssd_list[_min_phy_no] = 0;
		ssd_bitmap[_min_phy_no] = 0;
		ssd_times[_min_phy_no] += 1;
		ide_write(ssd_diskno, _min_phy_no, space, 1);

		ssd_list[_min_phy_no] = logic_no;
		ssd_bitmap[_min_phy_no] = 1;
		ide_write(ssd_diskno, _min_phy_no, src, 1);
	}
}

void ssd_erase(u_int logic_no)
{
	for (int i = 0; i < 32; i++)
	{
		if (ssd_bitmap[i] && ssd_list[i] == logic_no)
		{
			ide_write(ssd_diskno, i, space, 1);		
			ssd_list[i] = 0;
			ssd_bitmap[i] = 0;
			ssd_times[i] += 1;
			return;
		}
	}
}

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (1/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, 4));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4));
		temp = DEV_DISK_OPERATION_READ;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, 4));
		temp = 0;
		panic_on(syscall_read_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_STATUS, 4));
		if (!temp)
		{
			user_panic("ide read fail!");
		}
		panic_on(syscall_read_dev(dst + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, 512));
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		/* Exercise 5.3: Your code here. (2/2) */
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_ID, 4));
		temp = begin + off;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_OFFSET, 4));
		panic_on(syscall_write_dev(src + off, DEV_DISK_ADDRESS + DEV_DISK_BUFFER, 512));
		temp = DEV_DISK_OPERATION_WRITE;
		panic_on(syscall_write_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_START_OPERATION, 4));
		temp = 0;
		panic_on(syscall_read_dev(&temp, DEV_DISK_ADDRESS + DEV_DISK_STATUS, 4));
		if (!temp)
		{
			user_panic("ide write fail!");
		}	
	}
}
