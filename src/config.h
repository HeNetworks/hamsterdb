/*
 * Copyright (C) 2005-2014 Christoph Rupp (chris@crupp.de).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HAM_CONFIG_H__
#define HAM_CONFIG_H__

#include <ham/types.h>

// include autoconf header file; otherwise, assume sane default values
#ifdef HAVE_CONFIG_H
#   include "../config.h"
#else
#   ifdef UNDER_CE
#       define HAVE_MMAP          0
#       define HAVE_UNMMAP        0
#   else
#       define HAVE_MMAP          1
#       define HAVE_UNMMAP        1
#   endif
#   define HAVE_PREAD             1
#   define HAVE_PWRITE            1
#endif

// check for a valid build
#if (!defined(HAM_DEBUG))
#   if (defined(_DEBUG) || defined(DEBUG))
#     define HAM_DEBUG 1
#   endif
#endif

// The endian-architecture of the host computer; set this to
// HAM_LITTLE_ENDIAN or HAM_BIG_ENDIAN. Little endian is the
// default setting
#ifndef HAM_LITTLE_ENDIAN
#   ifndef HAM_BIG_ENDIAN
#       ifdef WORDS_BIGENDIAN
#           define HAM_BIG_ENDIAN           1
#       else // default
#           define HAM_LITTLE_ENDIAN        1
#       endif
#   endif
#endif

// the default cache size is 2 MB
#define HAM_DEFAULT_CACHESIZE    (2 * 1024 * 1024)

// the default page size is 16 kb
#define HAM_DEFAULT_PAGESIZE     (16 * 1024)

// use tcmalloc?
#if HAVE_GOOGLE_TCMALLOC_H == 1
#  if HAVE_LIBTCMALLOC_MINIMAL == 1
#    define HAM_USE_TCMALLOC 1
#  endif
#endif

#include <stddef.h>
#define OFFSETOF(type, member) offsetof(type, member)

#endif /* HAM_CONFIG_H__ */
