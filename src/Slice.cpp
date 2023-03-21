#include "Slice.h"
#include "Nalu.h"

namespace HM{

Slice::Slice(VideoParameters* vptr, BitStream* curStream, Nalu* nalu){
    first_mb_in_slice = curStream->bs_read_ue();
    auto tmp = curStream->bs_read_ue();
    if(tmp > 4) tmp -= 5;
    slice_type = (SliceType)tmp;
    pic_parameter_set_id = curStream->bs_read_ue();

    if(vptr->active_sps->separate_colour_plane_flag)
        colour_plane_id = curStream->bs_read_u(2);
    else
        colour_plane_id = PLANE_Y;
    
    //return curStream->getUsedBits();
    frame_num = curStream->bs_read_u(vptr->active_sps->log2_max_frame_num_minus4 + 4);
    if(nalu->GetNalType() == NaluType::NALU_TYPE_IDR){
        idr_flag = 1;        

    }
    if(vptr->active_sps->frame_mbs_only_flag){
        field_pic_flag = 0;
        vptr->structure = FRAME;
    }
    else{
        field_pic_flag = curStream->bs_read_u1();
        if(field_pic_flag){
            bottom_field_flag = (byte)curStream->bs_read_u1();
            vptr->structure = bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
        }
        else{
            vptr->structure = FRAME;
            bottom_field_flag = FALSE;
        }
    }

    if(idr_flag)
        idr_pic_id = curStream->bs_read_ue();
    if(vptr->active_sps->pic_order_cnt_type == 0){
        pic_order_cnt_lsb = curStream->bs_read_u(vptr->active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if(vptr->active_pps->bottom_field_pic_order_in_frame_present_flag == 1 && !field_pic_flag)
            delta_pic_order_cnt_bottom = curStream->bs_read_se();
        else
            delta_pic_order_cnt_bottom = 0;
    }
    if(vptr->active_sps->pic_order_cnt_type == 1){
        if(!vptr->active_sps->delta_pic_order_always_zero_flag){
            delta_pic_order_cnt[ 0 ] = curStream->bs_read_se();
            if(vptr->active_pps->bottom_field_pic_order_in_frame_present_flag == 1&& !field_pic_flag)
                delta_pic_order_cnt[1] = curStream->bs_read_se();
            else    
                delta_pic_order_cnt[1] = 0;
        }
        else{
            delta_pic_order_cnt[0] = 0;
            delta_pic_order_cnt[1] = 0;
        }
    }

    //redundant_pic_cnt is missing here
    if(vptr->active_pps->redundant_pic_cnt_present_flag)
        redundant_pic_cnt = curStream->bs_read_ue();

    if(slice_type == B_SLICE)
        direct_spatial_mv_pred_flag = curStream->bs_read_u1();

    num_ref_idx_active[LIST_0] = vptr->active_pps->num_ref_idx_l0_default_active_minus1 + 1;
    num_ref_idx_active[LIST_1] = vptr->active_pps->num_ref_idx_l1_default_active_minus1 + 1;
    
    if(slice_type == P_SLICE || slice_type == SP_SLICE || slice_type == B_SLICE){
        auto val = curStream->bs_read_u1();
        if(val){
            num_ref_idx_active[LIST_0] = 1 + curStream->bs_read_ue();
            if(slice_type = B_SLICE)
                num_ref_idx_active[LIST_1] = 1 + curStream->bs_read_ue();
        }
    }
    if(slice_type != B_SLICE)
        num_ref_idx_active[LIST_1] = 0;

    
}

}