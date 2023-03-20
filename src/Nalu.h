#pragma once
#include <cstdint>
#include "EBSP.h"

namespace HM{

class Nalu{

enum NaluType{
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12,
    #if (MVC_EXTENSION_ENABLE)
    NALU_TYPE_PREFIX   = 14,
    NALU_TYPE_SUB_SPS  = 15,
    NALU_TYPE_SLC_EXT  = 20,
    NALU_TYPE_VDRD     = 24  // View and Dependency Representation Delimiter NAL Unit
    #endif
};

enum NalRefIdc{
    NALU_PRIORITY_HIGHEST     = 3,
    NALU_PRIORITY_HIGH        = 2,
    NALU_PRIORITY_LOW         = 1,
    NALU_PRIORITY_DISPOSABLE  = 0
};

public:
    Nalu(uint8_t * _buf, int _len, int startCodeLen);
    Nalu(const Nalu & nalu);
    ~Nalu();

    Nalu & operator = (const Nalu & nalu);
    void ProcessNalu(VideoParameters* vptr);

private:
    int SetBuf(uint8_t * _buf, int _len);
    void ParseHeader(uint8_t* header);

private:
    uint8_t* buf = nullptr;
    int len = 0;
    
    int startCodeLen = 0;

    int forbidden_bit = 0;
    NalRefIdc nal_ref_idc;
    NaluType nal_unit_type;
    
    EBSP ebsp;
};

}
