#pragma once
#include <cstdint>

namespace HM{

class RBSP {
public:
    RBSP() = default;
    ~RBSP();

    RBSP(const RBSP & rbsp);
    RBSP & operator = (const RBSP & rbsp);
    int RBSPtoSODB();

public:
    uint8_t * buf = nullptr;
    int len = 0;
};

}