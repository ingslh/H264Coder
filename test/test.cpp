#include "gtest/gtest.h"
#include "Nalu.h"
#include "AnnexBReader.h"
#include "test_config.h"
#include "VideoParameters.h"

using namespace HM;
std::string test_file = ASSERT_PATH + std::string("/demo_video_176x144_baseline.h264");

TEST(AnnexBReaderTest, OpenFile){
  int ret = reader.Open();
  EXPECT_EQ(0, ret);
}

TEST(AnnexBReaderTest, ReadNaluHeader){
  AnnexBReader reader(test_file);
  reader.Open();
  std::shared_ptr<Nalu> nalu = nullptr;
  auto ret = reader.ReadNalu(nalu);
  EXPECT_EQ(0, ret);
}

TEST(AnnexBReaderTest, ReadSPSPPS){
  AnnexBReader reader(test_file);
  reader.Open();
  std::shared_ptr<Nalu> nalu = nullptr;
  VideoParameters vptr;
  while(1){
    auto ret = reader.ReadNalu(nalu);
    nalu->ProcessNalu(&vptr);
    if(nalu->GetNalType() == NALU_TYPE_PPS) break;
  }
  EXPECT_EQ(TRUE, vptr.active_pps->Valid);
  EXPECT_EQ(TRUE, vptr.active_sps->Valid);
}