
#ifndef _MV_CAMERA_PARAMS_H_
#define _MV_CAMERA_PARAMS_H_

#include "PixelType.h"

#ifndef __cplusplus
typedef char    bool;
#define true    1
#define false   0
#endif

/// \~chinese 
/// 设备类型定义     
/// \~english
/// Device Type Definition
#define MV_UNKNOW_DEVICE        0x00000000          ///< \~chinese 未知设备类型，保留意义       \~english Unknown Device Type, Reserved 
#define MV_GIGE_DEVICE          0x00000001          ///< \~chinese GigE设备                     \~english GigE Device
#define MV_1394_DEVICE          0x00000002          ///< \~chinese 1394-a/b 设备                \~english 1394-a/b Device
#define MV_USB_DEVICE           0x00000004          ///< \~chinese USB 设备                     \~english USB Device
#define MV_CAMERALINK_DEVICE    0x00000008          ///< \~chinese CamLink设备                  \~english CamLink Device

/// \~chinese GigE设备信息         \~english GigE device info
typedef struct _MV_GIGE_DEVICE_INFO_
{
    unsigned int        nIpCfgOption;        
    unsigned int        nIpCfgCurrent;              ///< \~chinese       \~english IP configuration:bit31-static bit30-dhcp bit29-lla      
    unsigned int        nCurrentIp;
    unsigned int        nCurrentSubNetMask;         ///< \~chinese       \~english curtent subnet mask     
    unsigned int        nDefultGateWay;             ///< \~chinese       \~english current gateway
    unsigned char       chManufacturerName[32];
    unsigned char       chModelName[32];
    unsigned char       chDeviceVersion[32];
    unsigned char       chManufacturerSpecificInfo[48];
    unsigned char       chSerialNumber[16];
    unsigned char       chUserDefinedName[16]; 
    unsigned int        nNetExport;                 ///< \~chinese 网口IP地址       \~english NetWork IP Address

    unsigned int        nReserved[4];
}MV_GIGE_DEVICE_INFO;

#define INFO_MAX_BUFFER_SIZE 64

/// \~chinese USB设备信息         \~english USB device info
typedef struct _MV_USB3_DEVICE_INFO_
{
    unsigned char       CrtlInEndPoint;                             ///< \~chinese 控制输入端点         \~english Control input endpoint
    unsigned char       CrtlOutEndPoint;                            ///< \~chinese 控制输出端点         \~english Control output endpoint
    unsigned char       StreamEndPoint;                             ///< \~chinese 流端点               \~english Flow endpoint
    unsigned char       EventEndPoint;                              ///< \~chinese 事件端点             \~english Event endpoint
    unsigned short      idVendor;                                   ///< \~chinese 供应商ID号           \~english Vendor ID Number
    unsigned short      idProduct;                                  ///< \~chinese 产品ID号             \~english Device ID Number
    unsigned int        nDeviceNumber;                              ///< \~chinese 设备序列号           \~english Device Serial Number
    unsigned char       chDeviceGUID[INFO_MAX_BUFFER_SIZE];         ///< \~chinese 设备GUID号           \~english Device GUID Number
    unsigned char       chVendorName[INFO_MAX_BUFFER_SIZE];         ///< \~chinese 供应商名字           \~english Vendor Name
    unsigned char       chModelName[INFO_MAX_BUFFER_SIZE];          ///< \~chinese 型号名字             \~english Model Name
    unsigned char       chFamilyName[INFO_MAX_BUFFER_SIZE];         ///< \~chinese 家族名字             \~english Family Name
    unsigned char       chDeviceVersion[INFO_MAX_BUFFER_SIZE];      ///< \~chinese 设备版本号           \~english Device Version
    unsigned char       chManufacturerName[INFO_MAX_BUFFER_SIZE];   ///< \~chinese 制造商名字           \~english Manufacturer Name
    unsigned char       chSerialNumber[INFO_MAX_BUFFER_SIZE];       ///< \~chinese 序列号               \~english Serial Number
    unsigned char       chUserDefinedName[INFO_MAX_BUFFER_SIZE];    ///< \~chinese 用户自定义名字       \~english User Defined Name
    unsigned int        nbcdUSB;                                    ///< \~chinese 支持的USB协议        \~english Support USB Protocol

    unsigned int        nReserved[3];                               ///< \~chinese 保留字节             \~english Reserved bytes
}MV_USB3_DEVICE_INFO;

/// \~chinese
/// \brief CamLink设备信息
/// \~english
/// \brief CamLink device info
typedef struct _MV_CamL_DEV_INFO_
{
    unsigned char       chPortID[INFO_MAX_BUFFER_SIZE];
    unsigned char       chModelName[INFO_MAX_BUFFER_SIZE];
    unsigned char       chFamilyName[INFO_MAX_BUFFER_SIZE];
    unsigned char       chDeviceVersion[INFO_MAX_BUFFER_SIZE];
    unsigned char       chManufacturerName[INFO_MAX_BUFFER_SIZE];
    unsigned char       chSerialNumber[INFO_MAX_BUFFER_SIZE];

    unsigned int        nReserved[38];
}MV_CamL_DEV_INFO;

/// \~chinese
/// \brief 设备信息
/// \~english
/// \brief Device info
typedef struct _MV_CC_DEVICE_INFO_
{
    unsigned short      nMajorVer;
    unsigned short      nMinorVer;
    unsigned int        nMacAddrHigh;               ///< \~chinese MAC 地址\~english MAC Address
    unsigned int        nMacAddrLow;

    unsigned int        nTLayerType;                ///< \~chinese 设备传输层协议类型，e.g. MV_GIGE_DEVICE\~english Device Transport Layer Protocol Type, e.g. MV_GIGE_DEVICE

    unsigned int        nReserved[4];

    union
    {
        MV_GIGE_DEVICE_INFO stGigEInfo;
        MV_USB3_DEVICE_INFO stUsb3VInfo; 
        MV_CamL_DEV_INFO    stCamLInfo;
        // more ...
    }SpecialInfo;

}MV_CC_DEVICE_INFO;

/// \~chinese 网络传输的相关信息\~english Network transmission information
typedef struct _MV_NETTRANS_INFO_
{
    int64_t             nReviceDataSize;    ///< \~chinese 已接收数据大小  [统计StartGrabbing和StopGrabbing之间的数据量]\~english Received Data Size  [Calculate the Data Size between StartGrabbing and StopGrabbing]
    int                 nThrowFrameCount;    ///< \~chinese 丢帧数量\~english Throw frame number
    unsigned int        nNetRecvFrameCount;
    int64_t             nRequestResendPacketCount;          ///< \~chinese 请求重发包数
    int64_t             nResendPacketCount;   ///< \~chinese 重发包数

}MV_NETTRANS_INFO;

#define MV_MAX_TLS_NUM          8               ///< \~chinese 最多支持的传输层实例个数\~english The maximum number of supported transport layer instances
#define MV_MAX_DEVICE_NUM      256              ///< \~chinese 最大支持的设备个数\~english The maximum number of supported devices

/// \~chinese
/// \brief 设备信息列表
/// \~english
/// \brief Device Information List
typedef struct _MV_CC_DEVICE_INFO_LIST_
{
    unsigned int        nDeviceNum;                         ///< \~chinese 在线设备数量\~english Online Device Number
    MV_CC_DEVICE_INFO*  pDeviceInfo[MV_MAX_DEVICE_NUM];     ///< \~chinese 支持最多256个设备\~english Support up to 256 devices

}MV_CC_DEVICE_INFO_LIST;


/// \~chinese Chunk内容         \~english The content of ChunkData
typedef struct _MV_CHUNK_DATA_CONTENT_
{
    unsigned char*      pChunkData;
    unsigned int        nChunkID;
    unsigned int        nChunkLen;

    unsigned int        nReserved[8];       // 保留

}MV_CHUNK_DATA_CONTENT;

/// \~chinese 输出帧的信息\~english Output Frame Information
typedef struct _MV_FRAME_OUT_INFO_
{
    unsigned short      nWidth;             ///< \~chinese 图像宽           \~english Image Width
    unsigned short      nHeight;            ///< \~chinese 图像高           \~english Image Height
    enum MvGvspPixelType     enPixelType;   ///< \~chinese 像素格式         \~english Pixel Type

    unsigned int        nFrameNum;          ///< \~chinese 帧号             \~english Frame Number
    unsigned int        nDevTimeStampHigh;  ///< \~chinese 时间戳高32位\~english Timestamp high 32 bits
    unsigned int        nDevTimeStampLow;   ///< \~chinese 时间戳低32位\~english Timestamp low 32 bits
    unsigned int        nReserved0;         ///< \~chinese 保留，8字节对齐\~english Reserved, 8-byte aligned
    int64_t             nHostTimeStamp;     ///< \~chinese 主机生成的时间戳\~english Host-generated timestamp

    unsigned int        nFrameLen;

    unsigned int        nLostPacket;  ///< \~chinese 本帧丢包数\~english Lost Pacekt Number In This Frame
    unsigned int        nReserved[2];
}MV_FRAME_OUT_INFO;

/// \~chinese 输出帧的信息\~english Output Frame Information
typedef struct _MV_FRAME_OUT_INFO_EX_
{
    unsigned short      nWidth;             ///< \~chinese 图像宽           \~english Image Width
    unsigned short      nHeight;            ///< \~chinese 图像高           \~english Image Height
    enum MvGvspPixelType     enPixelType;   ///< \~chinese 像素格式         \~english Pixel Type

    unsigned int        nFrameNum;          ///< \~chinese 帧号             \~english Frame Number
    unsigned int        nDevTimeStampHigh;  ///< \~chinese 时间戳高32位\~english Timestamp high 32 bits
    unsigned int        nDevTimeStampLow;   ///< \~chinese 时间戳低32位\~english Timestamp low 32 bits
    unsigned int        nReserved0;         ///< \~chinese 保留，8字节对齐\~english Reserved, 8-byte aligned
    int64_t             nHostTimeStamp;     ///< \~chinese 主机生成的时间戳\~english Host-generated timestamp

    unsigned int        nFrameLen;

    /// \~chinese 设备水印时标\~english Device frame-specific time scale
    unsigned int        nSecondCount;
    unsigned int        nCycleCount;
    unsigned int        nCycleOffset;

    float               fGain;
    float               fExposureTime;
    unsigned int        nAverageBrightness;     ///< \~chinese 平均亮度\~english Average brightness

    /// \~chinese 白平衡相关\~english White balance
    unsigned int        nRed;
    unsigned int        nGreen;
    unsigned int        nBlue;

    unsigned int        nFrameCounter;
    unsigned int        nTriggerIndex;      ///< \~chinese 触发计数\~english Trigger Counting

    unsigned int        nInput;        ///< \~chinese 输入\~english Input
    unsigned int        nOutput;       ///< \~chinese 输出\~english Output

    /// \~chinese ROI区域\~english ROI Region
    unsigned short      nOffsetX;
    unsigned short      nOffsetY;
    unsigned short      nChunkWidth;
    unsigned short      nChunkHeight;

    unsigned int        nLostPacket;  ///< \~chinese 本帧丢包数\~english Lost Pacekt Number In This Frame

    unsigned int        nUnparsedChunkNum;///< \~chinese 未解析的Chunkdata个数
    union
    {
        MV_CHUNK_DATA_CONTENT* pUnparsedChunkContent;
        int64_t nAligning;
    }UnparsedChunkList;

    unsigned int        nReserved[36];       // 保留
}MV_FRAME_OUT_INFO_EX;

/// \~chinese 图像结构体，输出图像指针地址及图像信息\~english Image Struct, output the pointer of Image and the information of the specific image
typedef struct _MV_FRAME_OUT_
{
    unsigned char*      pBufAddr;              ///< \~chinese 图像指针地址\~english  pointer of image
    MV_FRAME_OUT_INFO_EX    stFrameInfo;     ///< \~chinese 图像信息\~english information of the specific image

    unsigned int        nRes[16];                ///< \~chinese 保留\~english reserved
}MV_FRAME_OUT;


typedef struct _MV_DISPLAY_FRAME_INFO_
{
    void*               hWnd;               ///< \~chinese 窗口句柄\~english HWND
    unsigned char*      pData;              ///< \~chinese 显示的数据\~english Data Buffer
    unsigned int        nDataLen;           ///< \~chinese 数据长度\~english Data Size
    unsigned short      nWidth;             ///< \~chinese 图像宽\~english Width
    unsigned short      nHeight;            ///< \~chinese 图像高\~english Height
    enum MvGvspPixelType    enPixelType;        ///< \~chinese 像素格式\~english Pixel format
    unsigned int        nRes[4];

}MV_DISPLAY_FRAME_INFO;

/// \~chinese 保存图片格式\~english Save image type
enum MV_SAVE_IAMGE_TYPE
{
    MV_Image_Undefined                 = 0,
    MV_Image_Bmp                       = 1,
    MV_Image_Jpeg                      = 2,
    MV_Image_Png                       = 3,
    MV_Image_Tif                       = 4,
};

/// \~chinese 图片保存参数\~english Save Image Parameters
typedef struct _MV_SAVE_IMAGE_PARAM_T_
{
    unsigned char*      pData;              ///< [IN]   \~chinese 输入数据缓存\~english Input Data Buffer
    unsigned int        nDataLen;           ///< [IN]   \~chinese 输入数据大小\~english Input Data Size
    enum MvGvspPixelType    enPixelType;   ///< [IN]    \~chinese 输入数据的像素格式\~english Input Data Pixel Format
    unsigned short      nWidth;             ///< [IN]   \~chinese 图像宽\~english Image Width
    unsigned short      nHeight;            ///< [IN]    \~chinese 图像高\~english Image Height

    unsigned char*      pImageBuffer;       ///< [OUT]   \~chinese 输出图片缓存\~english Output Image Buffer
    unsigned int        nImageLen;          ///< [OUT]   \~chinese 输出图片大小\~english Output Image Size
    unsigned int        nBufferSize;        ///< [IN]    \~chinese 提供的输出缓冲区大小\~english Output buffer size provided
    enum MV_SAVE_IAMGE_TYPE  enImageType;   ///< [IN]     \~chinese 输出图片格式\~english Output Image Format

}MV_SAVE_IMAGE_PARAM;

/// \~chinese 图片保存参数\~english Save Image Parameters
typedef struct _MV_SAVE_IMAGE_PARAM_T_EX_
{
    unsigned char*      pData;              ///< [IN]   \~chinese 输入数据缓存\~english Input Data Buffer
    unsigned int        nDataLen;           ///< [IN]   \~chinese 输入数据大小\~english Input Data Size
    enum MvGvspPixelType    enPixelType;   ///< [IN]    \~chinese 输入数据的像素格式\~english Input Data Pixel Format
    unsigned short      nWidth;             ///< [IN]   \~chinese 图像宽\~english Image Width
    unsigned short      nHeight;            ///< [IN]    \~chinese 图像高\~english Image Height

    unsigned char*      pImageBuffer;       ///< [OUT]   \~chinese 输出图片缓存\~english Output Image Buffer
    unsigned int        nImageLen;          ///< [OUT]   \~chinese 输出图片大小\~english Output Image Size
    unsigned int        nBufferSize;        ///< [IN]    \~chinese 提供的输出缓冲区大小\~english Output buffer size provided
    enum MV_SAVE_IAMGE_TYPE enImageType;   ///< [IN]     \~chinese 输出图片格式\~english Output Image Format
    unsigned int        nJpgQuality;        ///< [IN]    \~chinese JPG编码质量(50-99]，其它格式无效\~english Encoding quality(50-99]，Other formats are invalid

    ///< [IN]    \~chinese Bayer格式转为RGB24的插值方法  0-最近邻 1-双线性 2-Hamilton （如果传入其它值则默认为最近邻）
	///< [IN]    \~english Interpolation method of convert Bayer to RGB24  0-nearest neighbour 1-bilinearity 2-Hamilton
    unsigned int        iMethodValue;
    unsigned int        nReserved[3];

}MV_SAVE_IMAGE_PARAM_EX;

/// \~chinese 旋转角度                  \~english Rotation angle
typedef enum _MV_IMG_ROTATION_ANGLE_
{
	MV_IMAGE_ROTATE_90                  = 1,
	MV_IMAGE_ROTATE_180                 = 2,
	MV_IMAGE_ROTATE_270                 = 3,

}MV_IMG_ROTATION_ANGLE;

/// \~chinese 图像旋转结构体            \~english Rotate image structure
typedef struct _MV_CC_ROTATE_IMAGE_PARAM_T_
{
	enum MvGvspPixelType    enPixelType;                            ///< [IN]  \~chinese 像素格式               \~english Pixel format
	unsigned int            nWidth;                                 ///< [IN][OUT] \~chinese 图像宽             \~english Width
	unsigned int            nHeight;                                ///< [IN][OUT] \~chinese 图像高             \~english Height

	unsigned char*          pSrcData;                               ///< [IN]  \~chinese 输入数据缓存           \~english Input data buffer
	unsigned int            nSrcDataLen;                            ///< [IN]  \~chinese 输入数据长度           \~english Input data length

	unsigned char*          pDstBuf;                                ///< [OUT] \~chinese 输出数据缓存           \~english Output data buffer
	unsigned int            nDstBufLen;                             ///< [OUT] \~chinese 输出数据长度           \~english Output data length
	unsigned int            nDstBufSize;                            ///< [IN]  \~chinese 提供的输出缓冲区大小   \~english Provided output buffer size

	MV_IMG_ROTATION_ANGLE   enRotationAngle;                        ///< [IN]  \~chinese 旋转角度               \~english Rotation angle

	unsigned int            nRes[8];                                ///<       \~chinese 预留                   \~english Reserved

}MV_CC_ROTATE_IMAGE_PARAM;

/// \~chinese 翻转类型                  \~english Flip type
typedef enum _MV_IMG_FLIP_TYPE_
{
	MV_FLIP_VERTICAL                    = 1,
	MV_FLIP_HORIZONTAL                  = 2,

}MV_IMG_FLIP_TYPE;

/// \~chinese 图像翻转结构体            \~english Flip image structure
typedef struct _MV_CC_FLIP_IMAGE_PARAM_T_
{
	enum MvGvspPixelType    enPixelType;                            ///< [IN]  \~chinese 像素格式               \~english Pixel format
	unsigned int            nWidth;                                 ///< [IN]  \~chinese 图像宽                 \~english Width
	unsigned int            nHeight;                                ///< [IN]  \~chinese 图像高                 \~english Height

	unsigned char*          pSrcData;                               ///< [IN]  \~chinese 输入数据缓存           \~english Input data buffer
	unsigned int            nSrcDataLen;                            ///< [IN]  \~chinese 输入数据长度           \~english Input data length

	unsigned char*          pDstBuf;                                ///< [OUT] \~chinese 输出数据缓存           \~english Output data buffer
	unsigned int            nDstBufLen;                             ///< [OUT] \~chinese 输出数据长度           \~english Output data length
	unsigned int            nDstBufSize;                            ///< [IN]  \~chinese 提供的输出缓冲区大小   \~english Provided output buffer size

	MV_IMG_FLIP_TYPE        enFlipType;                             ///< [IN]  \~chinese 翻转类型               \~english Flip type

	unsigned int            nRes[8];                                ///<       \~chinese 预留                   \~english Reserved

}MV_CC_FLIP_IMAGE_PARAM;


/// \~chinese 图像转换结构体     \~english Pixel convert structure
typedef struct _MV_PIXEL_CONVERT_PARAM_T_
{
    unsigned short      nWidth;             ///< [IN]     \~chinese 图像宽           \~english Width
    unsigned short      nHeight;            ///< [IN]     \~chinese 图像高\~english Height

    enum MvGvspPixelType    enSrcPixelType;     ///< [IN]     \~chinese 源像素格式\~english Source pixel format
    unsigned char*      pSrcData;           ///< [IN]     \~chinese 输入数据缓存\~english Input data buffer
    unsigned int        nSrcDataLen;        ///< [IN]     \~chinese 输入数据大小\~english Input data size

    enum MvGvspPixelType    enDstPixelType;     ///< [IN]     \~chinese 目标像素格式\~english Destination pixel format
    unsigned char*      pDstBuffer;         ///< [OUT]    \~chinese 输出数据缓存\~english Output data buffer
    unsigned int        nDstLen;            ///< [OUT]    \~chinese 输出数据大小\~english Output data size
    unsigned int        nDstBufferSize;     ///< [IN]     \~chinese 提供的输出缓冲区大小\~english Provided outbut buffer size

    unsigned int        nRes[4];
}MV_CC_PIXEL_CONVERT_PARAM;

/// \~chinese Gamma类型                 \~english Gamma type
typedef enum _MV_CC_GAMMA_TYPE_
{
	MV_CC_GAMMA_TYPE_NONE               = 0,                        ///< \~chinese 不启用                       \~english Disable
	MV_CC_GAMMA_TYPE_VALUE              = 1,                        ///< \~chinese Gamma值                      \~english Gamma value
	MV_CC_GAMMA_TYPE_USER_CURVE         = 2,                        ///< \~chinese Gamma曲线                    \~english Gamma curve
	///< \~chinese 长度：256*sizeof(unsigned char)      \~english 8bit,length:256*sizeof(unsigned char)
	MV_CC_GAMMA_TYPE_LRGB2SRGB          = 3,                        ///< \~chinese linear RGB to sRGB           \~english linear RGB to sRGB
	MV_CC_GAMMA_TYPE_SRGB2LRGB          = 4,                        ///< \~chinese sRGB to linear RGB(仅色彩插值时支持，色彩校正时无效) \~english sRGB to linear RGB

}MV_CC_GAMMA_TYPE;

// Gamma信息
/// \~chinese Gamma信息结构体           \~english Gamma info structure
typedef struct _MV_CC_GAMMA_PARAM_T_
{
	MV_CC_GAMMA_TYPE    enGammaType;                                ///< [IN]  \~chinese Gamma类型              \~english Gamma type
	float               fGammaValue;                                ///< [IN]  \~chinese Gamma值[0.1,4.0]       \~english Gamma value[0.1,4.0]
	unsigned char*      pGammaCurveBuf;                             ///< [IN]  \~chinese Gamma曲线缓存          \~english Gamma curve buffer
	unsigned int        nGammaCurveBufLen;                          ///< [IN]  \~chinese Gamma曲线长度          \~english Gamma curve buffer size

	unsigned int        nRes[8];                                    ///<       \~chinese 预留                   \~english Reserved

}MV_CC_GAMMA_PARAM;

/// \~chinese 水印信息                  \~english  Frame-specific information
typedef struct _MV_CC_FRAME_SPEC_INFO_
{
	/// \~chinese 设备水印时标      \~english Device frame-specific time scale
	unsigned int        nSecondCount;                               ///< [OUT] \~chinese 秒数                   \~english The Seconds
	unsigned int        nCycleCount;                                ///< [OUT] \~chinese 周期数                 \~english The Count of Cycle
	unsigned int        nCycleOffset;                               ///< [OUT] \~chinese 周期偏移量             \~english The Offset of Cycle

	float               fGain;                                      ///< [OUT] \~chinese 增益                   \~english Gain
	float               fExposureTime;                              ///< [OUT] \~chinese 曝光时间               \~english Exposure Time
	unsigned int        nAverageBrightness;                         ///< [OUT] \~chinese 平均亮度               \~english Average brightness

	/// \~chinese 白平衡相关        \~english White balance
	unsigned int        nRed;                                       ///< [OUT] \~chinese 红色                   \~english Red
	unsigned int        nGreen;                                     ///< [OUT] \~chinese 绿色                   \~english Green
	unsigned int        nBlue;                                      ///< [OUT] \~chinese 蓝色                   \~english Blue

	unsigned int        nFrameCounter;                              ///< [OUT] \~chinese 总帧数                 \~english Frame Counter
	unsigned int        nTriggerIndex;                              ///< [OUT] \~chinese 触发计数               \~english Trigger Counting

	unsigned int        nInput;                                     ///< [OUT] \~chinese 输入                   \~english Input
	unsigned int        nOutput;                                    ///< [OUT] \~chinese 输出                   \~english Output

	/// \~chinese ROI区域           \~english ROI Region
	unsigned short      nOffsetX;                                   ///< [OUT] \~chinese 水平偏移量             \~english OffsetX
	unsigned short      nOffsetY;                                   ///< [OUT] \~chinese 垂直偏移量             \~english OffsetY
	unsigned short      nFrameWidth;                                ///< [OUT] \~chinese 水印宽                 \~english The Width of Chunk
	unsigned short      nFrameHeight;                               ///< [OUT] \~chinese 水印高                 \~english The Height of Chunk

	unsigned int        nReserved[16];                              ///<       \~chinese 预留                   \~english Reserved

}MV_CC_FRAME_SPEC_INFO;

/// \~chinese 无损解码参数              \~english High Bandwidth decode structure
typedef struct _MV_CC_HB_DECODE_PARAM_T_
{
	unsigned char*          pSrcBuf;                                ///< [IN]  \~chinese 输入数据缓存           \~english Input data buffer
	unsigned int            nSrcLen;                                ///< [IN]  \~chinese 输入数据大小           \~english Input data size

	unsigned int            nWidth;                                 ///< [OUT] \~chinese 图像宽                 \~english Width
	unsigned int            nHeight;                                ///< [OUT] \~chinese 图像高                 \~english Height
	unsigned char*          pDstBuf;                                ///< [OUT] \~chinese 输出数据缓存           \~english Output data buffer
	unsigned int            nDstBufSize;                            ///< [IN]  \~chinese 提供的输出缓冲区大小   \~english Provided output buffer size
	unsigned int            nDstBufLen;                             ///< [OUT] \~chinese 输出数据大小           \~english Output data size
	enum MvGvspPixelType    enDstPixelType;                         ///< [OUT] \~chinese 输出的像素格式         \~english Output pixel format

	MV_CC_FRAME_SPEC_INFO   stFrameSpecInfo;                        ///< [OUT] \~chinese 水印信息               \~english Frame Spec Info

	unsigned int            nRes[8];                                ///<       \~chinese 预留                   \~english Reserved

}MV_CC_HB_DECODE_PARAM;

/// \~chinese 录像格式定义\~english Record Format Type
typedef enum _MV_RECORD_FORMAT_TYPE_
{
    MV_FormatType_Undefined     = 0,
    MV_FormatType_AVI           = 1,

}MV_RECORD_FORMAT_TYPE;

/// \~chinese 录像参数\~english Record Parameters
typedef struct _MV_CC_RECORD_PARAM_T_
{
    enum MvGvspPixelType    enPixelType;    ///< [IN]     \~chinese 输入数据的像素格式

    unsigned short      nWidth;             ///< [IN]     \~chinese 图像宽(指定目标参数时需为2的倍数)
    unsigned short      nHeight;            ///< [IN]     \~chinese 图像高(指定目标参数时需为2的倍数)

    float               fFrameRate;         ///< [IN]     \~chinese 帧率fps(1/16-120)
    unsigned int        nBitRate;           ///< [IN]     \~chinese 码率kbps(128kbps-16Mbps)

    MV_RECORD_FORMAT_TYPE enRecordFmtType;  ///< [IN]     \~chinese 录像格式

    char*               strFilePath;        ///< [IN]     \~chinese 录像文件存放路径(如果路径中存在中文，需转成utf-8)

    unsigned int        nRes[8];

}MV_CC_RECORD_PARAM;

/// \~chinese 录像数据\~english Record Data
typedef struct _MV_CC_INPUT_FRAME_INFO_T_
{
    unsigned char*      pData;              ///< [IN]     \~chinese 图像数据指针
    unsigned int        nDataLen;           ///< [IN]     \~chinese 图像大小

    unsigned int        nRes[8];

}MV_CC_INPUT_FRAME_INFO;

/// \~chinese 采集模式\~english Acquisition mode
typedef enum _MV_CAM_ACQUISITION_MODE_
{
    MV_ACQ_MODE_SINGLE      = 0,            ///< \~chinese 单帧模式\~english Single Mode
    MV_ACQ_MODE_MUTLI       = 1,            ///< \~chinese 多帧模式\~english Multi Mode
    MV_ACQ_MODE_CONTINUOUS  = 2,            ///< \~chinese 持续采集模式\~english Continuous Mode

}MV_CAM_ACQUISITION_MODE;

/// \~chinese 增益模式\~english Gain Mode
typedef enum _MV_CAM_GAIN_MODE_
{
    MV_GAIN_MODE_OFF        = 0,            ///< \~chinese 关闭\~english Single Mode
    MV_GAIN_MODE_ONCE       = 1,            ///< \~chinese 一次\~english Multi Mode
    MV_GAIN_MODE_CONTINUOUS = 2,            ///< \~chinese 连续\~english Continuous Mode

}MV_CAM_GAIN_MODE;

/// \~chinese 曝光模式\~english Exposure Mode
typedef enum _MV_CAM_EXPOSURE_MODE_
{
    MV_EXPOSURE_MODE_TIMED          = 0,            ///< Timed
    MV_EXPOSURE_MODE_TRIGGER_WIDTH  = 1,            ///< TriggerWidth
}MV_CAM_EXPOSURE_MODE;

/// \~chinese 自动曝光模式          \~english Auto Exposure Mode
typedef enum _MV_CAM_EXPOSURE_AUTO_MODE_
{
    MV_EXPOSURE_AUTO_MODE_OFF        = 0,            ///< \~chinese 关闭\~english Off
    MV_EXPOSURE_AUTO_MODE_ONCE       = 1,            ///< \~chinese 一次\~english Once
    MV_EXPOSURE_AUTO_MODE_CONTINUOUS = 2,            ///< \~chinese 连续\~english Continuous

}MV_CAM_EXPOSURE_AUTO_MODE;

/// \~chinese 触发模式          \~english Trigger Mode
typedef enum _MV_CAM_TRIGGER_MODE_
{
    MV_TRIGGER_MODE_OFF         = 0,            ///< \~chinese 关闭\~english Off
    MV_TRIGGER_MODE_ON          = 1,            ///< \~chinese 打开\~english ON

}MV_CAM_TRIGGER_MODE;

typedef enum _MV_CAM_GAMMA_SELECTOR_
{
    MV_GAMMA_SELECTOR_USER      = 1,
    MV_GAMMA_SELECTOR_SRGB      = 2,

}MV_CAM_GAMMA_SELECTOR;

typedef enum _MV_CAM_BALANCEWHITE_AUTO_
{
    MV_BALANCEWHITE_AUTO_OFF            = 0,
    MV_BALANCEWHITE_AUTO_ONCE           = 2,
    MV_BALANCEWHITE_AUTO_CONTINUOUS     = 1,            ///< \~chinese 连续\~english Continuous

}MV_CAM_BALANCEWHITE_AUTO;

typedef enum _MV_CAM_TRIGGER_SOURCE_
{
    MV_TRIGGER_SOURCE_LINE0             = 0,
    MV_TRIGGER_SOURCE_LINE1             = 1,
    MV_TRIGGER_SOURCE_LINE2             = 2,
    MV_TRIGGER_SOURCE_LINE3             = 3,
    MV_TRIGGER_SOURCE_COUNTER0          = 4,

    MV_TRIGGER_SOURCE_SOFTWARE          = 7,
    MV_TRIGGER_SOURCE_FrequencyConverter= 8,

}MV_CAM_TRIGGER_SOURCE;

typedef enum _MV_GIGE_TRANSMISSION_TYPE_
{
    MV_GIGE_TRANSTYPE_UNICAST                = 0x0,                ///< \~chinese 表示单播(默认)\~english Unicast mode
    MV_GIGE_TRANSTYPE_MULTICAST              = 0x1,                ///< \~chinese 表示组播\~english Multicast mode
    MV_GIGE_TRANSTYPE_LIMITEDBROADCAST       = 0x2,                ///< \~chinese 表示局域网内广播，暂不支持\~english Limited broadcast mode,not support
    MV_GIGE_TRANSTYPE_SUBNETBROADCAST        = 0x3,                ///< \~chinese 表示子网内广播，暂不支持\~english Subnet broadcast mode,not support
    MV_GIGE_TRANSTYPE_CAMERADEFINED          = 0x4,                ///< \~chinese 表示从相机获取，暂不支持\~english Transtype from camera,not support
    MV_GIGE_TRANSTYPE_UNICAST_DEFINED_PORT   = 0x5,                ///< \~chinese 表示用户自定义应用端接收图像数据Port号\~english User Defined Receive Data Port
    MV_GIGE_TRANSTYPE_UNICAST_WITHOUT_RECV   = 0x00010000,         ///< \~chinese 表示设置了单播，但本实例不接收图像数据\~english Unicast without receive data
    MV_GIGE_TRANSTYPE_MULTICAST_WITHOUT_RECV = 0x00010001,         ///< \~chinese 表示组播模式，但本实例不接收图像数据\~english Multicast without receive data
}MV_GIGE_TRANSMISSION_TYPE;
// GigEVision IP Configuration
#define MV_IP_CFG_STATIC        0x05000000
#define MV_IP_CFG_DHCP          0x06000000
#define MV_IP_CFG_LLA           0x04000000

// GigEVision Net Transfer Mode
#define MV_NET_TRANS_DRIVER     0x00000001
#define MV_NET_TRANS_SOCKET     0x00000002

// CameraLink Baud Rates (CLUINT32)
#define MV_CAML_BAUDRATE_9600             0x00000001
#define MV_CAML_BAUDRATE_19200           0x00000002
#define MV_CAML_BAUDRATE_38400           0x00000004
#define MV_CAML_BAUDRATE_57600           0x00000008
#define MV_CAML_BAUDRATE_115200          0x00000010
#define MV_CAML_BAUDRATE_230400          0x00000020
#define MV_CAML_BAUDRATE_460800          0x00000040
#define MV_CAML_BAUDRATE_921600          0x00000080
#define MV_CAML_BAUDRATE_AUTOMAX         0x40000000

/// \~chinese 信息类型\~english Information Type
#define MV_MATCH_TYPE_NET_DETECT             0x00000001      ///< \~chinese 网络流量和丢包信息\~english Network traffic and packet loss information
#define MV_MATCH_TYPE_USB_DETECT             0x00000002      ///< \~chinese host接收到来自U3V设备的字节总数\~english The total number of bytes host received from U3V device

/// \~chinese 某个节点对应的子节点个数最大值\~english The maximum number of child nodes corresponding to a node
#define MV_MAX_XML_NODE_NUM_C       128

/// \~chinese 节点名称字符串最大长度\~english The maximum length of node name string
#define MV_MAX_XML_NODE_STRLEN_C    64

/// \~chinese 节点String值最大长度\~english The maximum length of Node String
#define MV_MAX_XML_STRVALUE_STRLEN_C 64

/// \~chinese 节点描述字段最大长度\~english The maximum length of the node description field
#define MV_MAX_XML_DISC_STRLEN_C    512

/// \~chinese 最多的单元数\~englishThe maximum number of units
#define MV_MAX_XML_ENTRY_NUM        10

/// \~chinese 父节点个数上限\~english The maximum number of parent nodes
#define MV_MAX_XML_PARENTS_NUM      8

/// \~chinese 每个已经实现单元的名称长度\~english The length of the name of each unit that has been implemented
#define MV_MAX_XML_SYMBOLIC_STRLEN_C 64

#define MV_MAX_XML_SYMBOLIC_NUM      64


/// \~chinese 全匹配的一种信息结构体\~english A fully matched information structure
typedef struct _MV_ALL_MATCH_INFO_
{
    unsigned int        nType;          ///< \~chinese 需要输出的信息类型，e.g. MV_MATCH_TYPE_NET_DETECT\~english Information type need to output ,e.g. MV_MATCH_TYPE_NET_DETECT
    void*               pInfo;          ///< \~chinese 输出的信息缓存，由调用者分配\~english Output information cache, which is allocated by the caller
    unsigned int        nInfoSize;      ///< \~chinese 信息缓存的大小\~english Information cache size

}MV_ALL_MATCH_INFO;



/// \~chinese 网络流量和丢包信息反馈结构体，对应类型为 MV_MATCH_TYPE_NET_DETECT
/// \~en:Network traffic and packet loss feedback structure, the corresponding type is MV_MATCH_TYPE_NET_DETECT
typedef struct _MV_MATCH_INFO_NET_DETECT_
{
    int64_t             nReviceDataSize;    ///< \~chinese 已接收数据大小  [统计StartGrabbing和StopGrabbing之间的数据量]\~english Received data size 
    int64_t             nLostPacketCount;   ///< \~chinese 丢失的包数量\~english Number of packets lost
    unsigned int        nLostFrameCount;    ///< \~chinese 丢帧数量\~english Number of frames lost
    unsigned int        nNetRecvFrameCount;          ///< \~chinese 保留\~english Reserved
    int64_t             nRequestResendPacketCount;          ///< \~chinese 请求重发包数
    int64_t             nResendPacketCount;   ///< \~chinese 重发包数
}MV_MATCH_INFO_NET_DETECT;

/// \~chinese host收到从u3v设备端的总字节数，对应类型为 MV_MATCH_TYPE_USB_DETECT\~english The total number of bytes host received from the u3v device side, the corresponding type is MV_MATCH_TYPE_USB_DETECT
typedef struct _MV_MATCH_INFO_USB_DETECT_
{
    int64_t             nReviceDataSize;      ///< \~chinese 已接收数据大小    [统计OpenDevicce和CloseDevice之间的数据量]\~english Received data size
    unsigned int        nRevicedFrameCount;   ///< \~chinese 已收到的帧数\~english Number of frames received
    unsigned int        nErrorFrameCount;     ///< \~chinese 错误帧数\~english Number of error frames
    unsigned int        nReserved[2];         ///< \~chinese 保留\~english Reserved
}MV_MATCH_INFO_USB_DETECT;

typedef struct _MV_IMAGE_BASIC_INFO_
{
    // width
    unsigned short      nWidthValue;
    unsigned short      nWidthMin;
    unsigned int        nWidthMax;
    unsigned int        nWidthInc;

    // height
    unsigned int        nHeightValue;
    unsigned int        nHeightMin;
    unsigned int        nHeightMax;
    unsigned int        nHeightInc;

    // framerate
    float               fFrameRateValue;
    float               fFrameRateMin;
    float               fFrameRateMax;

    /// \~chinese 像素格式\~english pixel format
    unsigned int        enPixelType;                 ///< \~chinese 当前的像素格式\~english Current pixel format
    unsigned int        nSupportedPixelFmtNum;       ///< \~chinese 支持的像素格式种类\~english Support pixel format
    unsigned int        enPixelList[MV_MAX_XML_SYMBOLIC_NUM];
    unsigned int        nReserved[8];

}MV_IMAGE_BASIC_INFO;

/// \~chinese 异常消息类型\~english Exception message type
#define MV_EXCEPTION_DEV_DISCONNECT     0x00008001      ///< \~chinese 设备断开连接\~english The device is disconnected
#define MV_EXCEPTION_VERSION_CHECK      0x00008002      ///< \~chinese SDK与驱动版本不匹配\~english SDK does not match the driver version
/// \~chinese 设备的访问模式\~english Device Access Mode
/// \~chinese 独占权限，其他APP只允许读CCP寄存器\~english Exclusive authority, other APP is only allowed to read the CCP register
#define MV_ACCESS_Exclusive                                         1
/// \~chinese 可以从5模式下抢占权限，然后以独占权限打开\~english You can seize the authority from the 5 mode, and then open with exclusive authority
#define MV_ACCESS_ExclusiveWithSwitch                               2
/// \~chinese 控制权限，其他APP允许读所有寄存器\~english Control authority, allows other APP reading all registers
#define MV_ACCESS_Control                                           3
/// \~chinese 可以从5的模式下抢占权限，然后以控制权限打开\~english You can seize the authority from the 5 mode, and then open with control authority
#define MV_ACCESS_ControlWithSwitch                                 4
/// \~chinese 以可被抢占的控制权限打开\~english Open with seized control authority
#define MV_ACCESS_ControlSwitchEnable                               5
/// \~chinese 可以从5的模式下抢占权限，然后以可被抢占的控制权限打开\~english You can seize the authority from the 5 mode, and then open with seized control authority
#define MV_ACCESS_ControlSwitchEnableWithKey                        6
/// \~chinese 读模式打开设备，适用于控制权限下\~english Open with read mode and is available under control authority
#define MV_ACCESS_Monitor                                           7


/************************************************************************/
/* 封装了GenICam的C接口相关参数定义                                     */
/* Package of GenICam C interface-related parameters definition         */
/************************************************************************/

/// \~chinese 每个节点对应的接口类型\~english Interface type corresponds to each node 
enum MV_XML_InterfaceType
{
    IFT_IValue,         //!> IValue interface
    IFT_IBase,          //!> IBase interface
    IFT_IInteger,       //!> IInteger interface
    IFT_IBoolean,       //!> IBoolean interface
    IFT_ICommand,       //!> ICommand interface
    IFT_IFloat,         //!> IFloat interface
    IFT_IString,        //!> IString interface
    IFT_IRegister,      //!> IRegister interface
    IFT_ICategory,      //!> ICategory interface
    IFT_IEnumeration,   //!> IEnumeration interface
    IFT_IEnumEntry,     //!> IEnumEntry interface
    IFT_IPort,          //!> IPort interface
};

/// \~chinese 节点的访问模式\~english Node Access Mode
enum MV_XML_AccessMode
{
    AM_NI,          //!< Not implemented
    AM_NA,          //!< Not available
    AM_WO,          //!< Write Only
    AM_RO,          //!< Read Only
    AM_RW,          //!< Read and Write
    AM_Undefined,   //!< Object is not yet initialized
    AM_CycleDetect, //!< used internally for AccessMode cycle detection
};

/// \~chinese 节点的可见性权限\~english Node Visible Permission
enum MV_XML_Visibility
{
    V_Beginner      = 0,    //!< Always visible
    V_Expert        = 1,    //!< Visible for experts or Gurus
    V_Guru          = 2,    //!< Visible for Gurus
    V_Invisible     = 3,    //!< Not Visible
    V_Undefined     = 99    //!< Object is not yet initialized
};

/// \~chinese Event事件回调信息\~english Event callback infomation
#define MAX_EVENT_NAME_SIZE     128     // 相机Event事件名称最大长度\~english Max length of event name

typedef struct _MV_EVENT_OUT_INFO_
{
    char                EventName[MAX_EVENT_NAME_SIZE];     ///< \~chinese Event名称\~english Event name

    unsigned short      nEventID;                           ///< \~chinese Event号\~english Event ID
    unsigned short      nStreamChannel;                     ///< \~chinese 流通道序号\~english Circulation number

    unsigned int        nBlockIdHigh;                       ///< \~chinese 帧号高位\~english BlockId high
    unsigned int        nBlockIdLow;                        ///< \~chinese 帧号低位\~english BlockId low

    unsigned int        nTimestampHigh;                     ///< \~chinese 时间戳高位\~english Timestramp high
    unsigned int        nTimestampLow;                      ///< \~chinese 时间戳低位\~english Timestramp low

    void*               pEventData;                         ///< \~chinese Event数据\~english Event data
    unsigned int        nEventDataSize;                     ///< \~chinese Event数据长度\~english Event data len

    unsigned int        nReserved[16];                      ///< \~chinese 预留\~english Reserved
}MV_EVENT_OUT_INFO;

/// \~chinese 文件存取\~english File Access
typedef struct _MV_CC_FILE_ACCESS_T
{
    const char*         pUserFileName;                       ///< \~chinese 用户文件名\~english User file name
    const char*         pDevFileName;                        ///< \~chinese 设备文件名\~english Device file name

    unsigned int        nReserved[32];                      ///< \~chinese 预留\~english Reserved
}MV_CC_FILE_ACCESS;

/// \~chinese 文件存取进度\~english File Access Progress
typedef struct _MV_CC_FILE_ACCESS_PROGRESS_T
{
    int64_t             nCompleted;                          ///< \~chinese 已完成的长度\~english Completed Length
    int64_t             nTotal;                              ///< \~chinese 总长度\~english Total Length

    unsigned int        nReserved[8];                      ///< \~chinese 预留\~english Reserved
}MV_CC_FILE_ACCESS_PROGRESS;



/// \~chinese 传输模式，可以为单播模式、组播模式等\~english Transmission type
typedef struct _MV_TRANSMISSION_TYPE_T
{
    MV_GIGE_TRANSMISSION_TYPE   enTransmissionType; ///< \~chinese 传输模式\~english Transmission type
    unsigned int        nDestIp;            ///< \~chinese 目标IP，组播模式下有意义\~english Destination IP
    unsigned short      nDestPort;          ///< \~chinese 目标Port，组播模式下有意义\~english Destination port

    unsigned int        nReserved[32];                      ///< \~chinese 预留\~english Reserved
}MV_TRANSMISSION_TYPE;

/// \~chinese 动作命令信息\~english Action Command
typedef struct _MV_ACTION_CMD_INFO_T
{
    unsigned int        nDeviceKey;        ///< \~chinese 设备密钥
    unsigned int        nGroupKey;         ///< \~chinese 组键
    unsigned int        nGroupMask;        ///< \~chinese 组掩码

    unsigned int        bActionTimeEnable; ///< \~chinese 只有设置成1时Action Time才有效，非1时无效
    int64_t             nActionTime;       ///< \~chinese 预定的时间，和主频有关

    const char*         pBroadcastAddress; ///< \~chinese 广播包地址
    unsigned int        nTimeOut;          ///< \~chinese 等待ACK的超时时间，如果为0表示不需要ACK

    unsigned int        bSpecialNetEnable;                          ///< [IN]  \~chinese 只有设置成1时指定的网卡IP才有效，非1时无效 \~english Special IP Enable
    unsigned int        nSpecialNetIP;                              ///< [IN]  \~chinese 指定的网卡IP                               \~english Special Net IP address

    unsigned int        nReserved[14];     ///< \~chinese 预留\~english Reserved

}MV_ACTION_CMD_INFO;

/// \~chinese 动作命令返回信息\~english Action Command Result
typedef struct _MV_ACTION_CMD_RESULT_T
{
    unsigned char       strDeviceAddress[12 + 3 + 1];  ///< IP address of the device

    //status code returned by the device
    int                 nStatus;//1.0x0000:success.
                                //2.0x8001:Command is not supported by the device.
                                //3.0x8013:The device is not synchronized to a master clock to be used as time reference.
                                //4.0x8015:A device queue or packet data has overflowed.
                                //5.0x8016:The requested scheduled action command was requested at a time that is already past.

    unsigned int        nReserved[4];         ///< \~chinese 预留\~english Reserved

}MV_ACTION_CMD_RESULT;

/// \~chinese 动作命令返回信息列表\~english Action Command Result List
typedef struct _MV_ACTION_CMD_RESULT_LIST_T
{
    unsigned int        nNumResults;      ///< \~chinese 返回值个数
    MV_ACTION_CMD_RESULT*   pResults;

}MV_ACTION_CMD_RESULT_LIST;

// ch:单个节点基本属性 | en:Single Node Basic Attributes
typedef struct _MV_XML_NODE_FEATURE_
{
    enum MV_XML_InterfaceType    enType;                                 ///< \~chinese 节点类型\~english Node Type
    enum MV_XML_Visibility       enVisivility;                           ///< \~chinese 是否可见\~english Is visibility
    char                    strDescription[MV_MAX_XML_DISC_STRLEN_C];    ///< \~chinese 节点描述，目前暂不支持\~english Node Description, NOT SUPPORT NOW
    char                    strDisplayName[MV_MAX_XML_NODE_STRLEN_C];    ///< \~chinese 显示名称\~english Display Name
    char                    strName[MV_MAX_XML_NODE_STRLEN_C];           ///< \~chinese 节点名\~english Node Name
    char                    strToolTip[MV_MAX_XML_DISC_STRLEN_C];        ///< \~chinese 提示\~english Notice

    unsigned int            nReserved[4];
}MV_XML_NODE_FEATURE;

/// \~chinese 节点列表\~english Node List
typedef struct _MV_XML_NODES_LIST_
{
    unsigned int            nNodeNum;                                    ///< \~chinese 节点个数\~english Node Number
    MV_XML_NODE_FEATURE     stNodes[MV_MAX_XML_NODE_NUM_C];
}MV_XML_NODES_LIST;



typedef struct _MV_XML_FEATURE_Value_
{
    enum MV_XML_InterfaceType    enType;                                ///< \~chinese节点类型\~english Node Type
    char                    strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese节点描述     目前暂不支持\~english Node Description, NOT SUPPORT NOW
    char                    strDisplayName[MV_MAX_XML_NODE_STRLEN_C];   ///< \~chinese 显示名称\~english Display Name
    char                    strName[MV_MAX_XML_NODE_STRLEN_C];          ///< \~chinese 节点名\~english Node Name
    char                    strToolTip[MV_MAX_XML_DISC_STRLEN_C];       ///< \~chinese 提示\~english Notice
    unsigned int            nReserved[4];
}MV_XML_FEATURE_Value;

typedef struct _MV_XML_FEATURE_Base_
{
    enum MV_XML_AccessMode   enAccessMode;       ///< \~chinese 访问模式\~english Access Mode
}MV_XML_FEATURE_Base;

typedef struct _MV_XML_FEATURE_Integer_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility   enVisivility;                      ///< \~chinese是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                      ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                              ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    int64_t             nValue;                                 ///< \~chinese 当前值\~english Current Value
    int64_t             nMinValue;                              ///< \~chinese 最小值\~english Min Value
    int64_t             nMaxValue;                              ///< \~chinese 最大值\~english Max Value
    int64_t             nIncrement;                             ///< \~chinese 增量\~english Increment

    unsigned int        nReserved[4];

}MV_XML_FEATURE_Integer;

typedef struct _MV_XML_FEATURE_Boolean_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility   enVisivility;                          ///< \~chinese 是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                          ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                  ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    bool                bValue;                                     ///< \~chinese 当前值\~english Current Value

    unsigned int        nReserved[4];
}MV_XML_FEATURE_Boolean;

typedef struct _MV_XML_FEATURE_Command_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility   enVisivility;                          ///< \~chinese 是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                          ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                  ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW

    unsigned int        nReserved[4];
}MV_XML_FEATURE_Command;

typedef struct _MV_XML_FEATURE_Float_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility       enVisivility;                     ///< \~chinese 是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                         ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                 ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    double              dfValue;                                   ///< \~chinese 当前值\~english Current Value
    double              dfMinValue;                                ///< \~chinese 最小值\~english Min Value
    double              dfMaxValue;                                ///< \~chinese 最大值\~english Max Value
    double              dfIncrement;                               ///< \~chinese 增量\~english Increment

    unsigned int        nReserved[4];
}MV_XML_FEATURE_Float;

typedef struct _MV_XML_FEATURE_String_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility       enVisivility;                       ///< \~chinese 是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                           ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                   ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    char                strValue[MV_MAX_XML_STRVALUE_STRLEN_C];      ///< \~chinese 当前值\~english Current Value

    unsigned int        nReserved[4];
}MV_XML_FEATURE_String;

typedef struct _MV_XML_FEATURE_Register_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];

    enum MV_XML_Visibility       enVisivility;                      ///< \~chinese 是否可见\~english Visible
    enum MV_XML_AccessMode   enAccessMode;                          ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                  ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    int64_t             nAddrValue;                                 ///< \~chinese 当前值\~english Current Value

    unsigned int        nReserved[4];
}MV_XML_FEATURE_Register;

typedef struct _MV_XML_FEATURE_Category_
{
    char                    strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 节点描述 目前暂不支持\~english Node Description, NOT SUPPORT NOW
    char                    strDisplayName[MV_MAX_XML_NODE_STRLEN_C];   ///< \~chinese 显示名称\~english Display Name
    char                    strName[MV_MAX_XML_NODE_STRLEN_C];          ///< \~chinese 节点名\~english Node Name
    char                    strToolTip[MV_MAX_XML_DISC_STRLEN_C];       ///< \~chinese 提示\~english Notice

    enum MV_XML_Visibility       enVisivility;                          ///< \~chinese 是否可见\~english Visible

    unsigned int            nReserved[4];
}MV_XML_FEATURE_Category;

typedef struct _MV_XML_FEATURE_EnumEntry_
{
    char                strName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDisplayName[MV_MAX_XML_NODE_STRLEN_C];
    char                strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 目前暂不支持\~english NOT SUPPORT NOW
    char                strToolTip[MV_MAX_XML_DISC_STRLEN_C];
    int                 bIsImplemented;
    int                 nParentsNum;
    MV_XML_NODE_FEATURE stParentsList[MV_MAX_XML_PARENTS_NUM];

    enum MV_XML_Visibility       enVisivility;                      ///< \~chinese 是否可见\~english Visible
    int64_t             nValue;                                     ///< \~chinese 当前值\~english Current Value
    enum MV_XML_AccessMode   enAccessMode;                          ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                  ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    int                 nReserved[8];

}MV_XML_FEATURE_EnumEntry;


typedef struct _MV_XML_FEATURE_Enumeration_
{
    enum MV_XML_Visibility       enVisivility;                          ///< \~chinese 是否可见\~english Visible
    char                    strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 节点描述 目前暂不支持\~english Node Description, NOT SUPPORT NOW
    char                    strDisplayName[MV_MAX_XML_NODE_STRLEN_C];   ///< \~chinese 显示名称\~english Display Name
    char                    strName[MV_MAX_XML_NODE_STRLEN_C];          ///< \~chinese 节点名\~english Node Name
    char                    strToolTip[MV_MAX_XML_DISC_STRLEN_C];       ///< \~chinese 提示\~english Notice

    int                 nSymbolicNum;                                   ///< \~chinese Symbolic数\~english Symbolic Number
    char                strCurrentSymbolic[MV_MAX_XML_SYMBOLIC_STRLEN_C];     ///< \~chinese 当前Symbolic索引\~english Current Symbolic Index
    char                strSymbolic[MV_MAX_XML_SYMBOLIC_NUM][MV_MAX_XML_SYMBOLIC_STRLEN_C];
    enum MV_XML_AccessMode   enAccessMode;                               ///< \~chinese 访问模式\~english Access Mode
    int                 bIsLocked;                                       ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW
    int64_t             nValue;                                          ///< \~chinese 当前值\~english Current Value

    unsigned int        nReserved[4];
}MV_XML_FEATURE_Enumeration;


typedef struct _MV_XML_FEATURE_Port_
{
    enum MV_XML_Visibility       enVisivility;                          ///< \~chinese 是否可见\~english Visible
    char                    strDescription[MV_MAX_XML_DISC_STRLEN_C];   ///< \~chinese 节点描述 目前暂不支持\~english Node Description, NOT SUPPORT NOW
    char                    strDisplayName[MV_MAX_XML_NODE_STRLEN_C];   ///< \~chinese 显示名称\~english Display Name
    char                    strName[MV_MAX_XML_NODE_STRLEN_C];          ///< \~chinese 节点名\~english Node Name
    char                    strToolTip[MV_MAX_XML_DISC_STRLEN_C];       ///< \~chinese 提示\~english Notice

    enum MV_XML_AccessMode       enAccessMode;                          ///< \~chinese 访问模式\~english Access Mode
    int                     bIsLocked;                                  ///< \~chinese 是否锁定。0-否；1-是    目前暂不支持\~english Locked. 0-NO; 1-YES, NOT SUPPORT NOW

    unsigned int            nReserved[4];
}MV_XML_FEATURE_Port;

typedef struct _MV_XML_CAMERA_FEATURE_
{
    enum MV_XML_InterfaceType    enType;
    union
    {
        MV_XML_FEATURE_Integer      stIntegerFeature;
        MV_XML_FEATURE_Float        stFloatFeature;
        MV_XML_FEATURE_Enumeration  stEnumerationFeature;
        MV_XML_FEATURE_String       stStringFeature;
    }SpecialFeature;

}MV_XML_CAMERA_FEATURE;

typedef struct _MVCC_ENUMVALUE_T
{
    unsigned int        nCurValue;      ///< \~chinese 当前值\~english Current Value
    unsigned int        nSupportedNum;  ///< \~chinese 数据的有效数据个数\~english Number of valid data
    unsigned int        nSupportValue[MV_MAX_XML_SYMBOLIC_NUM];

    unsigned int        nReserved[4];
}MVCC_ENUMVALUE;

typedef struct _MVCC_INTVALUE_T
{
    unsigned int        nCurValue;      ///< \~chinese 当前值\~english Current Value
    unsigned int        nMax;
    unsigned int        nMin;
    unsigned int        nInc;

    unsigned int        nReserved[4];
}MVCC_INTVALUE;

typedef struct _MVCC_INTVALUE_EX_T
{
    int64_t             nCurValue;      ///< \~chinese 当前值\~english Current Value
    int64_t             nMax;
    int64_t             nMin;
    int64_t             nInc;

    unsigned int        nReserved[16];
}MVCC_INTVALUE_EX;

typedef struct _MVCC_FLOATVALUE_T
{
    float               fCurValue;      ///< \~chinese 当前值\~english Current Value
    float               fMax;
    float               fMin;

    unsigned int        nReserved[4];
}MVCC_FLOATVALUE;

typedef struct _MVCC_STRINGVALUE_T
{
    char                chCurValue[256];      ///< \~chinese 当前值\~english Current Value

    int64_t             nMaxLength;
    unsigned int        nReserved[2];
}MVCC_STRINGVALUE;

#endif /* _MV_CAMERA_PARAMS_H_ */
