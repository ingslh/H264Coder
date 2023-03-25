#include "Nalu.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "ParameterSet.h"
#include "BitStream.h"
#include "VideoParameters.h"


namespace HM{

Nalu::Nalu(uint8_t * _buf, int _len, int _startCodeLen) : startCodeLen(_startCodeLen){  
    SetBuf(_buf, _len);
    ParseHeader(buf + startCodeLen);

    ebsp.buf = buf + startCodeLen + 1;
    ebsp.len = len - startCodeLen - 1;

    auto ret = ebsp.GenerateRBSP();

    if(!ret || forbidden_bit){
        //output error
    }
}

int Nalu::SetBuf(uint8_t * _buf, int _len){
    if(buf != nullptr){
        free(buf);
        buf = nullptr;
    }
    len = _len;

    buf = (uint8_t *)malloc(len);
    memcpy(buf, _buf, len);

    return 0;
}

Nalu::Nalu(const Nalu & nalu){
    *this = nalu;
}

Nalu::~Nalu(){
    if(buf != nullptr){
        free(buf);
        buf = nullptr;
    }
}

Nalu & Nalu::operator = (const Nalu & nalu){
    if(buf != nullptr){
        free(buf);
        buf = nullptr;
    }

    startCodeLen = nalu.startCodeLen;
    len = nalu.len;
    buf = (uint8_t *)malloc(len);
    memcpy(buf, nalu.buf, len);

    forbidden_bit   = nalu.forbidden_bit;
    nal_ref_idc     = nalu.nal_ref_idc;
    nal_unit_type   = nalu.nal_unit_type;

    ebsp = nalu.ebsp;

    return *this;
}

void Nalu::ParseHeader(uint8_t* header){
    uint8_t naluHead = *header;
    forbidden_bit       = (naluHead >> 7) & 1;//1bit
    nal_ref_idc         = NalRefIdc((naluHead >> 5) & 3); //2bit
    nal_unit_type       = NaluType((naluHead >> 0) & 0x1f);//5bit
}

void Nalu::ProcessNalu(VideoParameters* vptr){
    uint8_t* buffStream = ebsp.rbsp.buf;
    int sdop_len = ebsp.rbsp.RBSPtoSODB();
    BitStream bitstream(buffStream, sdop_len);
    
    switch(nal_unit_type){
        case NALU_TYPE_SLICE:
        case NALU_TYPE_IDR:{
            
            break;
        }
        case NALU_TYPE_SPS:{
            auto sps = new seq_parameter_set_rbsp();
            auto ret = ParameterSet::ProcessSPS(sps, &bitstream, vptr);
            if(!sps->Valid)
                delete sps;
            break;
        }
        case NALU_TYPE_PPS:{
            auto pps = new pic_parameter_set_rbsp();
            auto ret = ParameterSet::ProcessPPS(pps, &bitstream, vptr);
            if(!pps->Valid)
                delete pps;
            break;
        }
        case NALU_TYPE_SEI:{
            
            break;
        }
        default:{
            //if (p_Inp->silent == FALSE)
            printf ("Found NALU type %d, len %d undefined, ignore NALU, moving on\n", (int) nal_unit_type, (int) len);
            break;
        }
    }
}


}