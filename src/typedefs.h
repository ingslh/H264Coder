#pragma once

typedef unsigned char  byte;     //!< byte type definition

//! Boolean Type
#ifdef FALSE
#  define Boolean int
#else
typedef enum {
  FALSE,
  TRUE
} Boolean;
#endif