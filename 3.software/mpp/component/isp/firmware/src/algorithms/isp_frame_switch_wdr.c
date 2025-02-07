/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : isp_frame_switch_wdr.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2013/07/15
  Description   : 
  History       :
  1.Date        : 2013/01/16
    Author      : l00184411
    Modification: Created file

******************************************************************************/

#include "isp_config.h"
#include "isp_alg.h"
#include "isp_sensor.h"
#include "isp_ext_config.h"
#include "isp_math_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct hiISP_FS_WDR_S
{
    /* Public */
    HI_U32 u32StitchErrThresh;

} ISP_FS_WDR_S;

ISP_FS_WDR_S g_astFSWDRCtx[ISP_MAX_DEV_NUM] = {{0}};
#define FS_WDR_GET_CTX(dev, pstCtx)   pstCtx = &g_astFSWDRCtx[dev]

static HI_VOID FrameWDREnable(ISP_DEV IspDev)
{
    /* Enable Frame-stitching WDR */
    hi_isp_top_bypass_frame_switch_wdr_write(0);
    hi_isp_wdr_wdr_mode_write(1);
    hi_isp_wdr_exposure_ratio_write(0x40);

    hi_isp_top_gamma_fe_position_write(1);
    hi_isp_wdr_use_stitching_error_write(1);
    hi_isp_wdr_long_first_write(1);

    return;
}

static HI_VOID FrameWDRDisable(ISP_DEV IspDev)
{
    /* Disable Frame-stitching WDR */
    hi_isp_top_bypass_frame_switch_wdr_write(1);
    hi_isp_wdr_wdr_mode_write(0);
    hi_isp_wdr_exposure_ratio_write(0x40);

    hi_isp_top_gamma_fe_position_write(0);

    return;
}

static HI_VOID FrameWDRExtRegsDefault(HI_VOID)
{
    
    return;
}

static HI_VOID FrameWDRRegsDefault(HI_VOID) 
{
    hi_isp_wdr_long_first_write(0);
    hi_isp_wdr_stitch_error_thresh_write(HI_ISP_WDR_STITCH_ERROR_THRESH_DEFAULT);
    
    return;
}

static HI_VOID FrameWDRExtRegsInitialize(ISP_DEV IspDev)
{

    return;
}

static HI_VOID FrameWDRRegsInitialize(ISP_DEV IspDev)
{
   
    return;
}

static HI_S32 FrameWDRReadExtRegs(ISP_DEV IspDev)
{
    return 0;
}


#define ISO_INPUT                 1,   2,   4,   8,  16,  32,  64, 128
#define STITCH_ERR_THRESH_PARAM   0,   0,0x08,0x10,0x18,0x20,0x28,0x30
#define STITCH_ERR_THRESH_MAX     0x40
#define STITCH_ERR_THRESH_MIN     0x1
HI_S32 FSWDRStitchErrThreshCal(ISP_FS_WDR_S *pstFSWDR, HI_U32 u32Iso)
{
    HI_U32 u32ErrThreshParam = 0;
    HI_U32 au32ISOLut[]         = {ISO_INPUT};
    HI_U32 au32ErrThreshParam[] = {STITCH_ERR_THRESH_PARAM};

    u32ErrThreshParam = transition(&au32ISOLut[0], &au32ErrThreshParam[0], sizeof(au32ISOLut) / sizeof(HI_U32), (u32Iso << 7)/100, 7);
    u32ErrThreshParam = CLIP3((STITCH_ERR_THRESH_MAX - u32ErrThreshParam), STITCH_ERR_THRESH_MIN, STITCH_ERR_THRESH_MAX);

    pstFSWDR->u32StitchErrThresh = u32ErrThreshParam;

    return HI_SUCCESS;
}


HI_S32 ISP_FrameWDRInit(ISP_DEV IspDev)
{
    ISP_FS_WDR_S *pstFSWDR = HI_NULL;
    FS_WDR_GET_CTX(IspDev, pstFSWDR);

    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_GET_CTX(IspDev, pstIspCtx);

    HI_U8 u8WDRMode;

    u8WDRMode = pstIspCtx->u8SnsWDRMode;

    FrameWDRRegsDefault();
    FrameWDRExtRegsDefault();
    FrameWDRRegsInitialize(IspDev);
    FrameWDRExtRegsInitialize(IspDev);

    pstFSWDR->u32StitchErrThresh = HI_ISP_WDR_STITCH_ERROR_THRESH_DEFAULT;

    if (IS_LINEAR_MODE(u8WDRMode) || IS_BUILT_IN_WDR_MODE(u8WDRMode))
    {
        FrameWDRDisable(IspDev);
    }
    else if (IS_FS_WDR_MODE(u8WDRMode))
    {
        FrameWDREnable(IspDev);
    }
    else
    {
        /* unknow mode */
    }
    
    return HI_SUCCESS;
}

HI_S32 ISP_FrameWDRRun(ISP_DEV IspDev, const HI_VOID *pStatInfo,
    HI_VOID *pRegCfg, HI_S32 s32Rsv)
{
    ISP_CTX_S    *pstIspCtx = HI_NULL;
    ISP_FS_WDR_S *pstFSWDR = HI_NULL;
    ISP_GET_CTX(IspDev, pstIspCtx);
    FS_WDR_GET_CTX(IspDev, pstFSWDR);

    if (pstIspCtx->stLinkage.bDefectPixel)
    {
        return HI_SUCCESS;
    }
    
    FrameWDRReadExtRegs(IspDev);

    FSWDRStitchErrThreshCal(pstFSWDR, pstIspCtx->stLinkage.u32Iso);

    hi_isp_wdr_stitch_error_thresh_write(pstFSWDR->u32StitchErrThresh);

    return HI_SUCCESS;
}

HI_S32 ISP_FrameWDRCtrl(ISP_DEV IspDev, HI_U32 u32Cmd, HI_VOID *pValue)
{
    HI_U8 u8Mode = *((HI_U8 *)pValue);
    
    switch (u32Cmd)
    {
        case ISP_WDR_MODE_SET:

            if (IS_LINEAR_MODE(u8Mode) || IS_BUILT_IN_WDR_MODE(u8Mode))
            {
                FrameWDRDisable(IspDev);
            }
            else if (IS_FS_WDR_MODE(u8Mode))
            {
                FrameWDREnable(IspDev);
            }
            else
            {
                /* unknow mode */
            }
            break;
        default :
            break;
    }
    return HI_SUCCESS;
}

HI_S32 ISP_FrameWDRExit(ISP_DEV IspDev)
{
    return HI_SUCCESS;
}

HI_S32 ISP_AlgRegisterFrameWDR(ISP_DEV IspDev)
{
    ISP_CTX_S *pstIspCtx = HI_NULL;
    ISP_ALG_NODE_S *pstAlgs = HI_NULL;
    
    ISP_GET_CTX(IspDev, pstIspCtx);

    pstAlgs = ISP_SearchAlg(pstIspCtx->astAlgs);
    ISP_CHECK_POINTER(pstAlgs);

    pstAlgs->enAlgType = ISP_ALG_FrameWDR;
    pstAlgs->stAlgFunc.pfn_alg_init = ISP_FrameWDRInit;
    pstAlgs->stAlgFunc.pfn_alg_run  = ISP_FrameWDRRun;
    pstAlgs->stAlgFunc.pfn_alg_ctrl = ISP_FrameWDRCtrl;
    pstAlgs->stAlgFunc.pfn_alg_exit = ISP_FrameWDRExit;
    pstAlgs->bUsed = HI_TRUE;

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

