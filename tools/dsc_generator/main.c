#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "easytlv.h"

/** Application Name */
#define APP_DB_KEY_APP_NAME         6
/** Application Resource Installer Name */
#define APP_DB_KEY_RES_PACKAGE_NAME 7
/** Application Program Installer Name */
#define APP_DB_KEY_PGM_PACKAGE_NAME  8
/** Application Thumbnail Image File Name */
#define APP_DB_KEY_THUMBNAIL_FILE_NAME  9
/** Application Title */
#define APP_DB_KEY_APP_TITLE 10

#define CREATE_TAG(key)             (key)
#define TAG_ITEM_LEN(val_len)       ((val_len) + 8) 


#define APP_DSC_FILE_SUFFIX  ".dsc"

int main(int argc, char *argv[])
{
	const char *app_name;
	const char *pgm_package_name;
	const char *res_package_name;
	const char *thumbnail_file_name;
	const char *app_title;
	const char *output_file;
	FILE  *fd;
	ETLVToken token[5];
	uint8_t info_num;
	int buf_len;
	uint8_t *buf = NULL;
	int wr_len;
	int r;

	if (argc < 7)
	{
		printf("invalid arg\n");
		return 1;
	}

	r = 0;
	app_name = argv[1];
	app_title = argv[2];
	pgm_package_name = argv[3];
    thumbnail_file_name = argv[4];
	res_package_name = argv[5];
	output_file = argv[6];

	token[0].tag = CREATE_TAG(APP_DB_KEY_APP_NAME);
	token[0].len = strlen(app_name) + 1;
	token[0].val = app_name;
	buf_len = TAG_ITEM_LEN(token[0].len);


	token[1].tag = CREATE_TAG(APP_DB_KEY_APP_TITLE);
	token[1].len = strlen(app_title) + 1;
	token[1].val = app_title;
	buf_len += TAG_ITEM_LEN(token[1].len);

	token[2].tag = CREATE_TAG(APP_DB_KEY_PGM_PACKAGE_NAME);
	token[2].len = strlen(pgm_package_name) + 1;
	token[2].val = pgm_package_name;
	buf_len += TAG_ITEM_LEN(token[2].len);

	token[3].tag = CREATE_TAG(APP_DB_KEY_THUMBNAIL_FILE_NAME);
	token[3].len = strlen(thumbnail_file_name) + 1;
	token[3].val = thumbnail_file_name;
	buf_len += TAG_ITEM_LEN(token[3].len);

	token[4].tag = CREATE_TAG(APP_DB_KEY_RES_PACKAGE_NAME);
	token[4].len = strlen(res_package_name) + 1;
	token[4].val = res_package_name;
	buf_len += TAG_ITEM_LEN(token[4].len);

	info_num = 5;
	buf = malloc(buf_len);
	wr_len = etlv_serialize(buf, &buf_len, token, info_num);
	if (wr_len < 0)
	{
		r = -1;
		goto __EXIT;
	}

	fd = fopen(output_file, "wb");
	if (fd)
	{
		buf_len = fwrite(buf, sizeof(buf[0]), wr_len, fd);
		fclose(fd);
		if (buf_len != wr_len)
		{
			printf("write fail\n");
		}
	}

__EXIT:
	if (buf)
	{
		free(buf);
	}

	return r;
}


