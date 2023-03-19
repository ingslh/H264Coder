#pragma once

#include "ParameterSet.h"
#include <map>


namespace HM{

struct VideoParameters{
  seq_parameter_set_rbsp* active_sps;
  pic_parameter_set_rbsp* active_pps;

  std::map<int, seq_parameter_set_rbsp*> SeqParSet;
  std::map<int, pic_parameter_set_rbsp*> PicParSet;
  
};

}