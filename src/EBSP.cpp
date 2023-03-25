#include "EBSP.h"

namespace HM{

EBSP::EBSP(uint8_t* _buf, int _len): buf(_buf), len(_len){
    GenerateRBSP();
}

bool EBSP::GenerateRBSP(){
    if(!buf) return false;
    
    int rbsp_len = EBSPtoRBSP(buf, len, 1);
    if(rbsp_len < 0) return false;
    
    rbsp.buf = buf;
    rbsp.len = rbsp_len;
    return true;
}

int EBSP::EBSPtoRBSP(uint8_t* stream, int end_bytepos, int begin_bytepos){
    int i, j, count;
    count = 0;

    if(end_bytepos < begin_bytepos)
        return end_bytepos;

    j = begin_bytepos;

    for(i = begin_bytepos; i < end_bytepos; ++i){
        //in NAL unit, 0x000000, 0x000001 or 0x000002 shall not occur at any byte-aligned position
        if(count == ZEROBYTES_SHORTSTARTCODE && stream[i] < 0x03)
            return -1;
        if(count == ZEROBYTES_SHORTSTARTCODE && stream[i] == 0x03){
            //check the 4th byte after 0x000003, except when cabac_zero_word is used, in which case the last three bytes of this NAL unit must be 0x000003
            if((i < end_bytepos-1) && (stream[i+1] > 0x03))
                return -1;
            if(i == end_bytepos-1)
                return j;
            
            ++i;
            count = 0;
        }
        stream[j] = stream[i];//remove 0x03, rewrite steamBuffer
        if(stream[i] == 0x00)
            ++count;
            
        else
            count = 0;
        ++j;
    }
    return j;
}

}