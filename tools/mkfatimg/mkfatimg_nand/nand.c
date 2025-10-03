#include <windows.h>
#include "nand.h"
#include <stdbool.h>


//#define BLK_SIZE_IN_BYTES   (128*1024)
//#define PAGE_SIZE_IN_BYTES  (2048)



// public function definitions
int dhara_nand_is_bad(const struct dhara_nand *n, dhara_block_t b)
{
	return 0;
}

void dhara_nand_mark_bad(const struct dhara_nand *n, dhara_block_t b)
{
	return;
}

int dhara_nand_erase(const struct dhara_nand *n, dhara_block_t b, dhara_error_t *err)
{
    BYTE *disk;
	uint32_t block_size = (1 << (n->log2_page_size + n->log2_ppb));

    disk = (BYTE *)n->user_data;

	FillMemory(disk + b * block_size, block_size, 0xFF);

	//printf("erase:%d\n", b);

	return 0;
}

int dhara_nand_prog(const struct dhara_nand *n, dhara_page_t p, const uint8_t *data,
                    dhara_error_t *err)
{
	BYTE *disk;
	uint32_t page_size;

	disk = (BYTE *)n->user_data;

	page_size = (1 << n->log2_page_size);
	CopyMemory(disk + p * page_size, data, page_size);

	//printf("prog:%d\n", p);

	return 0;
}

int dhara_nand_is_free(const struct dhara_nand *n, dhara_page_t p)
{
	BYTE *disk;
	bool is_free;
	uint32_t page_size;

	printf("is_free:%d\n", p);

	disk = (BYTE *)n->user_data;
	page_size = (1 << n->log2_page_size);
	disk += p * page_size;
    is_free = true;
    for (uint32_t i = 0; i < page_size; i++)
    {
        if (disk[i] != 0xFF)
        {
            is_free = false;
            break;
        }
    }
	//printf("is_free:%d,%d\n", p, is_free);

    return is_free;
}

int dhara_nand_read(const struct dhara_nand *n, dhara_page_t p, size_t offset, size_t length,
                    uint8_t *data, dhara_error_t *err)
{
	BYTE *disk;
	uint32_t page_size;

	disk = (BYTE *)n->user_data;
	page_size = (1 << n->log2_page_size);

	CopyMemory(data, disk + p * page_size + offset, length);

	//printf("read:%d,%d,%d\n", p, offset, length);

    return 0;
}

/* Read a page from one location and reprogram it in another location.
 * This might be done using the chip's internal buffers, but it must use
 * ECC.
 */
int dhara_nand_copy(const struct dhara_nand *n, dhara_page_t src, dhara_page_t dst,
	dhara_error_t *err)
{
	BYTE *disk;
	uint32_t page_size;

	disk = (BYTE *)n->user_data;
	page_size = (1 << n->log2_page_size);

	CopyMemory(disk + dst * page_size, disk + src * page_size, page_size);

	//printf("copy\n");

	return 0;
}
