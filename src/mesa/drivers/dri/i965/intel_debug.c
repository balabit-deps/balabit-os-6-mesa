/*
 * Copyright 2003 VMware, Inc.
 * Copyright © 2006 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file intel_debug.c
 *
 * Support for the INTEL_DEBUG environment variable, along with other
 * miscellaneous debugging code.
 */

#include "brw_context.h"
#include "intel_debug.h"
#include "util/u_atomic.h" /* for p_atomic_cmpxchg */
#include "util/debug.h"

uint64_t INTEL_DEBUG = 0;

static const struct debug_control debug_control[] = {
   { "tex",         DEBUG_TEXTURE},
   { "state",       DEBUG_STATE},
   { "blit",        DEBUG_BLIT},
   { "mip",         DEBUG_MIPTREE},
   { "fall",        DEBUG_PERF},
   { "perf",        DEBUG_PERF},
   { "perfmon",     DEBUG_PERFMON},
   { "bat",         DEBUG_BATCH},
   { "pix",         DEBUG_PIXEL},
   { "buf",         DEBUG_BUFMGR},
   { "fbo",         DEBUG_FBO},
   { "fs",          DEBUG_WM },
   { "gs",          DEBUG_GS},
   { "sync",        DEBUG_SYNC},
   { "prim",        DEBUG_PRIMS },
   { "vert",        DEBUG_VERTS },
   { "dri",         DEBUG_DRI },
   { "sf",          DEBUG_SF },
   { "stats",       DEBUG_STATS },
   { "wm",          DEBUG_WM },
   { "urb",         DEBUG_URB },
   { "vs",          DEBUG_VS },
   { "clip",        DEBUG_CLIP },
   { "aub",         DEBUG_AUB },
   { "shader_time", DEBUG_SHADER_TIME },
   { "no16",        DEBUG_NO16 },
   { "blorp",       DEBUG_BLORP },
   { "nodualobj",   DEBUG_NO_DUAL_OBJECT_GS },
   { "optimizer",   DEBUG_OPTIMIZER },
   { "ann",         DEBUG_ANNOTATION },
   { "no8",         DEBUG_NO8 },
   { "vec4",        DEBUG_VEC4VS },
   { "spill_fs",    DEBUG_SPILL_FS },
   { "spill_vec4",  DEBUG_SPILL_VEC4 },
   { "cs",          DEBUG_CS },
   { "hex",         DEBUG_HEX },
   { "nocompact",   DEBUG_NO_COMPACTION },
   { "hs",          DEBUG_TCS },
   { "tcs",         DEBUG_TCS },
   { "ds",          DEBUG_TES },
   { "tes",         DEBUG_TES },
   { "l3",          DEBUG_L3 },
   { NULL,    0 }
};

uint64_t
intel_debug_flag_for_shader_stage(gl_shader_stage stage)
{
   uint64_t flags[] = {
      [MESA_SHADER_VERTEX] = DEBUG_VS,
      [MESA_SHADER_TESS_CTRL] = DEBUG_TCS,
      [MESA_SHADER_TESS_EVAL] = DEBUG_TES,
      [MESA_SHADER_GEOMETRY] = DEBUG_GS,
      [MESA_SHADER_FRAGMENT] = DEBUG_WM,
      [MESA_SHADER_COMPUTE] = DEBUG_CS,
   };
   STATIC_ASSERT(MESA_SHADER_STAGES == 6);
   return flags[stage];
}

void
brw_process_intel_debug_variable(void)
{
   uint64_t intel_debug = parse_debug_string(getenv("INTEL_DEBUG"), debug_control);
   (void) p_atomic_cmpxchg(&INTEL_DEBUG, 0, intel_debug);
}
