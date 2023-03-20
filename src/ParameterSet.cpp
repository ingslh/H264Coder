#include "ParameterSet.h"

namespace HM{

static const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

int ParameterSet::ProcessSPS(seq_parameter_set_rbsp* sps, BitStream* bitstream, VideoParameters* vptr){
    if(!sps || !bitstream) return 0;

    sps->profile_idc = bitstream->bs_read_u8();

    if ((sps->profile_idc!=BASELINE       ) && (sps->profile_idc!=MAIN           ) &&
        (sps->profile_idc!=EXTENDED       ) && (sps->profile_idc!=FREXT_HP       ) &&
        (sps->profile_idc!=FREXT_Hi10P    ) && (sps->profile_idc!=FREXT_Hi422    ) &&
        (sps->profile_idc!=FREXT_Hi444    ) && (sps->profile_idc!=FREXT_CAVLC444 )
#if (MVC_EXTENSION_ENABLE)
      &&(sps->profile_idc!=MVC_HIGH       ) && (sps->profile_idc!=STEREO_HIGH    )
#endif
      )
    {
      printf("Invalid Profile IDC (%d) encountered. \n", sps->profile_idc);
      return bitstream->getUsedBits();
    }

  sps->constrained_set0_flag = (Boolean)bitstream->bs_read_u1();
  sps->constrained_set1_flag = (Boolean)bitstream->bs_read_u1();
  sps->constrained_set2_flag = (Boolean)bitstream->bs_read_u1();
  sps->constrained_set3_flag = (Boolean)bitstream->bs_read_u1();
#if (MVC_EXTENSION_ENABLE)
  sps->constrained_set4_flag = (Boolean)bitstream->bs_read_u1();
  sps->constrained_set5_flag = (Boolean)bitstream->bs_read_u1();
#endif
  bitstream->bs_skip_u(2);

  sps->level_idc = bitstream->bs_read_u8();
  sps->seq_parameter_set_id = bitstream->bs_read_ue();

  // Fidelity Range Extensions stuff
  sps->chroma_format_idc = 1;
  sps->bit_depth_luma_minus8   = 0;
  sps->bit_depth_chroma_minus8 = 0;
  sps->lossless_qpprime_flag   = 0;
  sps->separate_colour_plane_flag = 0;

  if((sps->profile_idc==FREXT_HP   ) ||
     (sps->profile_idc==FREXT_Hi10P) ||
     (sps->profile_idc==FREXT_Hi422) ||
     (sps->profile_idc==FREXT_Hi444) ||
     (sps->profile_idc==FREXT_CAVLC444)
#if (MVC_EXTENSION_ENABLE)
     || (sps->profile_idc==MVC_HIGH)
     || (sps->profile_idc==STEREO_HIGH)
#endif
     )
  {
    sps->chroma_format_idc = bitstream->bs_read_ue();
  }

  //use to calculate MaxFrameNum
  sps->log2_max_frame_num_minus4 = bitstream->bs_read_ue();
  sps->pic_order_cnt_type = bitstream->bs_read_ue();

  if(sps->pic_order_cnt_type == 0)
    sps->log2_max_pic_order_cnt_lsb_minus4 = bitstream->bs_read_ue();
  else if(sps->pic_order_cnt_type == 1){
    sps->delta_pic_order_always_zero_flag = (Boolean)bitstream->bs_read_u1();
    sps->offset_for_non_ref_pic = bitstream->bs_read_se();
    sps->offset_for_top_to_bottom_field = bitstream->bs_read_se();
    sps->num_ref_frames_in_pic_order_cnt_cycle = bitstream->bs_read_ue();
    for(unsigned i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
      sps->offset_for_ref_frame[i] = bitstream->bs_read_se();
  }
  //Reference frame max count
  sps->num_ref_frames = bitstream->bs_read_ue();
  sps->gaps_in_frame_num_value_allowed_flag = (Boolean)bitstream->bs_read_u1();
  //Macroblocks nums：width； frame_width = 16 × (pic_width_in_mbs_minus1 + 1);
  sps->pic_width_in_mbs_minus1 = bitstream->bs_read_ue();
  sps->pic_height_in_map_units_minus1 = bitstream->bs_read_ue();

	//The encoding method of the macroblock. if == 1 as frame encoded； if == 0，may be frame encoded or field encoded
	// FrameHeightInMbs = ( 2 − frame_mbs_only_flag ) * PicHeightInMapUnits  
  sps->frame_mbs_only_flag = (Boolean)bitstream->bs_read_u1();
  if(!sps->frame_mbs_only_flag){
    sps->mb_adaptive_frame_field_flag = (Boolean)bitstream->bs_read_u1();
  }
  sps->direct_8x8_inference_flag = (Boolean)bitstream->bs_read_u1();
  sps->frame_cropping_flag = (Boolean)bitstream->bs_read_u1();
  if(sps->frame_cropping_flag){
    sps->frame_crop_left_offset      = bitstream->bs_read_ue();
    sps->frame_crop_right_offset     = bitstream->bs_read_ue();
    sps->frame_crop_top_offset       = bitstream->bs_read_ue();
    sps->frame_crop_bottom_offset    = bitstream->bs_read_ue();
  }
  sps->vui_parameters_present_flag = (Boolean)bitstream->bs_read_u1();
  if(sps->vui_parameters_present_flag){
    vui_parameters(sps, bitstream);
  }
  sps->Valid = TRUE;
  vptr->active_sps = sps;
  vptr->SeqParSet[sps->seq_parameter_set_id] = sps;
  return bitstream->getUsedBits();
}

int ParameterSet::vui_parameters(seq_parameter_set_rbsp* sps, BitStream* bitstream){
  sps->vui_seq_parameters.aspect_ratio_info_present_flag = (Boolean)bitstream->bs_read_u1();
  if(sps->vui_seq_parameters.aspect_ratio_info_present_flag){
    sps->vui_seq_parameters.aspect_ratio_idc = bitstream->bs_read_u8();
    if(255==sps->vui_seq_parameters.aspect_ratio_idc){
      sps->vui_seq_parameters.sar_width = bitstream->bs_read_u(16);
      sps->vui_seq_parameters.sar_height = bitstream->bs_read_u(16);
    }
  }

  sps->vui_seq_parameters.overscan_info_present_flag = (Boolean)bitstream->bs_read_u1();
  if (sps->vui_seq_parameters.overscan_info_present_flag){
    sps->vui_seq_parameters.overscan_appropriate_flag = (Boolean)bitstream->bs_read_u1();
  }

  sps->vui_seq_parameters.video_signal_type_present_flag = (Boolean)bitstream->bs_read_u1();
  if(sps->vui_seq_parameters.video_signal_type_present_flag){
    sps->vui_seq_parameters.video_format = bitstream->bs_read_u(3);
    sps->vui_seq_parameters.video_full_range_flag = (Boolean)bitstream->bs_read_u1();
    sps->vui_seq_parameters.colour_description_present_flag = (Boolean)bitstream->bs_read_u1();
    if(sps->vui_seq_parameters.colour_description_present_flag){
      sps->vui_seq_parameters.colour_primaries = bitstream->bs_read_u(8);
      sps->vui_seq_parameters.transfer_characteristics = bitstream->bs_read_u(8);
      sps->vui_seq_parameters.matrix_coefficients = bitstream->bs_read_u(8);
    }
  }
  sps->vui_seq_parameters.chroma_location_info_present_flag = (Boolean)bitstream->bs_read_u1();
  if(sps->vui_seq_parameters.chroma_location_info_present_flag)
  {
    sps->vui_seq_parameters.chroma_sample_loc_type_top_field     = bitstream->bs_read_ue();
    sps->vui_seq_parameters.chroma_sample_loc_type_bottom_field  = bitstream->bs_read_ue();
  }
  sps->vui_seq_parameters.timing_info_present_flag = (Boolean)bitstream->bs_read_u1();
  if (sps->vui_seq_parameters.timing_info_present_flag)
  {
    sps->vui_seq_parameters.num_units_in_tick = bitstream->bs_read_u(32);
    sps->vui_seq_parameters.time_scale = bitstream->bs_read_u(32);
    sps->vui_seq_parameters.fixed_frame_rate_flag = (Boolean)bitstream->bs_read_u1();
  }
  sps->vui_seq_parameters.nal_hrd_parameters_present_flag = (Boolean)bitstream->bs_read_u1();
  if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag){
    read_hrd_parameters(&(sps->vui_seq_parameters.nal_hrd_parameters), bitstream);
  }
  sps->vui_seq_parameters.vcl_hrd_parameters_present_flag = (Boolean)bitstream->bs_read_u1();
  if (sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
  {
    read_hrd_parameters(&(sps->vui_seq_parameters.vcl_hrd_parameters), bitstream);
  }
  if (sps->vui_seq_parameters.nal_hrd_parameters_present_flag || sps->vui_seq_parameters.vcl_hrd_parameters_present_flag)
  {
    sps->vui_seq_parameters.low_delay_hrd_flag = (Boolean)bitstream->bs_read_u1();
  }
  sps->vui_seq_parameters.pic_struct_present_flag  = (Boolean)bitstream->bs_read_u1();
  sps->vui_seq_parameters.bitstream_restriction_flag =  (Boolean)bitstream->bs_read_u1();
  if (sps->vui_seq_parameters.bitstream_restriction_flag)
  {
    sps->vui_seq_parameters.motion_vectors_over_pic_boundaries_flag =  (Boolean)bitstream->bs_read_u1();
    sps->vui_seq_parameters.max_bytes_per_pic_denom                 =  bitstream->bs_read_ue();
    sps->vui_seq_parameters.max_bits_per_mb_denom                   =  bitstream->bs_read_ue();
    sps->vui_seq_parameters.log2_max_mv_length_horizontal           =  bitstream->bs_read_ue();
    sps->vui_seq_parameters.log2_max_mv_length_vertical             =  bitstream->bs_read_ue();
    sps->vui_seq_parameters.num_reorder_frames                      =  bitstream->bs_read_ue();
    sps->vui_seq_parameters.max_dec_frame_buffering                 =  bitstream->bs_read_ue();
  }
  return 0;
}


int ParameterSet::read_hrd_parameters(hrd_parameters *hrd, BitStream* bitstream){
  hrd->cpb_cnt_minus1 = bitstream->bs_read_ue();
  hrd->bit_rate_scale = bitstream->bs_read_u(4);
  hrd->cpb_size_scale = bitstream->bs_read_u(4);

  for(unsigned int SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1; SchedSelIdx++ )
  {
    hrd->bit_rate_value_minus1[ SchedSelIdx ] = bitstream->bs_read_ue();
    hrd->cpb_size_value_minus1[ SchedSelIdx ]  = bitstream->bs_read_ue();
    hrd->cbr_flag[ SchedSelIdx ] = bitstream->bs_read_ue();
  }

  hrd->initial_cpb_removal_delay_length_minus1 = bitstream->bs_read_u(5);
  hrd->cpb_removal_delay_length_minus1 = bitstream->bs_read_u(5);
  hrd->dpb_output_delay_length_minus1 = bitstream->bs_read_u(5);
  hrd->time_offset_length = bitstream->bs_read_u(5);

  return 0;
}

int ParameterSet::ProcessPPS(pic_parameter_set_rbsp* pps, BitStream* bitstream, VideoParameters* vptr){
  if(!pps || !bitstream) return 0;

  pps->pic_parameter_set_id = bitstream->bs_read_ue();
  pps->seq_parameter_set_id = bitstream->bs_read_ue();
  pps->entropy_coding_mode_flag = (Boolean)bitstream->bs_read_u1();

  pps->bottom_field_pic_order_in_frame_present_flag = (Boolean)bitstream->bs_read_u1();
  pps->num_slice_groups_minus1 = bitstream->bs_read_ue();

  // FMO stuff begins here
  if (pps->num_slice_groups_minus1 > 0)
  {
    pps->slice_group_map_type = bitstream->bs_read_ue();
    if (pps->slice_group_map_type == 0)
    {
      for (int i=0; i<=pps->num_slice_groups_minus1; i++)
        pps->run_length_minus1 [i] = bitstream->bs_read_ue();
    }
    else if (pps->slice_group_map_type == 2)
    {
      for (int i=0; i<pps->num_slice_groups_minus1; i++)
      {
        //! JVT-F078: avoid reference of SPS by using ue(v) instead of u(v)
        pps->top_left [i]                          = bitstream->bs_read_ue();
        pps->bottom_right [i]                      = bitstream->bs_read_ue();
      }
    }
    else if (pps->slice_group_map_type == 3 ||
             pps->slice_group_map_type == 4 ||
             pps->slice_group_map_type == 5)
    {
      pps->slice_group_change_direction_flag     = (Boolean)bitstream->bs_read_u1();
      pps->slice_group_change_rate_minus1        = (Boolean)bitstream->bs_read_u1();
    }
    else if (pps->slice_group_map_type == 6)
    {
      int NumberBitsPerSliceGroupId;
      if (pps->num_slice_groups_minus1+1 >4)
        NumberBitsPerSliceGroupId = 3;
      else if (pps->num_slice_groups_minus1+1 > 2)
        NumberBitsPerSliceGroupId = 2;
      else
        NumberBitsPerSliceGroupId = 1;
      pps->pic_size_in_map_units_minus1      = bitstream->bs_read_ue();
      if ((pps->slice_group_id = (byte*)calloc (pps->pic_size_in_map_units_minus1+1, 1)) == NULL)
        return 0;
      for (int i=0; i<=pps->pic_size_in_map_units_minus1; i++)
        pps->slice_group_id[i] = (byte) bitstream->bs_read_u(NumberBitsPerSliceGroupId);
    }
  }

  // End of FMO stuff
	//define if num_ref_idx_active_override_flag ==0，the default value of num_ref_idx_l0/l1_active_minus1
  pps->num_ref_idx_l0_default_active_minus1  = bitstream->bs_read_ue();
  pps->num_ref_idx_l1_default_active_minus1  = bitstream->bs_read_ue();

	//Whether to enable weighted prediction in the P/SP slice
	pps->weighted_pred_flag                    = (Boolean)bitstream->bs_read_u1();
  //Indicates the weighted prediction method in B Slice,
	// and the value range is [0,2].
	// 0 means default weighted forecast,
	// 1 means explicit weighted forecast,
	// 2 means implicit weighted forecast.
	pps->weighted_bipred_idc                   = bitstream->bs_read_u(2);
  //Indicates the initial quantization parameter
	pps->pic_init_qp_minus26                   = bitstream->bs_read_se();
  pps->pic_init_qs_minus26                   = bitstream->bs_read_se();
	//The quantization parameter used to calculate the chroma component, the value range is [-12,12].
  pps->chroma_qp_index_offset                = bitstream->bs_read_se();

	//An identification bit, used to indicate whether there is information for controlling the deblocking filter in the Slice header.
	// When the flag bit is 1, the slice header contains information corresponding to deblocking filtering;
	// when the flag bit is 0, there is no corresponding information in the slice header.
  pps->deblocking_filter_control_present_flag = (Boolean)bitstream->bs_read_u1();
  pps->constrained_intra_pred_flag           = (Boolean)bitstream->bs_read_u1();
  pps->redundant_pic_cnt_present_flag        = (Boolean)bitstream->bs_read_u1();

  if(bitstream->more_data()) // more_data_in_rbsp()
  {
    //Fidelity Range Extensions Stuff
    pps->transform_8x8_mode_flag           =  (Boolean)bitstream->bs_read_u1();
    pps->pic_scaling_matrix_present_flag   =  (Boolean)bitstream->bs_read_u1();

    if(pps->pic_scaling_matrix_present_flag)
    {
      int chroma_format_idc = vptr->SeqParSet[pps->seq_parameter_set_id]->chroma_format_idc;
      int n_ScalingList = 6 + ((chroma_format_idc != YUV444) ? 2 : 6) * pps->transform_8x8_mode_flag;
      for(int i=0; i<n_ScalingList; i++)
      {
        pps->pic_scaling_list_present_flag[i]= (Boolean)bitstream->bs_read_u1();

        if(pps->pic_scaling_list_present_flag[i])
        {
          if(i<6)
            Scaling_List(pps->ScalingList4x4[i], 16, &pps->UseDefaultScalingMatrix4x4Flag[i], bitstream);
          else
            Scaling_List(pps->ScalingList8x8[i-6], 64, &pps->UseDefaultScalingMatrix8x8Flag[i-6], bitstream);
        }
      }
    }
    pps->second_chroma_qp_index_offset = bitstream->bs_read_se();
  }
  else
  {
    pps->second_chroma_qp_index_offset = pps->chroma_qp_index_offset;
  }

  pps->Valid = TRUE;
  vptr->active_pps = pps;
  vptr->PicParSet[pps->pic_parameter_set_id] = pps;
  return bitstream->getUsedBits();
}

// syntax for scaling list matrix values
void ParameterSet::Scaling_List(int *scalingList, int sizeOfScalingList, Boolean *UseDefaultScalingMatrix, BitStream *s)
{
  int j, scanj;
  int delta_scale, lastScale, nextScale;

  lastScale      = 8;
  nextScale      = 8;

  for(j=0; j<sizeOfScalingList; j++)
  {
    scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

    if(nextScale!=0)
    {
      delta_scale = s->bs_read_se();
      nextScale = (lastScale + delta_scale + 256) % 256;
      *UseDefaultScalingMatrix = (Boolean) (scanj==0 && nextScale==0);
    }

    scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
    lastScale = scalingList[scanj];
  }
}


}