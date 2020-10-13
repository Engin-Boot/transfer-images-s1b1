//
// Copyright (C) 2019 IBM Watson Health Imaging.
//
// All rights reserved.
//
// This software is furnished under license and may be used and copied only in
// accordance with the terms of such license and with the inclusion of the
// above copyright notice. This software or any other copies thereof may not be
// provided or otherwise made available to any other person. No title to and
// ownership of the software is hereby transferred.
//

// This file generated on 14/08/19 11:28:20

/** 
 * The ServiceNamesObj structure defines constant strings representing names for the
 * DICOM services as defined in the mergecom.srv configuration file. It eliminates
 * the need to hard code these names and makes possible compile time checking.
 * As the standard evolves the file will be updated in order to remain current.
 * In particular, the following changes can be expected:
 * 1. New service names will be added as new services are defined in new
 *    Supplements and Change Proposals.
 */
#ifndef MTI_SERVICES_H
#define MTI_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(_WIN32) || defined(_WIN64) || defined(INTEL_BCC)) && defined(_MC3_DYNAMIC_LINK)
#  if !defined(SERV_ITEM_IMPORT_EXPORT)
#    if defined(SERVICES_ITEMS_INTERNAL)
#      define SERV_ITEM_IMPORT_EXPORT __declspec(dllexport)
#    else
#      define SERV_ITEM_IMPORT_EXPORT __declspec(dllimport)
#    endif
#  endif
#else
#  define SERV_ITEM_IMPORT_EXPORT
#endif

typedef struct
{
    const char* const ACQUISITION_CONTEXT_SR;
    const char* const ADVANCED_BLENDING_PRESENTATION_STATE;
    const char* const ARTERIAL_PULSE_WAVEFORM;
    const char* const AUTOREFRACTION_MEASUREMENTS;
    const char* const BASIC_ANNOTATION_BOX;
    const char* const BASIC_COLOR_IMAGE_BOX;
    const char* const BASIC_FILM_BOX;
    const char* const BASIC_FILM_SESSION;
    const char* const BASIC_GRAYSCALE_IMAGE_BOX;
    const char* const BASIC_PRINT_IMAGE_OVERLAY_BOX;
    const char* const BASIC_STRUCTURED_DISPLAY;
    const char* const BREAST_IMAGING_RPI_QUERY;
    const char* const BREAST_PROJ_PRESENT;
    const char* const BREAST_PROJ_PROCESS;
    const char* const BREAST_TOMO_IMAGE_STORAGE;
    const char* const C_ARM_PHOTON_ELECTRON_RADIATION;
    const char* const CARDIAC_RPI_QUERY;
    const char* const CHEST_CAD_SR;
    const char* const COLON_CAD_SR;
    const char* const COLOR_PALETTE_FIND;
    const char* const COLOR_PALETTE_GET;
    const char* const COLOR_PALETTE_MOVE;
    const char* const COLOR_PALETTE_STORAGE;
    const char* const COMPOSITE_INST_RET_NO_BULK_GET;
    const char* const COMPOSITE_INSTANCE_ROOT_RET_GET;
    const char* const COMPOSITE_INSTANCE_ROOT_RET_MOVE;
    const char* const COMPOSITING_PLANAR_MPR_VOLUMETRIC_PS;
    const char* const COMPREHENSIVE_3D_SR;
    const char* const CONTENT_ASSESSMENT_RESULTS;
    const char* const CORNEAL_TOPOGRAPHY_MAP;
    const char* const CT_DEFINED_PROCEDURE_PROTOCOL;
    const char* const CT_PERFORMED_PROCEDURE_PROTOCOL;
    const char* const DEFINED_PROCEDURE_PROTOCOL_FIND;
    const char* const DEFINED_PROCEDURE_PROTOCOL_GET;
    const char* const DEFINED_PROCEDURE_PROTOCOL_MOVE;
    const char* const DEFORMABLE_SPATIAL_REGISTRATION;
    const char* const DETACHED_INTERP_MANAGEMENT;
    const char* const DETACHED_PATIENT_MANAGEMENT;
    const char* const DETACHED_RESULTS_MANAGEMENT;
    const char* const DETACHED_STUDY_MANAGEMENT;
    const char* const DETACHED_VISIT_MANAGEMENT;
    const char* const DICOMDIR;
    const char* const DISPLAY_SYSTEM;
    const char* const ENCAPSULATED_CDA;
    const char* const ENCAPSULATED_STL;
    const char* const ENHANCED_CT_IMAGE;
    const char* const ENHANCED_MR_COLOR_IMAGE;
    const char* const ENHANCED_MR_IMAGE;
    const char* const ENHANCED_PET_IMAGE;
    const char* const ENHANCED_US_VOLUME;
    const char* const ENHANCED_XA_IMAGE;
    const char* const ENHANCED_XRF_IMAGE;
    const char* const EXTENSIBLE_SR;
    const char* const G_P_PERFORMED_PROCEDURE_STEP;
    const char* const G_P_SCHEDULED_PROCEDURE_STEP;
    const char* const G_P_WORKLIST;
    const char* const GENERAL_AUDIO_WAVEFORM;
    const char* const GENERAL_RPI_QUERY;
    const char* const GENERIC_IMPLANT_TEMPLATE;
    const char* const GENERIC_IMPLANT_TEMPLATE_FIND;
    const char* const GENERIC_IMPLANT_TEMPLATE_GET;
    const char* const GENERIC_IMPLANT_TEMPLATE_MOVE;
    const char* const GRAYSCALE_PLANAR_MPR_VOLUMETRIC_PS;
    const char* const HANGING_PROTOCOL;
    const char* const HANGING_PROTOCOL_FIND;
    const char* const HANGING_PROTOCOL_GET;
    const char* const HANGING_PROTOCOL_MOVE;
    const char* const IMAGE_OVERLAY_BOX_RETIRED;
    const char* const IMPLANT_ASSEMBLY_TEMPLATE;
    const char* const IMPLANT_ASSEMBLY_TEMPLATE_FIND;
    const char* const IMPLANT_ASSEMBLY_TEMPLATE_GET;
    const char* const IMPLANT_ASSEMBLY_TEMPLATE_MOVE;
    const char* const IMPLANT_TEMPLATE_GROUP;
    const char* const IMPLANT_TEMPLATE_GROUP_FIND;
    const char* const IMPLANT_TEMPLATE_GROUP_GET;
    const char* const IMPLANT_TEMPLATE_GROUP_MOVE;
    const char* const IMPLANTATION_PLAN_SR_DOCUMENT;
    const char* const INSTANCE_AVAIL_NOTIFICATION;
    const char* const INTRAOCULAR_LENS_CALCULATIONS;
    const char* const KERATOMETRY_MEASUREMENTS;
    const char* const KEY_OBJECT_SELECTION_DOC;
    const char* const LEGACY_CONVERTED_ENHANCED_CT_IMAGE;
    const char* const LEGACY_CONVERTED_ENHANCED_MR_IMAGE;
    const char* const LEGACY_CONVERTED_ENHANCED_PET_IMAGE;
    const char* const LENSOMETRY_MEASUREMENTS;
    const char* const MACULAR_GRID_THIICKNESS_VOLUME;
    const char* const MAMMOGRAPHY_CAD_SR;
    const char* const MEDIA_CREATION_MANAGEMENT;
    const char* const MODALITY_WORKLIST_FIND;
    const char* const MR_SPECTROSCOPY;
    const char* const MULTIPLE_VOLUME_RENDERING_VOLUMETRIC_PRESENTATION_STATE;
    const char* const OPHT_VIS_FIELD_STATIC_PERIM_MEAS;
    const char* const OPHTHALMIC_AXIAL_MEASUREMENTS;
    const char* const OPHTHALMIC_OCT_BSCAN_VOLUME_ANALYSIS;
    const char* const OPHTHALMIC_OCT_EN_FACE_IMAGE;
    const char* const OPHTHALMIC_TOMOGRAPHY_IMAGE;
    const char* const OPM_THICKNESS_MAP;
    const char* const PARAMETRIC_MAP;
    const char* const PATIENT_RADIATION_DOSE_SR;
    const char* const PATIENT_ROOT_QR_FIND;
    const char* const PATIENT_ROOT_QR_GET;
    const char* const PATIENT_ROOT_QR_MOVE;
    const char* const PATIENT_STUDY_ONLY_QR_FIND;
    const char* const PATIENT_STUDY_ONLY_QR_GET;
    const char* const PATIENT_STUDY_ONLY_QR_MOVE;
    const char* const PERFORMED_IMAGING_AGENT_ADMINISTRATION_SR;
    const char* const PERFORMED_PROCEDURE_STEP;
    const char* const PERFORMED_PROCEDURE_STEP_NOTIFY;
    const char* const PERFORMED_PROCEDURE_STEP_RETRIEVE;
    const char* const PLANNED_IMAGING_AGENT_ADMINISTRATION_SR;
    const char* const PRESENTATION_LUT;
    const char* const PRINT_JOB;
    const char* const PRINT_QUEUE_MANAGEMENT;
    const char* const PRINTER;
    const char* const PRINTER_CONFIGURATION;
    const char* const PROCEDURAL_EVENT_LOGGING;
    const char* const PROCEDURE_LOG;
    const char* const PRODUCT_CHARACTERISTICS_QUERY;
    const char* const PROTOCOL_APPROVAL;
    const char* const PROTOCOL_APPROVAL_FIND;
    const char* const PROTOCOL_APPROVAL_GET;
    const char* const PROTOCOL_APPROVAL_MOVE;
    const char* const PULL_PRINT_REQUEST;
    const char* const RADIOPHARMACEUTICAL_RADIATION_DOSE_SR;
    const char* const RAW_DATA;
    const char* const REAL_WORLD_VALUE_MAPPING;
    const char* const REFERENCED_IMAGE_BOX;
    const char* const RESPIRATORY_WAVEFORM;
    const char* const RT_BEAMS_DELIVERY_INSTR_RET;
    const char* const RT_BEAMS_DELIVERY_INSTRUCTION;
    const char* const RT_BRACHY_APP_SETUP_DELIVERY_INSTR;
    const char* const RT_CONV_MACHINE_VERIF_RET;
    const char* const RT_CONVENTIONAL_MACHINE_VERIF;
    const char* const RT_ION_MACHINE_VERIF;
    const char* const RT_ION_MACHINE_VERIF_RET;
    const char* const RT_PHYSICIAN_INTENT;
    const char* const RT_RADIATION_SET;
    const char* const RT_SEGMENT_ANNOTATION;
    const char* const SC_MULTIFRAME_GRAYSCALE_BYTE;
    const char* const SC_MULTIFRAME_GRAYSCALE_WORD;
    const char* const SC_MULTIFRAME_SINGLE_BIT;
    const char* const SC_MULTIFRAME_TRUE_COLOR;
    const char* const SEGMENTATION;
    const char* const SEGMENTED_VOLUME_RENDERING_VOLUMETRIC_PRESENTATION_STATE;
    const char* const SIMPLIFIED_ADULT_ECHO_SR;
    const char* const SPATIAL_FIDUCIALS;
    const char* const SPATIAL_REGISTRATION;
    const char* const SPECTACLE_PRESCRIPTION_REPORT;
    const char* const STANDARD_BASIC_TEXT_SR;
    const char* const STANDARD_BLENDING_SOFTCOPY_PS;
    const char* const STANDARD_COLOR_SOFTCOPY_PS;
    const char* const STANDARD_COMPREHENSIVE_SR;
    const char* const STANDARD_CR;
    const char* const STANDARD_CT;
    const char* const STANDARD_CURVE;
    const char* const STANDARD_DX_PRESENT;
    const char* const STANDARD_DX_PROCESS;
    const char* const STANDARD_ECHO;
    const char* const STANDARD_ENCAPSULATED_PDF;
    const char* const STANDARD_ENHANCED_SR;
    const char* const STANDARD_GRAYSCALE_SOFTCOPY_PS;
    const char* const STANDARD_HARDCOPY_COLOR;
    const char* const STANDARD_HARDCOPY_GRAYSCALE;
    const char* const STANDARD_IO_PRESENT;
    const char* const STANDARD_IO_PROCESS;
    const char* const STANDARD_IVOCT_PRESENT;
    const char* const STANDARD_IVOCT_PROCESS;
    const char* const STANDARD_MG_PRESENT;
    const char* const STANDARD_MG_PROCESS;
    const char* const STANDARD_MODALITY_LUT;
    const char* const STANDARD_MR;
    const char* const STANDARD_NM;
    const char* const STANDARD_NM_RETIRED;
    const char* const STANDARD_OPHTHALMIC_16_BIT;
    const char* const STANDARD_OPHTHALMIC_8_BIT;
    const char* const STANDARD_OVERLAY;
    const char* const STANDARD_PET;
    const char* const STANDARD_PET_CURVE;
    const char* const STANDARD_PRINT_STORAGE;
    const char* const STANDARD_PSEUDOCOLOR_SOFTCOPY_PS;
    const char* const STANDARD_RT_BEAMS_TREAT;
    const char* const STANDARD_RT_BRACHY_TREAT;
    const char* const STANDARD_RT_DOSE;
    const char* const STANDARD_RT_IMAGE;
    const char* const STANDARD_RT_ION_BEAMS_TREAT;
    const char* const STANDARD_RT_ION_PLAN;
    const char* const STANDARD_RT_PLAN;
    const char* const STANDARD_RT_STRUCTURE_SET;
    const char* const STANDARD_RT_TREAT_SUM;
    const char* const STANDARD_SEC_CAPTURE;
    const char* const STANDARD_US;
    const char* const STANDARD_US_MF;
    const char* const STANDARD_US_MF_RETIRED;
    const char* const STANDARD_US_RETIRED;
    const char* const STANDARD_VIDEO_ENDOSCOPIC;
    const char* const STANDARD_VIDEO_MICROSCOPIC;
    const char* const STANDARD_VIDEO_PHOTOGRAPHIC;
    const char* const STANDARD_VL_ENDOSCOPIC;
    const char* const STANDARD_VL_MICROSCOPIC;
    const char* const STANDARD_VL_PHOTOGRAPHIC;
    const char* const STANDARD_VL_SLIDE_MICROSCOPIC;
    const char* const STANDARD_VOI_LUT;
    const char* const STANDARD_WAVEFORM_12_LEAD_ECG;
    const char* const STANDARD_WAVEFORM_AMBULATORY_ECG;
    const char* const STANDARD_WAVEFORM_BASIC_VOICE_AU;
    const char* const STANDARD_WAVEFORM_CARDIAC_EP;
    const char* const STANDARD_WAVEFORM_GENERAL_ECG;
    const char* const STANDARD_WAVEFORM_HEMODYNAMIC;
    const char* const STANDARD_XRAY_ANGIO;
    const char* const STANDARD_XRAY_ANGIO_BIPLANE;
    const char* const STANDARD_XRAY_RF;
    const char* const STEREOMETRIC_RELATIONSHIP;
    const char* const STORAGE_COMMITMENT_PULL;
    const char* const STORAGE_COMMITMENT_PUSH;
    const char* const STUDY_COMPONENT_MANAGEMENT;
    const char* const STUDY_CONTENT_NOTIFICATION;
    const char* const STUDY_ROOT_QR_FIND;
    const char* const STUDY_ROOT_QR_GET;
    const char* const STUDY_ROOT_QR_MOVE;
    const char* const SUBJ_REFRACTION_MEASUREMENTS;
    const char* const SUBSTANCE_ADMIN_LOGGING;
    const char* const SUBSTANCE_APPROVAL_QUERY;
    const char* const SURFACE_SCAN_MESH;
    const char* const SURFACE_SCAN_POINT_CLOUD;
    const char* const SURFACE_SEGMENTATION;
    const char* const TRACTOGRAPHY_RESULTS;
    const char* const UPS_EVENT_SOP;
    const char* const UPS_EVENT_SOP_TRIAL_RETIRED;
    const char* const UPS_PULL_SOP;
    const char* const UPS_PULL_SOP_TRIAL_RETIRED;
    const char* const UPS_PUSH_SOP;
    const char* const UPS_PUSH_SOP_TRIAL_RETIRED;
    const char* const UPS_WATCH_SOP;
    const char* const UPS_WATCH_SOP_TRIAL_RETIRED;
    const char* const VISUAL_ACUITY_MEASUREMENTS;
    const char* const VL_WHOLE_SLIDE_MICROSCOPY_IMAGE;
    const char* const VOI_LUT_BOX;
    const char* const VOLUME_RENDERING_VOLUMETRIC_PRESENTATION_STATE;
    const char* const WIDE_FIELD_OPHTHALMIC_PHOTO_3D_COORDINATES;
    const char* const WIDE_FIELD_OPHTHALMIC_PHOTO_STEREOGRAPHIC_PROJ;
    const char* const XA_XRF_GRAYSCALE_SOFTCOPY_PS;
    const char* const XRAY_3D_ANGIO_IMAGE;
    const char* const XRAY_3D_CRANIO_IMAGE;
    const char* const XRAY_RADIATION_DOSE_SR;
} ServiceNamesObj;

SERV_ITEM_IMPORT_EXPORT extern ServiceNamesObj Services;

#ifdef __cplusplus
}
#endif

#endif /* MTI_SERVICES_H */

