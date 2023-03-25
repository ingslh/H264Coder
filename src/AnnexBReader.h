#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include "Nalu.h"

namespace HM{

class AnnexBReader{
public:
    AnnexBReader(const std::string & _filePath);
    ~AnnexBReader();

    int Open();
    int Close();

    int ReadNalu(std::shared_ptr<Nalu>& nalu);

private:
    bool CheckStartCode(int & startCodeLen, uint8_t * bufPtr, int bufLen);
    int ReadFromFile();

    std::string filePath;
    FILE * f = nullptr;

    bool isEnd = false;
    uint8_t * buffer = nullptr;
    int bufferLen = 0;
};

}