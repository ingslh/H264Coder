#pragma once

#include "ParameterSet.h"
#include <map>


namespace HM{
struct seq_parameter_set_rbsp;
struct pic_parameter_set_rbsp;
struct VideoParameters{
  seq_parameter_set_rbsp* active_sps = nullptr;
  pic_parameter_set_rbsp* active_pps = nullptr;

  std::map<int, seq_parameter_set_rbsp*> SeqParSet;
  std::map<int, pic_parameter_set_rbsp*> PicParSet;

  int structure;
  int Is_primary_correct;          //!< if primary frame is correct, 0: incorrect
  int Is_redundant_correct;        //!< if redundant frame is correct, 0:incorrect

  ~VideoParameters(){
  }
};

}