###-Meta-Data-###:Flow=edu.caltech.ipac.sirtf.mopexUI.modules.overlap.OverlapFlow
###-Meta-Data-###:Version=18.5  Final
###-Meta-Data-###:Date=08/12/2014
###-Meta-Data-###:User-Name=jrrigby1
###-Meta-Data-###:Host-Name=gs66-milk
###-Meta-Data-###:User-Description=Overlap Processing Flow
###-Meta-Data-###:FlowActive=true


###### Define which modules to run ######

compute_uncertainties_internally = 0
have_uncertainties = 1
run_fiducial_image_frame = 0
run_medfilter = 1
run_detect = 1
run_mosaic_interp = 1
compute_overlap_correction = 1
mosaic_corrected_images = 0


###### Input file lists ######

IMAGE_STACK_FILE_NAME = THE_CLUSTER_DIR/geom_all_bcbcd.list
SIGMALIST_FILE_NAME = THE_CLUSTER_DIR/geom_all_cbunc.list
DCE_STATUS_MASK_LIST = 
PMASK_FILE_NAME = /Users/jrrigby1/Applications/mopex/cal/super_masks/chan1_ormask_bcd.fits 
FIF_FILE_NAME = THE_CLUSTER_DIR/jrr_mosaic_fif.tbl

###### Output Dir and Files ######

OUTPUT_DIR = THE_CLUSTER_DIR
MEDFILTER_DIR = Medfilter-overlap
INTERP_DIR = Interp-overlap
DETECT_DIR = Detect-overlap
COADDER_DIR = Coadd-overlap

###### Mask Bit Parameter ######

DCE_Status_Mask_Fatal_BitPattern = 32520
PMask_Fatal_BitPattern = 32767

###### Global Parameters ######

USE_REFINED_POINTING = 0
MOSAIC_PIXEL_SIZE_X = -1.6667E-4
MOSAIC_PIXEL_SIZE_Y = 1.6667E-4

###### Multi-processing Parameters ######
do_multiprocess = 'on'
ncpu_multiprocess = 1

###### Other Parameters ######

delete_intermediate_files = 1
OVERLAP_CORR_FILE_NAME = offsets.tbl
RMASK_LIST = 
apply_zodiacal_subtraction = 0
SIGMA_DIR = Sigma-overlap
MOSAIC_IMG_FILE_NAME = mosaic.fits
RMask_Fatal_BitPattern = 15
mask_bright = 1
apply_overlap_correction = 1
OVERLAP_CORR_DIR = Overlap_corr
save_zodiacal_subtracted_images = 0
OverlapCorrectionTable = offsets.tbl
COMPUTE_OVERLAP_NAME = offsets.tbl
ZODSUB_DIR = Zod_Sub


###### Modules ######

&SNESTIMATORIN
Gain = 1290.0,
Read_Noise = 90.0,
Confusion_Sigma = 0.0,
&END

&FIDUCIALIMAGEFRAMEIN
Edge_Padding = 10,
Projection_Type = 'TAN',
Coordinate_System = 'J2000',
CROTA2 = 0.0,
&END

&MEDFILTER
Window_X = 25,
Window_Y = 25,
N_Outliers_Per_Window = 50,
Use_Sbkg_for_Med = 1,
Sbkg_Filt_Size_for_Med = 3,
&END

&DETECT
Detection_Max_Area = 90000,
Detection_Min_Area = 0,
Detection_Threshold = 3.0,
Threshold_Type = 'simple',
Detect_Min_Peak_Fraction = 0.0,
&END

&MOSAICINTIN
INTERP_METHOD = 2,
FINERES = 0.0,
DRIZ_FAC = 1.0,
GRID_RATIO = 4,
ALPHA = -0.5,
&END

&COMPUTEOVRLAPCORRIN
TOP_THRESHOLD = 3.0,
BOTTOM_THRESHOLD = 3.0,
MIN_IMG_NUM = 4,
USE_SPARSE_FOR_MANY_FILES = 0,
SKIP_BAD_IMAGE_CHECK = 0,
&END

&MOSAICCORRECTEDIMAGES
&END

&MOSAICCOADDIN
TILEMAX_X = 2000,
TILEMAX_Y = 2000,
&END

#END


