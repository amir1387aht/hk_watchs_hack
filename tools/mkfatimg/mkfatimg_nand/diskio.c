/*-----------------------------------------------------------------------*/
/* RAM disk control module for Win32              (C)ChaN, 2014          */
/*-----------------------------------------------------------------------*/

#include <windows.h>
#include <assert.h>
#include "stdio.h"
#include "diskio.h"
#include "ff.h"
#include "map.h"

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/


extern BYTE *RamDisk;		/* Poiter to the active RAM disk (main.c) */
extern DWORD RamDiskSize;	/* Size of RAM disk in unit of sector */
extern int gc_ratio;
extern int block_size;

static struct dhara_map map;
static BYTE page_buffer[8192];
static struct dhara_nand nand = {
	.log2_page_size = 11,
	.log2_ppb = 6,
	.num_blocks = 16,
	.user_data = 0
};



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv		/* Physical drive nmuber */
)
{
	if (pdrv) return STA_NOINIT;

	int ret;
	int page_num_per_block;

	if (!RamDisk) {
		RamDisk = VirtualAlloc(0, RamDiskSize * FF_MIN_SS, MEM_COMMIT, PAGE_READWRITE);

		FillMemory(RamDisk, RamDiskSize * FF_MIN_SS, 0xFF);

		nand.user_data = (void *)RamDisk;

		page_num_per_block = block_size / FF_MIN_SS;

		if (512 == FF_MIN_SS)
		{
			nand.log2_page_size = 9;
		}
		else if (1024 == FF_MIN_SS)
		{
			nand.log2_page_size = 10;
		}
		else if (2048 == FF_MIN_SS)
		{
			nand.log2_page_size = 11;
		}
		else if (4096 == FF_MIN_SS)
		{
			nand.log2_page_size = 12;
		}
		else if (8192 == FF_MIN_SS)
		{
			nand.log2_page_size = 13;
		}
		else
		{
			printf("Invalid page size:%d\n", FF_MIN_SS);
			assert(0);
		}

		if (64 == page_num_per_block)
		{
			nand.log2_ppb = 6;
		}
		else if (32 == page_num_per_block)
		{
			nand.log2_ppb = 5;
		}
		else if (16 == page_num_per_block)
		{
			nand.log2_ppb = 4;
		}
		else
		{
			printf("Invalid ppb:%d,%d\n", page_num_per_block, block_size);
			assert(0);
		}

		nand.num_blocks = RamDiskSize * FF_MIN_SS / block_size;

		assert(FF_MIN_SS <= 8192);

		// init flash translation layer
		dhara_map_init(&map, &nand, page_buffer, gc_ratio);
		dhara_error_t err = DHARA_E_NONE;
		ret = dhara_map_resume(&map, &err);
		printf("init\n");
	}

    return 0;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0) */
)
{
	if (pdrv) return STA_NOINIT;

	return RamDisk ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to read */
)
{
	dhara_error_t err;
	uint32_t sector_size = (1 << nand.log2_page_size);

	//printf("disk_read:%d,%d\n", sector, count);

	// read *count* consecutive sectors
	for (int i = 0; i < count; i++) {
		int ret = dhara_map_read(&map, sector, buff, &err);
		if (ret) {
			printf("dhara read failed: %d, error: %d\n", ret, err);
			return RES_ERROR;
		}
		buff += sector_size; // sector size == page size
		sector++;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	UINT count			/* Number of sectors to write */
)
{
	dhara_error_t err;
	int ret;
	uint32_t sector_size = (1 << nand.log2_page_size);

	//printf("disk_write:%d,%d\n", sector, count);


	// write *count* consecutive sectors
	for (int i = 0; i < count; i++)
	{
		ret = dhara_map_write(&map, sector, buff, &err);
		if (ret)
		{
			printf("dhara write failed: %d, error: %d\n", ret, err);
			return RES_ERROR;
		}
		buff += sector_size; // sector size == page size
		sector++;
	}

	//ret = dhara_map_sync(&map, &err);
	//if (ret)
	{
	//	printf("dhara write failed: %d, error: %d\n", ret, err);
	//	return RES_ERROR;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void* buff		/* Buffer to send/receive data block */
)
{
	DRESULT dr;
	dhara_error_t err;
	int ret;

    //printf("ioctrl:%d\n", ctrl);

	dr = RES_ERROR;
	if (!pdrv && RamDisk) {
		switch (ctrl) {
		case CTRL_SYNC:
			ret = dhara_map_sync(&map, &err);
			if (ret)
			{
				printf("dhara sync failed: %d, error: %d\n", ret, err);
				dr = RES_ERROR;
			}
			else
			{
				dr = RES_OK;
			}
			break;

		case GET_SECTOR_COUNT:
		{
			dhara_sector_t sector_count = dhara_map_capacity(&map);

            printf("sec_count:%d\n", sector_count);

			*(DWORD*)buff = sector_count;
			dr = RES_OK;
			break;
		}
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1;
			dr = RES_OK;
			break;
		case GET_SECTOR_SIZE:
			*(DWORD*)buff = (1 << nand.log2_page_size);;
			dr = RES_OK;
			break;
		case FS_CLEAN_GARBAGE:
			ret = dhara_map_gc_all(&map, &err);
			if (ret)
			{
				printf("gc error\n");
				dr = RES_ERROR;
			}
			else
			{
				printf("map:%d,%d\n", dhara_journal_size(&map.journal), map.count);
				dr = RES_OK;
			}
			break;
		}

	}
	return dr;
}


