#include "AnnexBReader.h"
#include <cstring>

namespace HM{

AnnexBReader::AnnexBReader(const std::string & _filePath)
{
    filePath = _filePath;
}

AnnexBReader::~AnnexBReader()
{
    Close();
}

int AnnexBReader::Open()
{
    f = fopen(filePath.c_str(), "rb");
    if(f == nullptr){
        return -1;
    }
    return 0;
}

int AnnexBReader::Close()
{
    if(f != nullptr){
        fclose(f);
        f = nullptr;
    }
    if(buffer != nullptr){
        free(buffer);
        buffer = nullptr;
    }
    return 0;
}

int AnnexBReader::ReadFromFile()
{
    int tempBufferLen = 1024;
    uint8_t * buf = (uint8_t *) malloc (tempBufferLen);
    int readedLen = fread(buf, 1, tempBufferLen, f);

    if(readedLen > 0){
        // After adding the newly read buf to the old buffer
        uint8_t * _buffer = (uint8_t *) malloc (bufferLen + readedLen);
        memcpy(_buffer,                 buffer, bufferLen);
        memcpy(_buffer + bufferLen,     buf,    readedLen);
        bufferLen = bufferLen + readedLen;

        if(buffer != nullptr){
            free(buffer);
            buffer = nullptr;
        }
        buffer = _buffer;
    }

    free(buf);
    return readedLen;
}

bool AnnexBReader::CheckStartCode(int& startCodeLen, uint8_t* bufPtr, int bufLen){
    if(bufLen >= 3){
        if(bufPtr[0] == 0 && bufPtr[1] == 0 && bufPtr[2] == 1){
            startCodeLen = 3;
            return true;
        }
        else if(bufferLen >3 && bufPtr[0] == 0 && bufPtr[1] == 0 && 
           bufPtr[2] == 0 && bufPtr[3] == 1){
            startCodeLen = 4;
            return true;
        }
    }
    startCodeLen = 0;
    return false;
}

int AnnexBReader::ReadNalu(std::shared_ptr<Nalu>& nalu){
    while(1){
        if(bufferLen <= 0){
            int readedLen = ReadFromFile();
            if(readedLen <= 0){
                isEnd = true;
            }
        }

        uint8_t* buf = buffer;
        int startCodeLen = 0;
        // Find Start Code
        if(!CheckStartCode(startCodeLen, buf, bufferLen))
            break;

        //nalu.startCodeLen = startCodeLen;
        
        //Find End Code
        int endPos = -1;
        for(int i = 2; i < bufferLen; i++){
            int startCodeLen = 0;
            if(CheckStartCode(startCodeLen, buf + i, bufferLen - i)){
                endPos = i;
                break;
            }
        }

        if(endPos > 0){
            nalu = std::make_shared<Nalu>(buffer, endPos, startCodeLen);

            uint8_t * _buffer = (uint8_t*)malloc(bufferLen - endPos);
            memcpy(_buffer, buffer + endPos, bufferLen - endPos);

            if(buffer != nullptr){
                free(buffer);
                buffer = nullptr;
            }
            buffer = _buffer;
            bufferLen = bufferLen - endPos;

            return 0;
        }
        else{
            if(isEnd == true){
                // Reach the end of the file, fetch all buffers out
                nalu = std::make_shared<Nalu>(buffer, bufferLen, startCodeLen);

                if(buffer != nullptr){
                    free(buffer);
                    buffer = nullptr;
                }
                buffer = nullptr;
                bufferLen = 0;

                return 0;
            }
            int readedLen = ReadFromFile();
            if(readedLen <= 0){
                isEnd = true;
            }
        }
    }
    return -1;
}

}