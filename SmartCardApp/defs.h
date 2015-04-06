/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *
 * Copyright (C) 2004, William Chia-Wei Cheng.
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE.QPL included in the packaging of this file.
 *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * @(#)$Id: defs.h,v 1.1 2007/03/02 00:16:27 william Exp $
 */

#ifndef _CS530_DEFS_H_
#define _CS530_DEFS_H_

#if __STDC__ || defined(__cplusplus) || defined(c_plusplus)
   /* ANSI || C++ */
#ifdef _NO_PROTO
#define ARGS_DECL(args) ()
#else /* ~_NO_PROTO */
#define ARGS_DECL(args) args
#endif /* _NO_PROTO */
#else
#undef _NO_PROTO
#define _NO_PROTO
#define ARGS_DECL(args) ()
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif /* ~TRUE */

#ifndef NULL
#define NULL 0
#endif /* ~NULL */

#ifndef DIR_SEP
#ifdef WIN32
#define DIR_SEP '\\'
#define DIR_SEP_STR "\\"
#else /* ~WIN32 */
#define DIR_SEP '/'
#define DIR_SEP_STR "/"
#endif /* WIN32 */
#endif /* ~DIR_SEP */

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif /* ~MAXPATHLEN */

#ifndef MAXSTRING
#define MAXSTRING 256
#endif /* ~MAXSTRING */

#ifndef NUM_MIN
#define NUM_MIN(a,b) ((a)>(b)?(b):(a))
#endif /* ~NUM_MIN */

#ifndef NUM_MAX
#define NUM_MAX(a,b) ((a)>(b)?(a):(b))
#endif /* ~NUM_MAX */

#ifndef round
#define round(X) (((X) >= 0) ? (int)((X)+0.5) : (int)((X)-0.5))
#endif

#endif /*_CS530_DEFS_H_*/

