#ifndef __SECBOOT_H__
#define __SECBOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* load app image and jump to app */
void dfu_boot_img_in_flash(int flashid);

void boot_set_flash_read_func(flash_read_func read_func);

/**
 * @brief  precheck fucntion before jump to app
 *
 * It's called by `dfu_boot_img_in_flash` and can be used to perform user-defined check before loading app image
 * It's a weak function that can be overridden by user. User can perform signature verification in this function.
 * If verification fails, need to while loop in this function to avoid jumping to app.
 *
 * Reference code is provided in secboot.c
 *
 */
void boot_precheck(void);

/** Get RSA-2028 public key.
 *
 * It's a weak function that can be overridden by user. The returned RAS public key is used by RSA decryption
 *
 * @return RSA public key
 */
uint8_t *sifli_get_sig_pub_key(void);

/**
 * @brief  Get UID
 *
 * @param[out] uid buffer used to save UID
 * @param[in]  len uid buffer length in byte, the length must be >= DFU_UID_SIZE, i.e. 16 bytes
 *
 * @retval 0: sucess, others: fail
 */
int32_t boot_get_uid(uint8_t *uid, uint32_t len);


/**
 * @brief  Read RSA-2048 signature
 *
 * It's a weak function that can be overridden by user. By default it reads signature from address specified by macro BOOT_SIG_REGION_START_ADDR
 *
 * @param[out] sig   buffer used to save signature
 * @param[in]  len   signature buffer length in byte, the length must be >= 256 bytes
 *
 * @retval 0: sucess, others: fail
 */
int32_t boot_read_sig(uint8_t *sig, uint32_t len);

/**
 * @brief  Calculate SHA256 hash value
 *
 *  If uid is not NULL, the uid will be appended to the end of input data and used to calulate hash value.
 *
 * @param[in] in         input data buffer
 * @param[in] in_size    input buffer size in byte
 * @param[in] uid        uid buffer, if NULL, uid will not be used
 * @param[in] uid_size   input buffer size in byte
 * @param[in] out        output hash buffer
 * @param[in] out_size   output hash buffer size in byte, must be >= 32 bytes
 *
 * @retval 0: sucess, others: fail
*/

int32_t boot_sha256_calculate(uint8_t *in, uint32_t in_size, uint8_t *uid, uint32_t uid_size, uint8_t *out, uint32_t out_size);


#ifdef __cplusplus
}
#endif


#endif /* __SECBOOT_H__ */

