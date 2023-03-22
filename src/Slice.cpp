#include "Slice.h"
#include "Nalu.h"

namespace HM{

Slice::Slice(VideoParameters* VidParams, BitStream* curStream, Nalu* nalu):
vptr(VidParams), bs(curStream){
    first_mb_in_slice = bs->bs_read_ue();
    auto tmp = bs->bs_read_ue();
    if(tmp > 4) tmp -= 5;
    slice_type = (SliceType)tmp;
    pic_parameter_set_id = bs->bs_read_ue();

    if(vptr->active_sps->separate_colour_plane_flag)
        colour_plane_id = bs->bs_read_u(2);
    else
        colour_plane_id = PLANE_Y;
    
    //return curStream->getUsedBits();
    frame_num = bs->bs_read_u(vptr->active_sps->log2_max_frame_num_minus4 + 4);
    if(nalu->GetNalType() == NaluType::NALU_TYPE_IDR){
        idr_flag = 1;        

    }
    if(vptr->active_sps->frame_mbs_only_flag){
        field_pic_flag = 0;
        vptr->structure = FRAME;
    }
    else{
        field_pic_flag = bs->bs_read_u1();
        if(field_pic_flag){
            bottom_field_flag = (byte)bs->bs_read_u1();
            vptr->structure = bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;
        }
        else{
            vptr->structure = FRAME;
            bottom_field_flag = FALSE;
        }
    }

    if(idr_flag)
        idr_pic_id = bs->bs_read_ue();
    if(vptr->active_sps->pic_order_cnt_type == 0){
        pic_order_cnt_lsb = bs->bs_read_u(vptr->active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if(vptr->active_pps->bottom_field_pic_order_in_frame_present_flag == 1 && !field_pic_flag)
            delta_pic_order_cnt_bottom = bs->bs_read_se();
        else
            delta_pic_order_cnt_bottom = 0;
    }
    if(vptr->active_sps->pic_order_cnt_type == 1){
        if(!vptr->active_sps->delta_pic_order_always_zero_flag){
            delta_pic_order_cnt[ 0 ] = bs->bs_read_se();
            if(vptr->active_pps->bottom_field_pic_order_in_frame_present_flag == 1&& !field_pic_flag)
                delta_pic_order_cnt[1] = bs->bs_read_se();
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
        redundant_pic_cnt = bs->bs_read_ue();

    if(slice_type == B_SLICE)
        direct_spatial_mv_pred_flag = bs->bs_read_u1();

    num_ref_idx_active[LIST_0] = vptr->active_pps->num_ref_idx_l0_default_active_minus1 + 1;
    num_ref_idx_active[LIST_1] = vptr->active_pps->num_ref_idx_l1_default_active_minus1 + 1;
    
    if(slice_type == P_SLICE || slice_type == SP_SLICE || slice_type == B_SLICE){
        auto val = bs->bs_read_u1();
        if(val){
            num_ref_idx_active[LIST_0] = 1 + bs->bs_read_ue();
            if(slice_type = B_SLICE)
                num_ref_idx_active[LIST_1] = 1 + bs->bs_read_ue();
        }
    }
    if(slice_type != B_SLICE)
        num_ref_idx_active[LIST_1] = 0;

#if (MVC_EXTENSION_ENABLE)
  //if (currSlice->svc_extension_flag == 0 || currSlice->svc_extension_flag == 1)
  //  ref_pic_list_mvc_modification(currSlice);
  //else
    ref_pic_list_reordering();
#else
  ref_pic_list_reordering(currSlice);
#endif

    weighted_pred_flag = (unsigned short) ((slice_type == P_SLICE || slice_type == SP_SLICE) 
    ? vptr->active_pps->weighted_pred_flag 
    : (slice_type == B_SLICE && vptr->active_pps->weighted_bipred_idc == 1));
    weighted_bipred_idc = (unsigned short) (slice_type == B_SLICE && vptr->active_pps->weighted_bipred_idc > 0);

    if((vptr->active_pps->weighted_pred_flag && (slice_type == P_SLICE || slice_type == SP_SLICE)) || 
        (vptr->active_pps->weighted_bipred_idc==1 && (slice_type == B_SLICE))){
        pred_weight_table();
    }
    
    if(nalu->GetNalRefIdc()){
        dec_ref_pic_marking();
    }

    if(vptr->active_pps->entropy_coding_mode_flag && slice_type != I_SLICE && slice_type != SI_SLICE){
        cabac_init_idc = bs->bs_read_ue();
    }
    else{
        cabac_init_idc = 0;
    }

    int qp_delta = slice_qp_delta = bs->bs_read_se();
    auto qp = 26 + vptr->active_pps->pic_init_qs_minus26 + qp_delta;

    if(qp < -6* vptr->active_sps->bit_depth_luma_minus8 || qp > 51)
        //error
    
    if(slice_type == SP_SLICE || slice_type == SI_SLICE){
        if(slice_type == SP_SLICE)
            sp_for_switch_flag = bs->bs_read_u1();
        int qs_delta = slice_qs_delta = bs->bs_read_se();
        auto qs = 26 + vptr->active_pps->pic_init_qs_minus26 + qs_delta;
        if(qs < 0 || qs > 51){
            //error
        }
    }

    if(vptr->active_pps->deblocking_filter_control_present_flag){
        disable_deblocking_filter_idc = bs->bs_read_ue();
        if(disable_deblocking_filter_idc != 1){
            slice_alpha_c0_offset_div2 = (short)2 * bs->bs_read_se();
            slice_beta_offset_div2 = (short)2 * bs->bs_read_se();
        }
        else
            slice_alpha_c0_offset_div2 = slice_beta_offset_div2 = 0;
    }
    else
        disable_deblocking_filter_idc = slice_alpha_c0_offset_div2 = slice_beta_offset_div2 = 0;

    /*
    auto isHI_intra_only_profile = [](unsigned int idc, Boolean flag)->int{
        return (((idc == FREXT_Hi10P) || (idc == FREXT_Hi422) ||
                (idc == FREXT_Hi444) && flag)|| idc == FREXT_CAVLC444);
    };
    if(isHI_intra_only_profile(vptr->active_sps->profile_idc, vptr->active_sps->constrained_set3_flag)&& (p_Inp->intra_profile_deblocking == 0))
    */
    if(vptr->active_pps->num_slice_groups_minus1 > 0 && vptr->active_pps->slice_group_map_type >=3 &&
       vptr->active_pps->slice_group_map_type <= 5)
       {
        auto len = (vptr->active_sps->pic_height_in_map_units_minus1+1)*(vptr->active_sps->pic_width_in_mbs_minus1+1)/
          (vptr->active_pps->slice_group_change_rate_minus1+1);
        if (((vptr->active_sps->pic_height_in_map_units_minus1+1)*(vptr->active_sps->pic_width_in_mbs_minus1+1))%
          (vptr->active_pps->slice_group_change_rate_minus1+1))
          len +=1;

        unsigned uiRet = 0;
        while( len != 0 ){
            len >>= 1;
            uiRet++;
        }
        len = uiRet;
        slice_group_change_cycle = bs->bs_read_u(len);
    }
}

void Slice::ref_pic_list_reordering(){
    if(!bs || !vptr) return;

    if(slice_type != I_SLICE && slice_type != SI_SLICE){
        ref_pic_list_reordering_flag[LIST_0] = bs->bs_read_u1();
        int val = ref_pic_list_reordering_flag[LIST_0];

        if(val){
            int i = 0;
            do
            {
                val = modification_of_pic_nums_idc[LIST_0][i] = bs->bs_read_ue();
                if(val == 0 || val == 1)
                    abs_diff_pic_num_minus1[LIST_0][i] = bs->bs_read_ue();
                else
                    if(val == 2)
                        long_term_pic_idx[LIST_0][i] = bs->bs_read_ue();
                i++;
            }while(val != 3);
        }
    }

    if(slice_type == B_SLICE){
        ref_pic_list_reordering_flag[LIST_0] = bs->bs_read_u1();
        int val = ref_pic_list_reordering_flag[LIST_0];

        if(val){
            int i = 0;
            do{
                val = modification_of_pic_nums_idc[LIST_1][i] = bs->bs_read_ue();
                if(val == 0 || val == 1)
                    abs_diff_pic_num_minus1[LIST_1][i] = bs->bs_read_ue();
                else    
                    if(val == 2)
                        long_term_pic_idx[LIST_1][i] = bs->bs_read_ue();
                i++;
            }while(val != 3);
        }
    }

    if(redundant_pic_cnt && (slice_type != I_SLICE))
        redundant_slice_ref_idx = abs_diff_pic_num_minus1[LIST_0][0] + 1;
}

void Slice::pred_weight_table(){
    luma_log2_weight_denom = bs->bs_read_ue();
    short wp_round_luma = luma_log2_weight_denom ? 1<<(luma_log2_weight_denom - 1): 0;

    if(vptr->active_sps->chroma_format_idc){
        chroma_log2_weight_denom = (unsigned short)bs->bs_read_ue();
        short wp_round_chroma = chroma_log2_weight_denom ? 1<<(chroma_log2_weight_denom - 1): 0;
    }

    for(int i = 0; i < num_ref_idx_active[LIST_0]; i++){
        luma_weight_flag_l0 = bs->bs_read_u1();

        if(luma_weight_flag_l0){
            wp_weight[LIST_0][i][0] = bs->bs_read_se();
            wp_offset[LIST_0][i][0] = bs->bs_read_se();
            wp_offset[LIST_0][i][0] = wp_offset[LIST_0][i][0] << vptr->active_sps->bit_depth_luma_minus8 ;
        }
        else{
            wp_weight[LIST_0][i][0] = 1 << luma_log2_weight_denom;
            wp_offset[LIST_0][i][0] = 0; 
        }

        if(vptr->active_sps->chroma_format_idc != 0){
            chroma_weight_flag_l0 = bs->bs_read_u1();

            for(int j = 1; j < 3; j++){
                if(chroma_weight_flag_l0){
                    wp_weight[LIST_0][i][j] = bs->bs_read_se();
                    wp_offset[LIST_0][i][j] = bs->bs_read_se();
                    wp_offset[LIST_0][i][0] = wp_offset[LIST_0][i][0] << vptr->active_sps->bit_depth_chroma_minus8 ;
                }
                else{
                    wp_weight[LIST_0][i][j] = chroma_log2_weight_denom;
                    wp_offset[LIST_0][i][j] = 0;
                }
            }
        }
    }

    if((slice_type == B_SLICE) && vptr->active_pps->weighted_bipred_idc == 1){
        for(int i = 0; i < num_ref_idx_active[LIST_1]; i++){
            luma_weight_flag_l1 = bs->bs_read_u1();
            if(luma_weight_flag_l1){
                wp_weight[LIST_1][i][0] = bs->bs_read_se();
                wp_offset[LIST_1][i][0] = bs->bs_read_se();
                wp_offset[LIST_1][i][0] = wp_offset[LIST_1][i][0] << (vptr->active_sps->bit_depth_luma_minus8);
            }
            else{
                wp_weight[LIST_1][i][0] = 1 << luma_log2_weight_denom;
                wp_offset[LIST_1][i][0] = 0;
            }

            if (vptr->active_sps->chroma_format_idc != 0){
                chroma_weight_flag_l1 = bs->bs_read_u1();
                for (int j = 1; j < 3; j++){
                    if (chroma_weight_flag_l1){
                        wp_weight[LIST_1][i][j] = bs->bs_read_se();
                        wp_offset[LIST_1][i][j] = bs->bs_read_se();
                        wp_offset[LIST_1][i][j] = wp_offset[LIST_1][i][j]<<(vptr->active_sps->bit_depth_chroma_minus8);
                    }
                    else{
                        wp_weight[LIST_1][i][j] = 1<<chroma_log2_weight_denom;
                        wp_offset[LIST_1][i][j] = 0;
                    }
                }
            }

        }
    }
}

void Slice::dec_ref_pic_marking(){


  if (idr_flag ){
    no_output_of_prior_pics_flag = bs->bs_read_u1();
    //p_Vid->no_output_of_prior_pics_flag = pSlice->no_output_of_prior_pics_flag;
    long_term_reference_flag = bs->bs_read_u1();
  }
  else{
    adaptive_ref_pic_buffering_flag = bs->bs_read_u1();
    if(adaptive_ref_pic_buffering_flag){
        int val;
        do{
            DecRefPicMarking tmp_drpm;
             
            val = tmp_drpm.memory_management_control_operation = bs->bs_read_ue();
            if(val==1 || val == 3)
                tmp_drpm.difference_of_pic_nums_minus1 = bs->bs_read_ue();
            if(val == 2)
                tmp_drpm.long_term_pic_num = bs->bs_read_ue();
            if(val == 3 || val == 6)
                tmp_drpm.long_term_frame_idx = bs->bs_read_ue();
            if(val == 4)
                tmp_drpm.max_long_term_frame_idx_plus1 = bs->bs_read_ue();

            dec_ref_pic_marking_buffer.push_back(tmp_drpm);
        }while(val != 0);
    }
  }
}

}