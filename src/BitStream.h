#pragma once

#include <cstdint>
#include <cstddef>
#include "Buffer.h"

namespace HM{

class BitStream{
public:
  BitStream(Buffer & buffer);
  BitStream(uint8_t * buf, size_t size);
  ~BitStream();

  uint32_t bs_read_u8();
  uint32_t bs_read_u(int n);
  uint32_t bs_peek_u(int n);
  uint32_t bs_read_u1();

  void bs_skip_u(int n);
  void bs_skip_u1();

  uint32_t bs_read_ue();
  int32_t bs_read_se();

  int bs_eof();

  bool bs_byte_aligned();

  bool more_data();

  int getUsedBits() const {return used_bits;}
  //int PrintInfo(int level = 5);
public:
  uint8_t * start;
  uint8_t * p;
  uint8_t * end;
  int  bits_left;
  int  used_bits;
};

}
