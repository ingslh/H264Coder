#pragma once
#include "ParameterSet.h"
#include "VideoParameters.h"
#include "BitStream.h"
#include "Nalu.h"

namespace HM{

enum SliceType{
  P_SLICE = 0,
  B_SLICE = 1,
  I_SLICE = 2,
  SP_SLICE = 3,
  SI_SLICE = 4,
  NUM_SLICE_TYPES = 5
};

enum {
  LIST_0 = 0,
  LIST_1 = 1,
  BI_PRED = 2,
  BI_PRED_L0 = 3,
  BI_PRED_L1 = 4
};

class Macroblock;
class Slice{
public:
    Slice(VideoParameters* VidParams, BitStream* curStream, Nalu* nalu);
    void readOneMacroblock(Macroblock* curMb);

private:
    void ref_pic_list_reordering();
    void pred_weight_table();
    void dec_ref_pic_marking();
    void error_tracking(VideoParameters *p_Vid);

private:
    BitStream* bs = nullptr;
    VideoParameters* vptr = nullptr;

    short weighted_pred_flag;
    short weighted_bipred_idc;

    int idr_flag = 0;

    int first_mb_in_slice;
    int slice_type;
    int pic_parameter_set_id;
    int colour_plane_id;

    int frame_num;
    int field_pic_flag;
    byte bottom_field_flag;

    int idr_pic_id;
    int pic_order_cnt_lsb;
    int delta_pic_order_cnt_bottom;

    int delta_pic_order_cnt[2];
    int redundant_pic_cnt;

    int direct_spatial_mv_pred_flag;
    int num_ref_idx_active[2]; 

    //ref_pic_list_reordering()
    int ref_pic_list_reordering_flag[2];
    int *modification_of_pic_nums_idc[2]; 
    int *abs_diff_pic_num_minus1[2];
    int *long_term_pic_idx[2];
    int redundant_slice_ref_idx;     //!< reference index of redundant slice

    //pred_weight_table
    unsigned short luma_log2_weight_denom;
    unsigned short chroma_log2_weight_denom;
    int luma_weight_flag_l0;
    int wp_weight[2][MAX_REFERENCE_PICTURES][3];
    int wp_offset[6][MAX_REFERENCE_PICTURES][3];
    int chroma_weight_flag_l0;
    int luma_weight_flag_l1;
    int chroma_weight_flag_l1;

    //dec_ref_pic_marking
    int no_output_of_prior_pics_flag;
    int long_term_reference_flag;
    int adaptive_ref_pic_buffering_flag;
    std::vector<DecRefPicMarking> dec_ref_pic_marking_buffer;

    int cabac_init_idc;
    int slice_qp_delta;
    int sp_for_switch_flag;
    int slice_qs_delta;
    short disable_deblocking_filter_idc;
    short slice_alpha_c0_offset_div2;
    short slice_beta_offset_div2;
    int slice_group_change_cycle;
};

}