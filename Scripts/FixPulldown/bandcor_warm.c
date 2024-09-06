/*************************************************************************/
/* bandcor.c - correct row-wise banding in IRAC images by forcing        */
/*                all rows in box to have the same median                */
/* Original version by JLHora                                            */
/* This modified version by MLNAshby                                     */
/* 2009 Nov. 20 -- tested on SEDS and PID60194 data                      */
/* 2009 Dec. 4  -- Added user-settable thresholding capability           */
/* 2009 Dec. 11 -- Added better choice of reference pixel a[4]           */
/* 2009 Dec. 22 -- Tried forcing a[4] to be held constant                */
/* 2010 Jan. 6  -- Changed three errant print statements                 */
/* 2010 Jan. 6  -- Changed fourth errant printf; trapped NaN in 1st row  */
/* 2010 Jan. 27 -- Removed Chan as an input parameter                    */
/*                                                                       */
/* Bug: FORCE appears not to be getting set by calling routine           */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/* compile:  cc bandcor_warm.c fitsio.c  swapbytes.c -lm -o bandcor_warm */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "fiocom.h"

#define TRUE 1
#define FALSE 0

#define MA 4
#define TOL 0.00001
/* Don't perform any fitting unless we have at least MIN_FIT_PIXELS to fit */
#define MIN_FIT_PIXELS 10
/* This is an absolute value to determine whether to fit to a column */
/* I tried 8.0; this was too small for 100 s frames                  */
/* Exclude pixels exceeding N_SIGMA from the 2nd-order fits within a column */
#define N_SIGMA 10.0


static float image[XPIX][YPIX],newim[XPIX][YPIX];
int maxi();
int mini();
float median();


main(argc, argv)           /***********Main program start**********/
int argc;
char *argv[];
{
   char  specify,FORCE,ROWMODE, COLLIMIT,fname[80],rootname[80];
   struct BUFFER imhead;
   struct fhead_t *fhbuf;
   double factor,sigma,sum, avg,sumsq;
   float medval,medval2,medval3,mean,stddev,d2,cslope,maxpix,med_cs1,med_cs2;

   int   m, n, ndata, idx, isize, ihsz,i,j, k, l,imin,imax,jmin,jmax,ifile,
         nfit_max,nfit_min,max_n,floor,ceiling,ii;
   float stat[65535];
   FILE *fp1,*fp2;
   double checksum;
   float *a, *x, *y, *sig; 
   int   good_pix, iter;
   int   mfit, *lista, *ivector();
   float alambda,old_alambda,chisq,ochisq,**covar,**alpha,**matrix(),*vector();
   float dchi, old_chi, dlam;
   float estimate, FIT_THRESHOLD, threshold;

   int all_done;
   
   void warm_pulldown(), mrqmin();

   FIT_THRESHOLD = 20.0;
   COLLIMIT=FORCE=specify=FALSE;
   ROWMODE=TRUE;
   isize=5;

/*  Initialize parameter values  */

   strcpy(fname,"NONE");
   
/*    If enough command line parameters have been entered, read them in    */
   if (argc <2)
   {
      printf("  Enter file name on command line");
      exit(0);
   }
  for (ifile=1; ifile<argc; ++ifile)
  {
    sscanf(argv[ifile],"%s",fname);
 
    if (fname[0]=='-')
    {
       switch (fname[1])
       {
          case 'b':
            sscanf(argv[++ifile],"%i",&imin);
            sscanf(argv[++ifile],"%i",&imax);
            sscanf(argv[++ifile],"%i",&jmin);
            sscanf(argv[++ifile],"%i",&jmax);
            specify=TRUE;
            break;
          case 't':
            sscanf(argv[++ifile],"%f",&FIT_THRESHOLD);
            if (FIT_THRESHOLD<0.0)
               FIT_THRESHOLD=100.0;
            break;
          case 'l':
            COLLIMIT=TRUE;
            break;
          case 'c':
            ROWMODE=FALSE;
            break;
          case 'f':
            FORCE=TRUE;
            break;
       }
 
    }
    else
    {
    if (readfits(fname,&imhead,image)!=0)
    {
      printf("Error reading FITS file, program terminating.\n");
      exit(1);
   }

   /* Thresholding on low-valued median is not always effective */
   /* I find this value is not being set by the calling program */
   /* ergo, I force FORCE here */
   FORCE=TRUE; 

   isize=min(isize,32);
   ihsz=isize / 2;

   if (!specify)
   {
      printf("Error - must specify limits with -b\n");
      exit(1);
   } else {
      imin=max(0,min(imin,imhead.naxis1));
      jmin=max(0,min(jmin,imhead.naxis2));
      imax=max(0,min(imax,imhead.naxis1));
      jmax=max(0,min(jmax,imhead.naxis2));
      if (imin>imax)
      {
         m=imax;
         imax=imin;
         imin=m;
      }
      if (jmin>jmax)
      {
         m=jmax;
         jmax=jmin;
         jmin=m;
      }
   }

   /* Initialize fit parameters */


   /* mfit  = MA; */
   /* In this version, we're not going to adjust a[4] */
   mfit  = 3;
   a   = vector(1, MA); 
   lista = ivector(1, MA);
   lista[1] = 1; lista[2] = 2; lista[3] = 3; lista[4] = 4;

   x   = vector(1, YPIX); 
   y   = vector(1, YPIX); 
   sig = vector(1, YPIX); 

   covar = matrix(1, MA, 1, MA); 
   alpha = matrix(1, MA, 1, MA); 


   idx=0;
   mean = d2 = 0;
      /* Calculate median of entire region [imin:imax,jmin:jmax] */
      idx=0;
      for(m=max(imin-1,0); m<min(imhead.naxis1,imax); m++)
         for(n=max(jmin-1,0); n<min(imhead.naxis2,jmax); n++)
            stat[idx++]=image[m][n];
      if (idx==0)
      {
         printf("Error - no points\n");
         exit(1);
      }
      medval=median(stat,idx);

      /* Calculate mean, stddev of entire region [imin:imax,jmin:jmax] */
      for(m=max(imin-1,0); m<min(imhead.naxis1,imax); m++)
      {
         idx=0;
         for(n=max(jmin-1,0); n<min(imhead.naxis2,jmax); n++)
            stat[idx++]=image[m][n];
         medval2=median(stat,idx);
         mean+=medval2;
         d2+=medval2*medval2;
      }
      mean=mean/(min(imhead.naxis1,imax)-max(imin-1,0)+1);
      stddev=sqrt(d2/(min(imhead.naxis1,imax)-max(imin-1,0)+1)-mean*mean);
      printf("%s median: %f  mean: %f  stddev: %f\n",fname,medval,mean,stddev);

      /* This is the threshold used to exclude pixels w/ flux from fits to columns */
      threshold = N_SIGMA * stddev + medval;

      for(m=max(imin-1,0); m<min(imhead.naxis1,imax); m++) {

         /* Compute column-wise median values, put --> medval2 */
         /* Compute required adjustment, then  put --> medval3 */
         idx=0;
         for(n=max(jmin-1,0); n<min(imhead.naxis2,jmax); n++)
            stat[idx++]=image[m][n];
         medval2=median(stat,idx);
         medval3=medval2 - medval;
         /* printf("For column %d the median is %f \n",m,medval2); */

         /* If the values are out of range in this column */
         /* do adjustment for whole column                */
         /* First step is to identify max-valued pixel    */
         /* if ((fabs(medval3)>stddev/1.0)|| FORCE) {  */
         if ((fabs(medval3)>stddev/1.0)|| FORCE) {
             /* maxpix=image[m][0]; */
             maxpix= - 1000.0; 
             max_n=0;
             for (n=0; n<imhead.naxis2; n++) {
                 if (image[m][n]>maxpix) {
                     maxpix=image[m][n];
                     max_n=n;
                 }
             } 

             if (maxpix > (medval + FIT_THRESHOLD)) {

                 /* Figure out how many rows above and below the peak pixel  */
                 /* contain flux from the saturating source.  Pixels with    */
                 /* significant flux must not be used in the fitting process */
		 all_done = FALSE;
		 floor = max_n;
                 for (n=max_n+1; n<imhead.naxis2; n++) {
                     if ((image[m][n]-medval2>0.0) && (all_done == FALSE)){
                         floor=n;
                     } else { 
                         all_done = TRUE;
                     }
                 }
		 /* Add some buffer for faint emission from source */
		 floor+=4; floor = min(imhead.naxis2, floor);

		 all_done = FALSE;
		 ceiling = max_n;
                 for (n=max_n-1; n>0; n--) {
                     if ((image[m][n]-medval2>0.0) && (all_done == FALSE)){
                         ceiling=n;
                 /* printf("pixel %d,%d =%f; medval2=%f, medval=%f, ceiling=%d,
 * diff=%f\n",m,n,image[m][n],medval2,medval,ceiling,(image[m][n]-medval2)); */
                     } else {
                         all_done = TRUE;
                     }
                 }
		 /* Add some buffer for faint emission from source */
		 ceiling-=4; ceiling = max(1, ceiling);


                 /* First examine pixels ABOVE the peak of the column.   */
                 /* This means pixels between "floor" and imhead.naxis2. */
                 /* Find the median of the columns above the peak,       */
                 /* and adjust this to the median of the larger region   */
                 /* originally selected (medval)                         */
                 /* for (n=min(max_n+8,imhead.naxis2); n<imhead.naxis2; n++) */
                 for (n=min(floor,imhead.naxis2); n<imhead.naxis2; n++) 
                     stat[idx++]=image[m][n];
                 medval2=median(stat,idx);
                 printf("Fixing column %03d: max=%7.2f at pixel %03d for source between pixels %03d and %03d\n", m, maxpix, max_n, ceiling, floor);
                 medval3=medval2 - medval;

                 /* This is the new way, we attempt a 2nd-order fit */
                 /* nfit_min=max_n+8; */
                 nfit_min=floor;
                 nfit_max=imhead.naxis2;
                 /* With fewer than 32 pixels, only apply a median correction */
                 if (nfit_max-nfit_min<32) {
                     printf("   Fewer than 32 pixels available for fit\n");
                     for (j=max_n; j<imhead.naxis2; j++)
                         image[m][j]-=medval3;
                 } else {
                     idx=0;
                     for (ii=nfit_min; ii<nfit_min+8; ii++)
                         stat[idx++]=image[m][ii];
                     med_cs1=median(stat,idx);
                     idx=0;
                     for (ii=nfit_max-8; ii<nfit_max; ii++)
                         stat[idx++]=image[m][ii];
                     med_cs2=median(stat,idx);
                     cslope=(med_cs1 - med_cs2 ) / (nfit_max-nfit_min - 8);
                     /* printf("cslope: %f\n",cslope); */

                     /* Begin a 2nd-order fitting procedure */
                     /* Fill vectors with row numbers and pixel values */
                     /* Keep only pixels without strong (> 3-sigma) sources */
                     idx=0;
                     for (j=nfit_min; j<imhead.naxis2; j++){
                         /* estimate = image[m][j] - medval3 + 0.4 * cslope * j; */
                         estimate = image[m][j] - medval3;
                         if (estimate < threshold) {
                             idx++;
                             sig[idx] = stddev;
                             x[idx] = 1.0 * j;
                             y[idx] = image[m][j];
                         }
                     }
                     ndata = idx;

                     /* Trap cases where the above crude slope estimate fails */
                     if (ndata <= MIN_FIT_PIXELS) {
                         idx=0;  
                         for (j=nfit_min; j<imhead.naxis2; j++){
                             idx++;
                             sig[idx] = stddev;
                             x[idx] = 1.0 * j;
                             y[idx] = image[m][j];
                         }
                     }
                     ndata = idx;

                     /* I played with these to determine empirically that they are */
                     /* Good intial guesses, at least for channel 1                */
                     a[1] =  0.75 * medval2;
                     a[2] =  -0.6 * cslope;
                     a[3] =  -5.0e-7;
                     a[4] =  x[ndata] - x[1] / 2.0;

                     /* Verify output */
                     /* printf("Initial guesses to fit %3d pixels of column %3d are const=%f, slope=%f, curve=%9e, pix=%f \n",ndata,m,a[1],a[2],a[3],a[4]); */

                     /* Initialize mrqmin */
                     alambda = -1;
                     mrqmin(x,y,sig,ndata,a,MA,lista,mfit,covar,alpha,&chisq,warm_pulldown,&alambda);

                     iter = 0;
                     chisq = 100;
                     do {
                         iter++;
                         old_chi = chisq;
                         old_alambda = alambda;
                         mrqmin(x,y,sig,ndata,a,MA,lista,mfit,covar,alpha,&chisq,warm_pulldown,&alambda);
                         dchi = old_chi - chisq;
                         dlam = old_alambda - alambda;
                     } while (dchi > TOL || dchi < 0.0 || dlam < 0.0);

                     /* Verify output */
                     /* printf("After %d iterations: fitted const=%f, slope=%f, curve=%9e, pix=%f\n",iter,a[1],a[2],a[3], a[4]); */

                     /* Subtract the fitted curve from the data */
                     for (j=max_n+1; j<imhead.naxis2; j++){
                         image[m][j]-=(a[1] + a[2]*(j - a[4]) + a[3]*(j-a[4])*(j-a[4]) - medval); 
                         /* image[m][j]-=((a[1] + a[2]*j + a[3]*j*j) - medval); */
                     }

                 }


                 /* Do region BELOW the peak.  For each column, fit the    */
                 /* median plus a linear function that varies along column */
                 idx=0;
                 for (n=0; n<min(imhead.naxis2,ceiling); n++)
                     stat[idx++]=image[m][n];
                 medval2=median(stat,idx);
                 /* printf("Lower section median=%f\n",medval2);  */
                 medval3=medval2 - medval;

                 nfit_min=0;
                 /* nfit_max=max_n-8; */
                 nfit_max=ceiling;
                 /* printf("fit min, max range: %i %i\n",nfit_min,nfit_max); */

                 /* With fewer than 32 pixels, only apply a median correction */
                 if (nfit_max-nfit_min<32) {
                     printf("   Fewer than 32 pixels available for fit\n");
                     for (j=0; j<max_n; j++)
                         image[m][j]-=medval3;
                 } else {
                     idx=0;
                     for (ii=nfit_min; ii<nfit_min+8; ii++)
                         stat[idx++]=image[m][ii];
                     med_cs1=median(stat,idx);
                     idx=0;
                     for (ii=nfit_max-8; ii<nfit_max; ii++)
                         stat[idx++]=image[m][ii];
                     med_cs2=median(stat,idx);
                     cslope=(med_cs1 - med_cs2 ) / (nfit_max-nfit_min - 8);

                     /* Begin a 2nd-order fitting procedure */
                     /* Fill vectors with row numbers and pixel values */
                     /* Keep only pixels without strong (> 3-sigma) sources */
                     idx=0;
                     for (j=0; j<nfit_max; j++){ 
                         /* estimate = image[m][j] - medval3 + 0.4 * cslope * j; */
                         estimate = image[m][j] - medval3;
                         if (estimate < threshold) {
                             idx++;
                             sig[idx] = stddev;
                             x[idx] = 1.0 * j;
                             y[idx] = image[m][j];
                         }
                     }
	    	     ndata = idx;

                     /* Trap cases where the above crude slope estimate fails */
                     if (ndata <= MIN_FIT_PIXELS) {
                         idx=0;  
                         for (j=nfit_min; j<imhead.naxis2; j++){
                             idx++;
                             sig[idx] = stddev;
                             x[idx] = 1.0 * j;
                             y[idx] = image[m][j];
                         }
                     }
                     ndata = idx;

    		     /* I played with these to determine empirically that they are */
    		     /* Good intial guesses, at least for channel 1                */
                     a[1] =  0.75 * medval2;
    		     a[2] =  -0.6 * cslope;
		     a[3] =  -5.0e-7;  
                     a[4] =  x[ndata] - x[1] / 2.0;

                     /* Verify output */
                     /* printf("Initial guesses to fit %3d pixels of column %3d are const=%f, slope=%f, curve=%9e pixel=%f\n",ndata, m,a[1],a[2],a[3],a[4]); */

                     /* Initialize mrqmin */
                     alambda = -1;
                     mrqmin(x,y,sig,ndata,a,MA,lista,mfit,covar,alpha,&chisq,warm_pulldown,&alambda);

	    	     iter = 0;
                     chisq = 100;
                     do {
                         iter++;
                         old_chi = chisq;
    		         old_alambda = alambda;
                         mrqmin(x,y,sig,ndata,a,MA,lista,mfit,covar,alpha,&chisq,warm_pulldown,&alambda);
                         dchi = old_chi - chisq;
    		         dlam = old_alambda - alambda;
                     } while (dchi > TOL || dchi < 0.0 || dlam < 0.0);

                     /* Verify output */
                     /* printf("After %d iterations: fitted const=%f, slope=%f, curve=%9e, pix=%f\n",iter,a[1],a[2],a[3],a[4]); */


                     /* Subtract the fitted curve from the data */
                     for (j=0; j<max_n; j++){ 
                         image[m][j]-=(a[1] + a[2]*(j - a[4]) + a[3]*(j-a[4])*(j-a[4]) - medval); 
                     }

                 }
             }
         }   /* End of loop that tests on full-column median pixel value */
      }  /* End of loop over columns */
      printf("\n");

    /* Done processing image */
    writefits(fname,&imhead,image);

    /* free up memory for fits buffer: original FITS header lines are discarded,
    but the parameters such as naxis, bitpix, etc. are saved. */

    free_buffer(&imhead);
   }
   }

   exit(0);
 

}   /************************* End main *************************************/

/*************************************************************************/
/* This implements a functionality for the warm-mission column pulldown  */
/* that acts as a polynomial y = a[1] + a[2] * pix + a[3] * pix * pix    */
/* y = a[1] + a[2] * (x-x') + a[3] * (x-x') * (x-x') */
/*                                                                       */ 
/*************************************************************************/
void warm_pulldown(x,a,y,dyda,ma)
float x, a[], *y, dyda[];
int ma;
{
    dyda[1] = 1.0;
    dyda[2] = x - a[4];
    dyda[3] = (x - a[4]) * (x - a[4]);
    dyda[4] = -1.0 * a[2] + 2.0 * a[3] * (a[4] - x);
    *y = a[1] + a[2] * (x - a[4]) + a[3] * (x - a[4]) * (x - a[4]); 
}


int max(i1,i2)
int i1;
int i2;
{
    if (i1<i2)
        return i2;
    else
        return i1;
}

int min(i1,i2)
int i1;
int i2;
{
    if (i1<i2)
        return i1;
    else
        return i2;
}

float median(a,inum)
float a[];
int inum;
{
    float l,c[65535],b[65535];
    int i,j,k,n,m;

    n=inum;
    for (i=0; i<n; i++)
     b[i]=a[i];

    k=0;
    while (n>1)
    {
       l=b[0]; m=0;
       for (i=0; i<n; i++)
          if (b[i]<l)
          {
             l=b[i];
             m=i;
          }
       c[k++]=l;
       for (i=m; i<n-1; i++)
          b[i]=b[i+1];
       n--;
       if (n==1)
          c[k]=b[0];
    }
    n=inum/2;
    return(c[n]);
     
}



void mrqmin(x,y,sig,ndata,a,ma,lista,mfit,covar,alpha,chisq,funcs,alambda)
float x[],y[],sig[],a[],**covar,**alpha,*chisq,*alambda;
int ndata,ma,lista[],mfit;
void (*funcs)();

/* Levenberg-Marquardt method, attempting to reduce the chi-sqr of a fit
between a set of points x[1..ndata], y[1..ndata] with individual standard
deviations sig[1..ndata], and a nonlinear function dependent on coefficients
a[a..ma]. The array lista[1..ma] numbers the parameters a such that the first
mfit elements correspond to values actually being adjusted; the remaining
ma-mfit parameters are held fixed at their input value.The program returns
current best-fit values for the ma fit parameters a, and chisq.
The [1..mfit][1..mfit] elements of the arrays covar[1..ma][a..ma],
alpha[1..ma][1..ma] are used as working space during most iterations. Supply
a routine funcs(x,a,yfit,dyda,ma) that evaluates the fitting function yfit,
and its derivatives dyda[1..ma] with respect to the fitting parameters a at x
On the first call provide an initial guess for the parameters a, and set
alamda < 0  for initializaton (which then sets alamda = .001). If a step
succeeds chisq becomes smaller and alamda decreases by a factor of ten. If a
step fails, alamda grows by a factor of ten. You must call this routine
repeatedly untill convergence is achieved. Then make one final call with
alamda =0, so that covar returns the covariance matrix, and alpha the
curvature matrix. */

{
	int k,kk,j,ihit;
	static float *da,*atry,**oneda,*beta,ochisq;
	float *vector(),**matrix();
	void mrqcof(),gaussj(),covsrt(),nrerror(),free_matrix(),free_vector();

/*
printf("mrqmin: mfit=%d; initial guess params are a1=%f, a2=%f, a3=%f, a4=%f\n",mfit, a[1],a[2],a[3],a[4]); 
*/

/* Print out the alpha array, for debugging purposes */
/* for (j=1;j<=mfit;j++) {
    printf ("\n");
    for (k=1;k<=mfit;k++) printf ("alpha[%d][%d]=%9e ",j,k,alpha[j][k]);
}
printf ("\n"); */

	if (*alambda < 0.0) {
		oneda=matrix(1,mfit,1,1);
		atry=vector(1,ma);
		da=vector(1,ma);
		beta=vector(1,ma);
		kk=mfit+1;
		for (j=1;j<=ma;j++) {
			ihit=0;
			for (k=1;k<=mfit;k++)
				if (lista[k] == j) ihit++;
			if (ihit == 0)
				lista[kk++]=j;
			else if (ihit > 1) nrerror("Bad LISTA permutation in MRQMIN-1");
		}
		if (kk != ma+1) nrerror("Bad LISTA permutation in MRQMIN-2");
		*alambda=0.001;
		mrqcof(x,y,sig,ndata,a,ma,lista,mfit,alpha,beta,chisq,funcs);
		ochisq=(*chisq);
	}
	for (j=1;j<=mfit;j++) {
		for (k=1;k<=mfit;k++) covar[j][k]=alpha[j][k];
		covar[j][j]=alpha[j][j]*(1.0+(*alambda));
		oneda[j][1]=beta[j];
	}
	gaussj(covar,mfit,oneda,1);
	for (j=1;j<=mfit;j++)
		da[j]=oneda[j][1];
	if (*alambda == 0.0) {
		covsrt(covar,ma,lista,mfit);
		free_vector(beta,1,ma);
		free_vector(da,1,ma);
		free_vector(atry,1,ma);
		free_matrix(oneda,1,mfit,1,1);
		return;
	}
	for (j=1;j<=ma;j++) atry[j]=a[j];
	for (j=1;j<=mfit;j++)
		atry[lista[j]] = a[lista[j]]+da[j];
	mrqcof(x,y,sig,ndata,atry,ma,lista,mfit,covar,da,chisq,funcs);
	if (*chisq < ochisq) {
		*alambda *= 0.1;
		ochisq=(*chisq);
		for (j=1;j<=mfit;j++) {
			for (k=1;k<=mfit;k++) alpha[j][k]=covar[j][k];
			beta[j]=da[j];
			a[lista[j]]=atry[lista[j]];
		}
	} else {
		*alambda *= 10.0;
		*chisq=ochisq;
	}
	return;
}


void mrqcof(x,y,sig,ndata,a,ma,lista,mfit,alpha,beta,chisq,funcs)
float x[],y[],sig[],a[],**alpha,beta[],*chisq;
int ndata,ma,lista[],mfit;
void (*funcs)();	/* ANSI: void (*funcs)(float,float *,float *,float *,int); */
{
	int k,j,i;
	float ymod,wt,sig2i,dy,*dyda,*vector();
	void free_vector();

	dyda=vector(1,ma);
/* printf("mrqcof: mfit=%d; initial guess params are a1=%f, a2=%f, a3=%f; * ndata=%d\n",mfit, a[1],a[2],a[3],ndata); */
	for (j=1;j<=mfit;j++) {
		for (k=1;k<=j;k++) alpha[j][k]=0.0;
		beta[j]=0.0;
	}

/* Print out the alpha array, for debugging purposes */
/*
for (j=1;j<=mfit;j++) {
    printf ("\n");
    for (k=1;k<=mfit;k++) printf ("alpha[%d][%d]=%9e ",j,k,alpha[j][k]);
}
printf ("\n");
*/
	*chisq=0.0;
	for (i=1;i<=ndata;i++) {
		(*funcs)(x[i],a,&ymod,dyda,ma);
		sig2i=1.0/(sig[i]*sig[i]);
		dy=y[i]-ymod;
		for (j=1;j<=mfit;j++) {
			wt=dyda[lista[j]]*sig2i;
			for (k=1;k<=j;k++)
				alpha[j][k] += wt*dyda[lista[k]];
			beta[j] += dy*wt;
		}
		(*chisq) += dy*dy*sig2i;
	}
	for (j=2;j<=mfit;j++)
		for (k=1;k<=j-1;k++) alpha[k][j]=alpha[j][k];
	free_vector(dyda,1,ma);
}


void covsrt(covar,ma,lista,mfit)
float **covar;
int ma,lista[],mfit;
{
	int i,j;
	float swap;

	for (j=1;j<ma;j++)
		for (i=j+1;i<=ma;i++) covar[i][j]=0.0;
	for (i=1;i<mfit;i++)
		for (j=i+1;j<=mfit;j++) {
			if (lista[j] > lista[i])
				covar[lista[j]][lista[i]]=covar[i][j];
			else
				covar[lista[i]][lista[j]]=covar[i][j];
		}
	swap=covar[1][1];
	for (j=1;j<=ma;j++) {
		covar[1][j]=covar[j][j];
		covar[j][j]=0.0;
	}
	covar[lista[1]][lista[1]]=swap;
	for (j=2;j<=mfit;j++) covar[lista[j]][lista[j]]=covar[1][j];
	for (j=2;j<=ma;j++)
		for (i=1;i<=j-1;i++) covar[i][j]=covar[j][i];
}


#define SWAP(a,b) {float temp=(a);(a)=(b);(b)=temp;}

void gaussj(a,n,b,m)
float **a,**b;
int n,m;
{
	int *indxc,*indxr,*ipiv;
	int i,icol,irow,j,k,l,ll,*ivector();
	float big,dum,pivinv;
	void nrerror(),free_ivector();

/* 
printf ("n=%d m=%d\n",n,m);
for (j=1;j<=n;j++) printf ("b[%d] = %f\n",j,b[j]);
for (j=1;j<=n;j++) {
    printf ("\n");
    for (i=1;i<=n;i++) printf ("a[%d][%d]=%9e ",j,i,a[j][i]);
}
printf ("\n");
*/

	indxc=ivector(1,n);
	indxr=ivector(1,n);
	ipiv=ivector(1,n);
	for (j=1;j<=n;j++) ipiv[j]=0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if (ipiv[j] != 1)
				for (k=1;k<=n;k++) {
					if (ipiv[k] == 0) {
						if (fabs(a[j][k]) >= big) {
							big=fabs(a[j][k]);
							irow=j;
							icol=k;
						}
					} else if (ipiv[k] > 1) nrerror("GAUSSJ: Singular Matrix-1");
				}
		++(ipiv[icol]);
		if (irow != icol) {
			for (l=1;l<=n;l++) SWAP(a[irow][l],a[icol][l])
			for (l=1;l<=m;l++) SWAP(b[irow][l],b[icol][l])
		}
		indxr[i]=irow;
		indxc[i]=icol;
		if (a[icol][icol] == 0.0) nrerror("GAUSSJ: Singular Matrix-2");
		pivinv=1.0/a[icol][icol];
		a[icol][icol]=1.0;
		for (l=1;l<=n;l++) a[icol][l] *= pivinv;
		for (l=1;l<=m;l++) b[icol][l] *= pivinv;
		for (ll=1;ll<=n;ll++)
			if (ll != icol) {
				dum=a[ll][icol];
				a[ll][icol]=0.0;
				for (l=1;l<=n;l++) a[ll][l] -= a[icol][l]*dum;
				for (l=1;l<=m;l++) b[ll][l] -= b[icol][l]*dum;
			}
	}
	for (l=n;l>=1;l--) {
		if (indxr[l] != indxc[l])
			for (k=1;k<=n;k++)
				SWAP(a[k][indxr[l]],a[k][indxc[l]]);
	}
	free_ivector(ipiv,1,n);
	free_ivector(indxr,1,n);
	free_ivector(indxc,1,n);
}

#undef SWAP



#include <malloc/malloc.h>
#include <stdio.h>

void nrerror(error_text)
char error_text[];
{
	void exit();

	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}



float *vector(nl,nh)
int nl,nh;
{
	float *v;

	v=(float *)malloc((unsigned) (nh-nl+1)*sizeof(float));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl;
}

int *ivector(nl,nh)
int nl,nh;
{
	int *v;

	v=(int *)malloc((unsigned) (nh-nl+1)*sizeof(int));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl;
}

double *dvector(nl,nh)
int nl,nh;
{
	double *v;

	v=(double *)malloc((unsigned) (nh-nl+1)*sizeof(double));
	if (!v) nrerror("allocation failure in dvector()");
	return v-nl;
}



float **matrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
	int i;
	float **m;

	m=(float **) malloc((unsigned) (nrh-nrl+1)*sizeof(float*));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m -= nrl;

	for(i=nrl;i<=nrh;i++) {
		m[i]=(float *) malloc((unsigned) (nch-ncl+1)*sizeof(float));
		if (!m[i]) nrerror("allocation failure 2 in matrix()");
		m[i] -= ncl;
	}
	return m;
}

double **dmatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
	int i;
	double **m;

	m=(double **) malloc((unsigned) (nrh-nrl+1)*sizeof(double*));
	if (!m) nrerror("allocation failure 1 in dmatrix()");
	m -= nrl;

	for(i=nrl;i<=nrh;i++) {
		m[i]=(double *) malloc((unsigned) (nch-ncl+1)*sizeof(double));
		if (!m[i]) nrerror("allocation failure 2 in dmatrix()");
		m[i] -= ncl;
	}
	return m;
}

int **imatrix(nrl,nrh,ncl,nch)
int nrl,nrh,ncl,nch;
{
	int i,**m;

	m=(int **)malloc((unsigned) (nrh-nrl+1)*sizeof(int*));
	if (!m) nrerror("allocation failure 1 in imatrix()");
	m -= nrl;

	for(i=nrl;i<=nrh;i++) {
		m[i]=(int *)malloc((unsigned) (nch-ncl+1)*sizeof(int));
		if (!m[i]) nrerror("allocation failure 2 in imatrix()");
		m[i] -= ncl;
	}
	return m;
}



float **submatrix(a,oldrl,oldrh,oldcl,oldch,newrl,newcl)
float **a;
int oldrl,oldrh,oldcl,oldch,newrl,newcl;
{
	int i,j;
	float **m;

	m=(float **) malloc((unsigned) (oldrh-oldrl+1)*sizeof(float*));
	if (!m) nrerror("allocation failure in submatrix()");
	m -= newrl;

	for(i=oldrl,j=newrl;i<=oldrh;i++,j++) m[j]=a[i]+oldcl-newcl;

	return m;
}



void free_vector(v,nl,nh)
float *v;
int nl,nh;
{
	free((char*) (v+nl));
}

void free_ivector(v,nl,nh)
int *v,nl,nh;
{
	free((char*) (v+nl));
}

void free_dvector(v,nl,nh)
double *v;
int nl,nh;
{
	free((char*) (v+nl));
}



void free_matrix(m,nrl,nrh,ncl,nch)
float **m;
int nrl,nrh,ncl,nch;
{
	int i;

	for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}

void free_dmatrix(m,nrl,nrh,ncl,nch)
double **m;
int nrl,nrh,ncl,nch;
{
	int i;

	for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}

void free_imatrix(m,nrl,nrh,ncl,nch)
int **m;
int nrl,nrh,ncl,nch;
{
	int i;

	for(i=nrh;i>=nrl;i--) free((char*) (m[i]+ncl));
	free((char*) (m+nrl));
}



void free_submatrix(b,nrl,nrh,ncl,nch)
float **b;
int nrl,nrh,ncl,nch;
{
	free((char*) (b+nrl));
}



float **convert_matrix(a,nrl,nrh,ncl,nch)
float *a;
int nrl,nrh,ncl,nch;
{
	int i,j,nrow,ncol;
	float **m;

	nrow=nrh-nrl+1;
	ncol=nch-ncl+1;
	m = (float **) malloc((unsigned) (nrow)*sizeof(float*));
	if (!m) nrerror("allocation failure in convert_matrix()");
	m -= nrl;
	for(i=0,j=nrl;i<=nrow-1;i++,j++) m[j]=a+ncol*i-ncl;
	return m;
}



void free_convert_matrix(b,nrl,nrh,ncl,nch)
float **b;
int nrl,nrh,ncl,nch;
{
	free((char*) (b+nrl));
}

