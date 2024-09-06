/*---------------------------------------------------------------
           FILE: swapbytes.h
      $Revision: 2.1 $
         $State: Exp $
  Last submitted $Date: 94/10/26 18:23:07 $ $Author: jh $

   Locked out by $Locker: jh $

  REVISION HISTORY
  $Log:	swapbytes.h,v $
 * Revision 2.1  94/10/26  18:23:07  jh
 * Separated data-moving function from byte-swapping function.  This is a major
 * change that is not backward-compatible!  The number of arguments has changed
 * in all functions.  Swaps occur in place.  To do a move-with-swap, use
 * memmove prior to calling these functions to move the data to the new place
 * first.
 * 
 * Revision 1.1  94/10/26  16:17:49  jh
 * Initial revision
 * 

  $Source: /u1/jh/comp/C/RCS/swapbytes.h,v $

  Written in 1994 by Joseph Harrington <jh@mit.edu>.
  This file may be freely distributed.  If you change the
  external behavior, please rename the file, functions, and
  macros to something else, so other people who use these
  functions won't be confused.  Please document all changes
  in the space above.  The author welcomes suggestions.
---------------------------------------------------------------*/
#ifndef SWAPBYTES_PROTECT
#ifndef LINT
static char rcsidswapbytesh[] = "$Header: /u1/jh/comp/C/RCS/swapbytes.h,v 2.1 94/10/26 18:23:07 jh Exp Locker: jh $";
static char copyrightswapbytesh[] =
    "Written in 1994 by Joseph Harrington <jh@mit.edu>.";
#endif
#endif
/*---------------------------------------------------------------
swapbytes.h = include file for swapbytes.c
---------------------------------------------------------------*/
#ifndef SWAPBYTES_PROTECT
#define SWAPBYTES_PROTECT

/* inclusions */

#include <stddef.h>
#include <string.h>

/* definitions */

/* Define the swap routines generically.  The most common reason to swap */
/* bytes is to transport data from one computing architecture to another. */
/* These definitions make the decision whether or not to swap in such */
/* situations in one place (here), not all over the main code.  The */
/* "missing" definitions result in empty statements (no-ops) on machines */
/* where the data are already in the right format. */

/* #if (defined(ultrix) || defined(linux))*/
#define LITTLEENDIAN
/* #else
#define BIGENDIAN
#endif
*/

#ifdef BIGENDIAN
#define Sble2(ptr, n_swap) swap2bytes((ptr), (n_swap))
#define Sble4(ptr, n_swap) swap4bytes((ptr), (n_swap))
#define Sble8(ptr, n_swap) swap8bytes((ptr), (n_swap))
#define Sbbe2(ptr, n_swap)
#define Sbbe4(ptr, n_swap)
#define Sbbe8(ptr, n_swap)
#else
#define Sble2(ptr, n_swap)
#define Sble4(ptr, n_swap)
#define Sble8(ptr, n_swap)
#define Sbbe2(ptr, n_swap) swap2bytes((ptr), (n_swap))
#define Sbbe4(ptr, n_swap) swap4bytes((ptr), (n_swap))
#define Sbbe8(ptr, n_swap) swap8bytes((ptr), (n_swap))
#endif

/* prototypes */

void swap2bytes(void *ptr, size_t n_swap);
void swap4bytes(void *ptr, size_t n_swap);
void swap8bytes(void *ptr, size_t n_swap);

#endif
