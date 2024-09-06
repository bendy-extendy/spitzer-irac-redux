###-Meta-Data-###:Flow=edu.caltech.ipac.sirtf.mopexUI.modules.mosaic.MosaicFlow
###-Meta-Data-###:Version=18.5  Final
###-Meta-Data-###:Date=08/12/2014
###-Meta-Data-###:User-Name=jrrigby1
###-Meta-Data-###:Host-Name=gs66-milk
###-Meta-Data-###:User-Description=Mosaic Processing Flow
###-Meta-Data-###:FlowActive=true


###### Define which modules to run ######

compute_uncertainties_internally = 0
have_uncertainties = 1
run_fiducial_image_frame = 0
run_mosaic_geom = 0
run_medfilter = 0
run_detect_radhit = 0
run_mosaic_interp = 1
run_detect_outlier = 0
run_mosaic_proj = 0
run_mosaic_covg = 1
run_mosaic_dual_outlier = 0
run_level = 0
run_mosaic_outlier = 1
run_mosaic_box_outlier = 1
run_mosaic_rmask = 1
run_mosaic_reinterp = 0
run_fix_coverage = 0
run_mosaic_coadder = 1
run_mosaic_combiner = 1
run_mosaic_medfilter = 0
create_rmask_mosaic = 0
make_array_corr_files = 0
make_array_corr_mosaic = 0


##### Define output of run #####

run_median_mosaic = 1
run_absolute_minimum_mosaic = 0
create_std_mosaic = 1
create_unc_mosaic = 1
create_outlier_mosaic = 0
create_dual_outlier_mosaic = 0

###### Test for array location corr ######

delete_intermediate_files_array_corr = 0

###### Input file lists ######

IMAGE_STACK_FILE_NAME = THE_CLUSTER_DIR/Overlap_corr/correct_geom_all_bcbcd.list
SIGMALIST_FILE_NAME = THE_CLUSTER_DIR/geom_all_cbunc.list
DCE_STATUS_MASK_LIST = 
PMASK_FILE_NAME = /Users/jrrigby1/Applications/mopex/cal/super_masks/chan1_ormask_bcd.fits 
RMASK_LIST = 

FIF_FILE_NAME = THE_CLUSTER_DIR/jrr_mosaic_fif.tbl

###### Output Dir and Files ######

OUTPUT_DIR = THE_CLUSTER_DIR
MEDFILTER_DIR = Medfilter-mosaic
INTERP_DIR = Interp-mosaic
DETECT_DIR = Detect-mosaic
DUAL_OUTLIER_DIR = DualOutlier-mosaic
OUTLIER_DIR = Outlier-mosaic
BOX_OUTLIER_DIR = BoxOutlier-mosaic
RMASK_DIR = Rmask-mosaic
REINTERP_DIR = Reinterp-mosaic
COADDER_DIR = Coadd-mosaic
COMBINER_DIR = Combine-mosaic

###### Mask Bit Parameter ######

DCE_Status_Mask_Fatal_BitPattern = 32520
DCE_Status_Mask_Radhit_Bit = 9
PMask_Fatal_BitPattern = 32767
RMask_Fatal_BitPattern = 15

###### Global Parameters ######

USE_REFINED_POINTING = 0
USE_OUTLIER_FOR_RMASK = 1
USE_BOX_OUTLIER_FOR_RMASK = 1
USE_DUAL_OUTLIER_FOR_RMASK = 0
MOSAIC_PIXEL_SIZE_X = -1.6667E-4
MOSAIC_PIXEL_SIZE_Y = 1.6667E-4

###### Other Parameters ######

overwrite_dmask = 0
keep_coadded_tiles = 0
sigma_weighted_coadd = 1
delete_intermediate_files = 1
SIGMA_DIR = Sigma-mosaic
ncpu_multiprocess = 1
ARRAY_CORR_IMAGE =  /Users/jrrigby1/Applications/mopex/cal/ch1_photcorr_rj_warm.fits
RMASK_MOSAIC_DIR = RmaskMosaic-mosaic
ARRAY_PIXAREA_IMAGE =  /Users/jrrigby1/Applications/mopex/cal/ch1relpixarea.fits
do_multiprocess = 'on'
DMASK_DIR = Dmask-mosaic


###### Modules ######

&SNESTIMATORIN
Gain = 1290.0,
Read_Noise = 90.0,
Confusion_Sigma = 0.0,
&END

&FIDUCIALIMAGEFRAMEIN
Edge_Padding = 100,
Projection_Type = 'TAN',
Coordinate_System = 'J2000',
CROTA2 = 0.0,
&END

&MOSAICGEOM
&END

&MEDFILTER
Window_X = 45,
Window_Y = 45,
N_Outliers_Per_Window = 50,
Use_Sbkg_for_Med = 1,
Sbkg_Filt_Size_for_Med = 1,
&END

&DETECT_RADHIT
Segmentation_Threshold = 3.0,
Detection_Max_Area = 3,
Radhit_Threshold = 6.0,
&END

&MOSAICINTIN
INTERP_METHOD = 2,
FINERES = 0.0,
DRIZ_FAC = 0.6,
GRID_RATIO = 2,
ALPHA = -0.5,
&END

&DETECT
Detection_Max_Area = 100,
Detection_Min_Area = 0,
Detection_Threshold = 4.0,
Threshold_Type = 'simple',
Detect_Min_Peak_Fraction = 0.0,
&END

&MOSAICPROJIN
&END

&MOSAICCOVGIN
TILEMAX_X = 2000,
TILEMAX_Y = 2000,
&END

&MOSAICDUALOUTLIERIN
MAX_OUTL_IMAGE = 2,
MAX_OUTL_FRAC = 0.51,
TILE_XSIZ = 2000,
TILE_YSIZ = 2000,
&END

&LEVEL
Threshold_Ratio = 0.5,
&END

&MOSAICOUTLIERIN
THRESH_OPTION = 1,
BOTTOM_THRESHOLD = 3.0,
TOP_THRESHOLD = 3.0,
MIN_PIX_NUM = 3,
TILE_XSIZ = 256,
TILE_YSIZ = 256,
&END

&MOSAICBOXOUTLIERIN
BOX_X = 3,
BOX_Y = 3,
BOX_MEDIAN_BIAS = 1,
TILE_XSIZ = 256,
TILE_YSIZ = 256,
&END

&MOSAICRMASKIN
RM_THRESH = 0.5,
BOTTOM_THRESHOLD = 3.0,
TOP_THRESHOLD = 3.0,
MIN_COVERAGE = 3,
MAX_COVERAGE = 100,
REFINE_OUTLIER = 0,
REFINE_OUTLIER_THRESH = 1,
BOX_BOTTOM_THRESHOLD = 2.0,
BOX_TOP_THRESHOLD = 3.0,
BOX_MIN_COVERAGE = 1.0,
&END

&MOSAICREINTIN
&END

&FIX_COVERAGE
Min_Single_Coverage = 0.95,
Min_Block_Coverage = 0.83,
&END

&MOSAICCOADDIN
TILEMAX_X = 256,
TILEMAX_Y = 256,
INTEG_TIME_KWD = EXPTIME,
&END

&MOSAICCOMBINER
&END

&MOSAIC_MEDFILTER
Window_X = 45,
Window_Y = 45,
N_Outliers_Per_Window = 500,
Use_Sbkg_for_Med = 0,
Sbkg_Filt_Size_for_Med = 3,
&END

&CREATERMASKMOSAIC
&END

&MAKEARRAYCORRFILES
&END

&MAKEARRAYCORRMOSAIC
&END

#END


