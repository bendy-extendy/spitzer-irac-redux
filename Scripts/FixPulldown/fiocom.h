/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*  The following definitions and structures are used by the mrc2fts program
 *  and the m2fitsio routines.
 */
#define TRUE 	  1
#define FALSE	  0
#define FITS_REC_LEN 2880
#define MAXLONG 0xEFFFFFFF
#define XPIX 512
#define YPIX 512

struct fhead_t             /*  This structure stores a FITS header block, and */
{                          /*  points to the next one. If it is the last block*/
   struct fhead_t * next;  /*  the next pointer will be NULL. */
   char  buf[FITS_REC_LEN];
};

struct BUFFER              /*  This structure contains the header information */
{                          /*  read in and write to the FITS file.            */
   short status;           /*  Current status of file                         */
   short naxis;            /*  Number of axes in the array, usually 2         */
   short naxis1;           /*  Length of horizontal axis in pixels            */
   short naxis2;           /*  Length of vertical axis in pixels              */
   double bscale;          /*  The value of the data is given by the following*/
   double bzero;           /*    value = filedata * bscale + bzero            */
   short bitpix;           /*  Number of bits/pixel.                          */
   short size;             /*  Number of bytes/pixel  (bitpix/4)              */
   long N;                 /*  Total number of pixels                         */
   long max;               /*  Maximum data value (not currently used)        */
   long min;               /*  Minimum data value (not currently used)        */
   short Nheader;          /*  Number of FITS header lines                    */
   short Ncurbuf;          /*  Number of header lines in current FITS block   */
   char fname[40];         /*  FITS file name                                 */
   struct fhead_t * fhead; /*  Holds FITS header in proper FITS format        */
   long * fdata;           /*  Not currently used                             */
};

union intbufs              /*  Structure used when reading in FITS data blocks*/
{
   long longarr[720];
   short shortarr[1440];
   char chararr[2880];
   float farray[720];
   double darray[360];
};
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

