/*
 * Copyright © 2014-2017 Broadcom
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
 * @file vc5_formats.c
 *
 * Contains the table and accessors for VC5 texture and render target format
 * support.
 *
 * The hardware has limited support for texture formats, and extremely limited
 * support for render target formats.  As a result, we emulate other formats
 * in our shader code, and this stores the table for doing so.
 */

#include "util/macros.h"

#include "vc5_context.h"
#include "vc5_format_table.h"

static const struct vc5_format *
get_format(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        if (devinfo->ver >= 41)
                return v3d41_get_format_desc(f);
        else
                return v3d33_get_format_desc(f);
}

bool
vc5_rt_format_supported(const struct v3d_device_info *devinfo,
                        enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        if (!vf)
                return false;

        return vf->rt_type != V3D_OUTPUT_IMAGE_FORMAT_NO;
}

uint8_t
vc5_get_rt_format(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        if (!vf)
                return 0;

        return vf->rt_type;
}

bool
vc5_tex_format_supported(const struct v3d_device_info *devinfo,
                         enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        return vf != NULL;
}

uint8_t
vc5_get_tex_format(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        if (!vf)
                return 0;

        return vf->tex_type;
}

uint8_t
vc5_get_tex_return_size(const struct v3d_device_info *devinfo,
                        enum pipe_format f, enum pipe_tex_compare compare)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        if (!vf)
                return 0;

        if (compare == PIPE_TEX_COMPARE_R_TO_TEXTURE)
                return 16;

        return vf->return_size;
}

uint8_t
vc5_get_tex_return_channels(const struct v3d_device_info *devinfo,
                            enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);

        if (!vf)
                return 0;

        return vf->return_channels;
}

const uint8_t *
vc5_get_format_swizzle(const struct v3d_device_info *devinfo, enum pipe_format f)
{
        const struct vc5_format *vf = get_format(devinfo, f);
        static const uint8_t fallback[] = {0, 1, 2, 3};

        if (!vf)
                return fallback;

        return vf->swizzle;
}

void
vc5_get_internal_type_bpp_for_output_format(const struct v3d_device_info *devinfo,
                                            uint32_t format,
                                            uint32_t *type,
                                            uint32_t *bpp)
{
        if (devinfo->ver >= 41) {
                return v3d41_get_internal_type_bpp_for_output_format(format,
                                                                     type, bpp);
        } else {
                return v3d33_get_internal_type_bpp_for_output_format(format,
                                                                     type, bpp);
        }
}
