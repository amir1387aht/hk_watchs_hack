/*------------------------------------------------------------------------------
--       Copyright (c) 2015-2017, VeriSilicon Inc. All rights reserved        --
--         Copyright (c) 2011-2014, Google Inc. All rights reserved.          --
--         Copyright (c) 2007-2010, Hantro OY. All rights reserved.           --
--                                                                            --
-- This software is confidential and proprietary and may be used only as      --
--   expressly authorized by VeriSilicon in a written licensing agreement.    --
--                                                                            --
--         This entire notice must be reproduced on all copies                --
--                       and may not be removed.                              --
--                                                                            --
--------------------------------------------------------------------------------
-- Redistribution and use in source and binary forms, with or without         --
-- modification, are permitted provided that the following conditions are met:--
--   * Redistributions of source code must retain the above copyright notice, --
--       this list of conditions and the following disclaimer.                --
--   * Redistributions in binary form must reproduce the above copyright      --
--       notice, this list of conditions and the following disclaimer in the  --
--       documentation and/or other materials provided with the distribution. --
--   * Neither the names of Google nor the names of its contributors may be   --
--       used to endorse or promote products derived from this software       --
--       without specific prior written permission.                           --
--------------------------------------------------------------------------------
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"--
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  --
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE --
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  --
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        --
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF       --
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   --
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN    --
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    --
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE --
-- POSSIBILITY OF SUCH DAMAGE.                                                --
--------------------------------------------------------------------------------
--                                                                            --
--  Abstract : Encoder Wrapper Layer for 6280/7280/8270/8290/H1, common parts
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "rtthread.h"
#include "board.h"
#include <string.h>

//#include "ewl_linux_lock.h"
//#include "ewl_x280_common.h"
#include "ewl.h"

#define LOG_TAG                "JpegE"
#include "log.h"

#define PTRACE  LOG_I

#define JPEG_ENC_READ_REG(offset)     (*((volatile uint32_t *)JPEG_ENC_BASE + (offset)))
#define JPEG_ENC_WRITE_REG(offset, value)   (*((volatile uint32_t *)JPEG_ENC_BASE + (offset)) = (value)) 


#ifdef USE_EFENCE
#include "efence.h"
#endif

/* EWL internal information for Linux */
typedef struct
{
    u32 clientType;
    u32 regSize;             /* IO mem size */
    volatile u32 *pRegBase;  /* IO mem base */
    struct rt_semaphore reserve_lock;
    struct rt_semaphore wait_sema;
} hx280ewl_t;

static hx280ewl_t ewl_device;

static const char *busTypeName[7] = { "UNKNOWN", "AHB", "OCP", "AXI", "PCI", "AXIAHB", "AXIAPB" };
static const char *synthLangName[3] = { "UNKNOWN", "VHDL", "VERILOG" };

void JPEG_ENC_IRQHandler(void)
{
    hx280ewl_t *dev = (hx280ewl_t *)&ewl_device;
    u32 irq_status;
    u32 is_write1_clr;

    rt_interrupt_enter();
        
    irq_status = JPEG_ENC_READ_REG(1);

    //PTRACE("irq_status:%x\n", irq_status);
    
    /* BASE_HWFuse2 = 0x4a0; HWCFGIrqClearSupport = 0x00800000 */
    is_write1_clr =  JPEG_ENC_READ_REG(0x4a0/4) & 0x00800000;
    if(irq_status & 0x01)
    {
        /* clear enc IRQ and slice ready interrupt bit */
        if (is_write1_clr)
        {
            JPEG_ENC_WRITE_REG(1, irq_status & (0x101UL));
        }
        else
        {
            JPEG_ENC_WRITE_REG(1, irq_status & (~0x101UL));
        }

        /* Handle slice ready interrupts. The reference implementation
         * doesn't signal slice ready interrupts to EWL.
         * The EWL will poll the slices ready register value. */
        if ((irq_status & 0x1FE) == 0x100)
        {
            //PDEBUG("Slice ready IRQ handled!\n");
            return;
        }

        rt_sem_release(&dev->wait_sema);
    }
    else
    {
        RT_ASSERT(0);
    }
    rt_interrupt_leave();    
}


int MapAsicRegisters(hx280ewl_t * ewl)
{
    ewl->regSize = sizeof(*hwp_jpeg_enc);
    ewl->pRegBase = (u32 *)JPEG_ENC_BASE;

    return 0;
}

/*******************************************************************************
 Function name   : EWLReadAsicID
 Description     : Read ASIC ID register, static implementation
 Return type     : u32 ID
 Argument        : void
*******************************************************************************/
u32 EWLReadAsicID()
{
    u32 id;

    id = JPEG_ENC_READ_REG(0);

    PTRACE("EWLReadAsicID: 0x%08x at 0x%08lx\n", id, JPEG_ENC_BASE);

    return id;
}

/*******************************************************************************
 Function name   : EWLReadAsicConfig
 Description     : Reads ASIC capability register, static implementation
 Return type     : EWLHwConfig_t 
 Argument        : void
*******************************************************************************/
EWLHwConfig_t EWLReadAsicConfig(void)
{
    u32 cfgval;
    EWLHwConfig_t cfg_info;

    memset(&cfg_info, 0, sizeof(cfg_info));
    
    cfgval = JPEG_ENC_READ_REG(63);
    cfg_info.maxEncodedWidth = cfgval & ((1 << 12) - 1);
    cfg_info.h264Enabled = (cfgval >> 27) & 1;
    cfg_info.vp8Enabled = (cfgval >> 26) & 1;
    cfg_info.jpegEnabled = (cfgval >> 25) & 1;
    cfg_info.vsEnabled = (cfgval >> 24) & 1;
    cfg_info.rgbEnabled = (cfgval >> 28) & 1;
    cfg_info.searchAreaSmall = (cfgval >> 29) & 1;
    cfg_info.scalingEnabled = (cfgval >> 30) & 1;
    cfg_info.busType = (cfgval >> 20) & 15;
    cfg_info.synthesisLanguage = (cfgval >> 16) & 15;
    cfg_info.busWidth = (cfgval >> 12) & 15;

    cfgval = JPEG_ENC_READ_REG(296);
    cfg_info.addr64Support = (cfgval >> 31) & 1;
    cfg_info.dnfSupport = (cfgval >> 30) & 1;
    cfg_info.rfcSupport = (cfgval >> 28) & 3;
    cfg_info.enhanceSupport = (cfgval >> 27) & 1;
    cfg_info.instantSupport = (cfgval >> 26) & 1;
    cfg_info.svctSupport = (cfgval >> 25) & 1;
    cfg_info.inAxiIdSupport = (cfgval >> 24) & 1;
    cfg_info.inLoopbackSupport = (cfgval >> 23) & 1;
    cfg_info.irqEnhanceSupport = (cfgval >> 22) & 1;

    PTRACE("EWLReadAsicConfig:\n"
           "    maxEncodedWidth   = %d\n"
           "    h264Enabled       = %s\n"
           "    jpegEnabled       = %s\n"
           "    vp8Enabled        = %s\n"
           "    vsEnabled         = %s\n"
           "    rgbEnabled        = %s\n"
           "    searchAreaSmall   = %s\n"
           "    scalingEnabled    = %s\n"
           "    address64bits     = %s\n"
           "    denoiseEnabled    = %s\n"
           "    rfcEnabled        = %s\n"
           "    instanctEnabled   = %s\n"
           "    busType           = %s\n"
           "    synthesisLanguage = %s\n"
           "    busWidth          = %d\n",
           cfg_info.maxEncodedWidth,
           cfg_info.h264Enabled == 1 ? "YES" : "NO",
           cfg_info.jpegEnabled == 1 ? "YES" : "NO",
           cfg_info.vp8Enabled == 1 ? "YES" : "NO",
           cfg_info.vsEnabled == 1 ? "YES" : "NO",
           cfg_info.rgbEnabled == 1 ? "YES" : "NO",
           cfg_info.searchAreaSmall == 1 ? "YES" : "NO",
           cfg_info.scalingEnabled == 1 ? "YES" : "NO",
           cfg_info.addr64Support == 1 ? "YES" : "NO",
           cfg_info.dnfSupport == 1 ? "YES" : "NO",
           cfg_info.rfcSupport == 0 ? "NO" : 
                  ( cfg_info.rfcSupport == 2 ? "LUMA"  :
                   (cfg_info.rfcSupport == 1 ? "CHROMA":"FULL")),
           cfg_info.instantSupport == 1 ? "YES" : "NO",
           cfg_info.busType < 7 ? busTypeName[cfg_info.busType] : "UNKNOWN",
           cfg_info.synthesisLanguage <
           3 ? synthLangName[cfg_info.synthesisLanguage] : "ERROR",
           cfg_info.busWidth * 32);

    return cfg_info;
}

/*******************************************************************************
 Function name   : EWLInit
 Description     : Allocate resources and setup the wrapper module
 Return type     : ewl_ret 
 Argument        : void
*******************************************************************************/
const void *EWLInit(EWLInitParam_t * param)
{
    PTRACE("EWLInit: Start\n");

    /* Check for NULL pointer */
    if(param == NULL || param->clientType != EWL_CLIENT_TYPE_JPEG_ENC)
    {
        PTRACE(("EWLInit: Bad calling parameters!\n"));
        return NULL;
    }
    
    return &ewl_device;
}

/*******************************************************************************
 Function name   : EWLRelease
 Description     : Release the wrapper module by freeing all the resources
 Return type     : ewl_ret 
 Argument        : void
*******************************************************************************/
i32 EWLRelease(const void *inst)
{

    PTRACE("EWLRelease: instance freed\n");
   
    return EWL_OK;
}

/*******************************************************************************
 Function name   : EWLWriteReg
 Description     : Set the content of a hadware register
 Return type     : void 
 Argument        : u32 offset
 Argument        : u32 val
*******************************************************************************/
void EWLWriteReg(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    RT_ASSERT(enc != NULL && offset < enc->regSize);

    if(offset == 0x04)
    {
        //asic_status = val;
    }

    offset = offset / 4;
    *(enc->pRegBase + offset) = val;

    PTRACE("EWLWriteReg 0x%02x with value %08x\n", offset * 4, val);
}

/*------------------------------------------------------------------------------
    Function name   : EWLEnableHW
    Description     : 
    Return type     : void 
    Argument        : const void *inst
    Argument        : u32 offset
    Argument        : u32 val
------------------------------------------------------------------------------*/
void EWLEnableHW(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    RT_ASSERT(enc != NULL && offset < enc->regSize);

    if(offset == 0x04)
    {
        //asic_status = val;
    }

    offset = offset / 4;
    *(enc->pRegBase + offset) = val;

    PTRACE("EWLEnableHW 0x%02x with value %08x\n", offset * 4, val);
}

/*------------------------------------------------------------------------------
    Function name   : EWLDisableHW
    Description     : 
    Return type     : void 
    Argument        : const void *inst
    Argument        : u32 offset
    Argument        : u32 val
------------------------------------------------------------------------------*/
void EWLDisableHW(const void *inst, u32 offset, u32 val)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    RT_ASSERT(enc != NULL && offset < enc->regSize);

    offset = offset / 4;
    *(enc->pRegBase + offset) = val;

    //asic_status = val;

    PTRACE("EWLDisableHW 0x%02x with value %08x\n", offset * 4, val);
}

/*******************************************************************************
 Function name   : EWLReadReg
 Description     : Retrive the content of a hadware register
                    Note: The status register will be read after every MB
                    so it may be needed to buffer it's content if reading
                    the HW register is slow.
 Return type     : u32 
 Argument        : u32 offset
*******************************************************************************/
u32 EWLReadReg(const void *inst, u32 offset)
{
    u32 val;
    hx280ewl_t *enc = (hx280ewl_t *) inst;

    RT_ASSERT(offset < enc->regSize);

    if(offset == 0x04)
    {
        //return asic_status;
    }

    offset = offset / 4;
    val = *(enc->pRegBase + offset);

    PTRACE("EWLReadReg 0x%02x --> %08x\n", offset * 4, val);

    return val;
}

/*------------------------------------------------------------------------------
    Function name   : EWLMallocRefFrm
    Description     : Allocate a frame buffer (contiguous linear RAM memory)
    
    Return type     : i32 - 0 for success or a negative error code 
    
    Argument        : const void * instance - EWL instance
    Argument        : u32 size - size in bytes of the requested memory
    Argument        : EWLLinearMem_t *info - place where the allocated memory
                        buffer parameters are returned
------------------------------------------------------------------------------*/
i32 EWLMallocRefFrm(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;
    i32 ret;

    RT_ASSERT(enc_ewl != NULL);
    RT_ASSERT(buff != NULL);

    PTRACE("EWLMallocRefFrm\t%8d bytes\n", size);

    ret = EWLMallocLinear(enc_ewl, size, buff);

    PTRACE("EWLMallocRefFrm %08x --> %p\n", buff->busAddress,
           buff->virtualAddress);

    return ret;
}

/*------------------------------------------------------------------------------
    Function name   : EWLFreeRefFrm
    Description     : Release a frame buffer previously allocated with 
                        EWLMallocRefFrm.
    
    Return type     : void 
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - frame buffer memory information
------------------------------------------------------------------------------*/
void EWLFreeRefFrm(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    RT_ASSERT(enc_ewl != NULL);
    RT_ASSERT(buff != NULL);

    EWLFreeLinear(enc_ewl, buff);

    PTRACE("EWLFreeRefFrm\t%p\n", buff->virtualAddress);
}

/*------------------------------------------------------------------------------
    Function name   : EWLMallocLinear
    Description     : Allocate a contiguous, linear RAM  memory buffer
    
    Return type     : i32 - 0 for success or a negative error code  
    
    Argument        : const void * instance - EWL instance
    Argument        : u32 size - size in bytes of the requested memory
    Argument        : EWLLinearMem_t *info - place where the allocated memory
                        buffer parameters are returned
------------------------------------------------------------------------------*/
i32 EWLMallocLinear(const void *instance, u32 size, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;
    u32 pgsize = 4;

    RT_ASSERT(enc_ewl != NULL);
    RT_ASSERT(buff != NULL);

    PTRACE("EWLMallocLinear\t%8d bytes\n", size);

    buff->size = (size + (pgsize - 1)) & (~(pgsize - 1));
    buff->virtualAddress = rt_malloc(buff->size);

    if(!buff->virtualAddress)
    {
        PTRACE("EWLMallocLinear: Failed to malloc: %d\n", buff->size);
        return EWL_ERROR;
    }

    buff->busAddress = (ptr_t)(buff->virtualAddress);
    PTRACE("EWLMallocLinear 0x%08x\n",
           buff->busAddress);

    return EWL_OK;
}

/*------------------------------------------------------------------------------
    Function name   : EWLFreeLinear
    Description     : Release a linera memory buffer, previously allocated with 
                        EWLMallocLinear.
    
    Return type     : void 
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - linear buffer memory information
------------------------------------------------------------------------------*/
void EWLFreeLinear(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;
    EWLLinearMem_t *buff = (EWLLinearMem_t *) info;

    RT_ASSERT(enc_ewl != NULL);
    RT_ASSERT(buff != NULL);

    if(buff->virtualAddress)
    {
        rt_free(buff->virtualAddress);
    }

    PTRACE("EWLFreeLinear\t%p\n", buff->virtualAddress);
}

/*******************************************************************************
 Function name   : EWLReserveHw
 Description     : Reserve HW resource for currently running codec
*******************************************************************************/
i32 EWLReserveHw(const void *inst)
{
    hx280ewl_t *ewl = (hx280ewl_t *) inst;
    i32 ret;
    u32 temp = 0;
    rt_err_t err;
    
    /* Check invalid parameters */
    if(ewl == NULL)
      return EWL_ERROR;
    
    PTRACE("EWLReserveHw: PID 0x%p trying to reserve ...\n", rt_thread_self());
    
    
    err = rt_sem_take(&ewl->reserve_lock, 1000);
    if (RT_EOK != err)
    {
        PTRACE("EWLReserveHw failed\n");
        return EWL_ERROR;
    }
    else
    {
        PTRACE("EWLReserveHw successed\n");
    }
    
    EWLWriteReg(ewl, 0x38, 0);//disable encoder
    
    PTRACE("EWLReserveHw: ENC HW locked by PID 0x%p\n", rt_thread_self());

    return EWL_OK;
}

/*******************************************************************************
 Function name   : EWLReleaseHw
 Description     : Release HW resource when frame is ready
*******************************************************************************/
void EWLReleaseHw(const void *inst)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;
    u32 val, temp;
    rt_err_t err;

    RT_ASSERT(enc != NULL);

    val = EWLReadReg(inst, 0x38);
    EWLWriteReg(inst, 0x38, val & (~0x01)); /* reset ASIC */

    PTRACE("EWLReleaseHw: PID 0x%p trying to release ...\n", rt_thread_self());

    err = rt_sem_release(&enc->reserve_lock);
    RT_ASSERT(RT_EOK == err)

    PTRACE("EWLReleaseHw: HW released by PID 0x%p\n", rt_thread_self());
    return ;
}

/* SW/SW shared memory */
/*------------------------------------------------------------------------------
    Function name   : EWLmalloc
    Description     : Allocate a memory block. Same functionality as
                      the ANSI C malloc()
    
    Return type     : void pointer to the allocated space, or NULL if there
                      is insufficient memory available
    
    Argument        : u32 n - Bytes to allocate
------------------------------------------------------------------------------*/
void *EWLmalloc(u32 n)
{

    void *p = rt_malloc((size_t) n);

    PTRACE("EWLmalloc\t%8d bytes --> %p\n", n, p);

    return p;
}

/*------------------------------------------------------------------------------
    Function name   : EWLfree
    Description     : Deallocates or frees a memory block. Same functionality as
                      the ANSI C free()
    
    Return type     : void 
    
    Argument        : void *p - Previously allocated memory block to be freed
------------------------------------------------------------------------------*/
void EWLfree(void *p)
{
    PTRACE("EWLfree\t%p\n", p);
    if(p != NULL)
        rt_free(p);
}

/*------------------------------------------------------------------------------
    Function name   : EWLcalloc
    Description     : Allocates an array in memory with elements initialized
                      to 0. Same functionality as the ANSI C calloc()
    
    Return type     : void pointer to the allocated space, or NULL if there
                      is insufficient memory available
    
    Argument        : u32 n - Number of elements
    Argument        : u32 s - Length in bytes of each element.
------------------------------------------------------------------------------*/
void *EWLcalloc(u32 n, u32 s)
{
    void *p = rt_calloc((size_t) n, (size_t) s);

    PTRACE("EWLcalloc\t%8d bytes --> %p\n", n * s, p);

    return p;
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemcpy
    Description     : Copies characters between buffers. Same functionality as
                      the ANSI C memcpy()
    
    Return type     : The value of destination d
    
    Argument        : void *d - Destination buffer
    Argument        : const void *s - Buffer to copy from
    Argument        : u32 n - Number of bytes to copy
------------------------------------------------------------------------------*/
void *EWLmemcpy(void *d, const void *s, u32 n)
{
    return memcpy(d, s, (size_t) n);
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemset
    Description     : Sets buffers to a specified character. Same functionality
                      as the ANSI C memset()
    
    Return type     : The value of destination d
    
    Argument        : void *d - Pointer to destination
    Argument        : i32 c - Character to set
    Argument        : u32 n - Number of characters
------------------------------------------------------------------------------*/
void *EWLmemset(void *d, i32 c, u32 n)
{
    return memset(d, (int) c, (size_t) n);
}

/*------------------------------------------------------------------------------
    Function name   : EWLmemcmp
    Description     : Compares two buffers. Same functionality
                      as the ANSI C memcmp()

    Return type     : Zero if the first n bytes of s1 match s2

    Argument        : const void *s1 - Buffer to compare
    Argument        : const void *s2 - Buffer to compare
    Argument        : u32 n - Number of characters
------------------------------------------------------------------------------*/
int EWLmemcmp(const void *s1, const void *s2, u32 n)
{
    return memcmp(s1, s2, (size_t) n);
}


/*------------------------------------------------------------------------------
    Function name   : EWLGetInputLineBufferBase
    Description        : Get the base address of on-chip sram used for input MB line buffer.
    
    Return type     : i32 - 0 for success or a negative error code  
    
    Argument        : const void * instance - EWL instance
    Argument        : EWLLinearMem_t *info - place where the sram parameters are returned
------------------------------------------------------------------------------*/
i32 EWLGetInputLineBufferBase(const void *instance, EWLLinearMem_t * info)
{
    hx280ewl_t *enc_ewl = (hx280ewl_t *) instance;

    RT_ASSERT(enc_ewl != NULL);
    RT_ASSERT(info != NULL);

    info->virtualAddress = NULL;
    info->busAddress = 0;
    info->size = 0;


    PTRACE("EWLMallocLinear 0x%08x (ASIC) --> %p\n", info->busAddress, info->virtualAddress);

    return EWL_OK;
}


/*******************************************************************************
 Function name   : EWLWaitHwRdy
 Description     : Wait for the encoder semaphore
 Return type     : i32 
 Argument        : void
*******************************************************************************/
i32 EWLWaitHwRdy(const void *inst, u32 *slicesReady)
{
    hx280ewl_t *enc = (hx280ewl_t *) inst;
    i32 ret = EWL_HW_WAIT_OK;
    //u32 prevSlicesReady = 0;
    u32 temp;
    rt_err_t err;

    PTRACE("EWLWaitHw: Start\n");

    //if (slicesReady)
    //    prevSlicesReady = *slicesReady;
        
    /* Check invalid parameters */
    if(enc == NULL)
    {
        RT_ASSERT(0);
        return EWL_HW_WAIT_ERROR;
    }

    err = rt_sem_take(&enc->wait_sema, 1000);
    if (RT_EOK != err)
    {
        PTRACE("WAIT failed\n");
        ret = EWL_HW_WAIT_ERROR;
        goto out;
        
    }

    if (slicesReady)
       *slicesReady = (enc->pRegBase[21] >> 16) & 0xFF;
    // PTRACE("EWLWaitHw: asic_status = %x\n", asic_status);
out:
    // asic_status = enc->pRegBase[1]; /* update the buffered asic status */
    PTRACE("EWLWaitHw: OK!\n");

    return EWL_OK;
}


void JpegEnc_Trace(const char *msg)
{
    PTRACE(msg);
}


static int JpegEncDeviceInit(void)
{
    rt_err_t err;    
    hx280ewl_t *enc = NULL;
    
    HAL_RCC_EnableModule(RCC_MOD_JENC);

    /* Allocate instance */
    enc = &ewl_device;
    
    err = rt_sem_init(&enc->reserve_lock, "jpege_lock", 1, RT_IPC_FLAG_FIFO);
    RT_ASSERT(RT_EOK == err);
    err = rt_sem_init(&enc->wait_sema, "jpege_wait", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(RT_EOK == err);
    
    enc->clientType = EWL_CLIENT_TYPE_JPEG_ENC;

    HAL_NVIC_SetPriority(JPEG_ENC_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(JPEG_ENC_IRQn);

    /* map hw registers to user space */
    if(MapAsicRegisters(enc) != 0)
    {
        RT_ASSERT(0);
    }

    PTRACE("EWLInit: mmap regs %d bytes --> %p\n", enc->regSize, enc->pRegBase);



    return 0;
}
INIT_DEVICE_EXPORT(JpegEncDeviceInit);




