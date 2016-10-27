/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

/**
 * @file
 * VMware SVGA specific winsys interface.
 * 
 * @author Jose Fonseca <jfonseca@vmware.com>
 * 
 * Documentation taken from the VMware SVGA DDK.
 */

#ifndef SVGA_WINSYS_H_
#define SVGA_WINSYS_H_


#include "svga_types.h"
#include "svga_reg.h"
#include "svga3d_reg.h"

#include "pipe/p_compiler.h"
#include "pipe/p_defines.h"


struct svga_winsys_screen;
struct svga_winsys_buffer;
struct pipe_screen;
struct pipe_context;
struct pipe_fence_handle;
struct pipe_resource;
struct svga_region;
struct winsys_handle;


#define SVGA_BUFFER_USAGE_PINNED  (1 << 0)
#define SVGA_BUFFER_USAGE_WRAPPED (1 << 1)
#define SVGA_BUFFER_USAGE_SHADER  (1 << 2)

/**
 * Relocation flags to help with dirty tracking
 * SVGA_RELOC_WRITE -   The command will cause a GPU write to this
 *                      resource.
 * SVGA_RELOC_READ -    The command will cause a GPU read from this
 *                      resource.
 * SVGA_RELOC_INTERNAL  The command will only transfer data internally
 *                      within the resource, and optionally clear
 *                      dirty bits
 * SVGA_RELOC_DMA -     Only set for resource buffer DMA uploads for winsys
 *                      implementations that want to track the amount
 *                      of such data referenced in the command stream.
 */
#define SVGA_RELOC_WRITE          (1 << 0)
#define SVGA_RELOC_READ           (1 << 1)
#define SVGA_RELOC_INTERNAL       (1 << 2)
#define SVGA_RELOC_DMA            (1 << 3)

#define SVGA_FENCE_FLAG_EXEC      (1 << 0)
#define SVGA_FENCE_FLAG_QUERY     (1 << 1)

#define SVGA_SURFACE_USAGE_SHARED  (1 << 0)
#define SVGA_SURFACE_USAGE_SCANOUT (1 << 1)

#define SVGA_QUERY_FLAG_SET        (1 << 0)
#define SVGA_QUERY_FLAG_REF        (1 << 1)

#define SVGA_HINT_FLAG_CAN_PRE_FLUSH (1 << 0)  /* Can preemptively flush */

/** Opaque surface handle */
struct svga_winsys_surface;

/** Opaque guest-backed objects */
struct svga_winsys_gb_shader;
struct svga_winsys_gb_query;


/**
 * SVGA per-context winsys interface.
 */
struct svga_winsys_context
{
   void
   (*destroy)(struct svga_winsys_context *swc);

   void *       
   (*reserve)(struct svga_winsys_context *swc, 
	      uint32_t nr_bytes, uint32_t nr_relocs );
   
   /**
    * Emit a relocation for a host surface.
    * 
    * @param flags bitmask of SVGA_RELOC_* flags
    * 
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*surface_relocation)(struct svga_winsys_context *swc, 
	                 uint32 *sid,
                         uint32 *mobid,
	                 struct svga_winsys_surface *surface,
	                 unsigned flags);
   
   /**
    * Emit a relocation for a guest memory region.
    * 
    * @param flags bitmask of SVGA_RELOC_* flags
    * 
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*region_relocation)(struct svga_winsys_context *swc, 
	                struct SVGAGuestPtr *ptr, 
	                struct svga_winsys_buffer *buffer,
	                uint32 offset,
                        unsigned flags);

   /**
    * Emit a relocation for a guest-backed shader object.
    * 
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*shader_relocation)(struct svga_winsys_context *swc, 
	                uint32 *shid,
			uint32 *mobid,
			uint32 *offset,
	                struct svga_winsys_gb_shader *shader,
                        unsigned flags);

   /**
    * Emit a relocation for a guest-backed context.
    * 
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*context_relocation)(struct svga_winsys_context *swc, uint32 *cid);

   /**
    * Emit a relocation for a guest Memory OBject.
    *
    * @param flags bitmask of SVGA_RELOC_* flags
    * @param offset_into_mob Buffer starts at this offset into the MOB.
    *
    * Note that not all commands accept an offset into the MOB and
    * those commands can't use suballocated buffer pools. To trap
    * errors from improper buffer pool usage, set the offset_into_mob
    * pointer to NULL.
    */
   void
   (*mob_relocation)(struct svga_winsys_context *swc,
		     SVGAMobId *id,
		     uint32 *offset_into_mob,
		     struct svga_winsys_buffer *buffer,
		     uint32 offset,
		     unsigned flags);

   /**
    * Emit a relocation for a guest-backed query object.
    *
    * NOTE: Order of this call does matter. It should be the same order
    * as relocations appear in the command buffer.
    */
   void
   (*query_relocation)(struct svga_winsys_context *swc,
	               SVGAMobId *id,
	               struct svga_winsys_gb_query *query);

   /**
    * Bind queries to context.
    * \param flags  exactly one of SVGA_QUERY_FLAG_SET/REF
    */
   enum pipe_error
   (*query_bind)(struct svga_winsys_context *sws,
                 struct svga_winsys_gb_query *query,
                 unsigned flags);

   void
   (*commit)(struct svga_winsys_context *swc);
   
   enum pipe_error
   (*flush)(struct svga_winsys_context *swc, 
	    struct pipe_fence_handle **pfence);

   /** 
    * Context ID used to fill in the commands
    * 
    * Context IDs are arbitrary small non-negative integers,
    * global to the entire SVGA device.
    */
   uint32 cid;

   /**
    * Flags to hint the current context state
    */
   uint32 hints;

   /**
    ** BEGIN new functions for guest-backed surfaces.
    **/

   boolean have_gb_objects;

   /**
    * Map a guest-backed surface.
    * \param flags  bitmaks of PIPE_TRANSFER_x flags
    *
    * The surface_map() member is allowed to fail due to a
    * shortage of command buffer space, if the
    * PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE bit is set in flags.
    * In that case, the caller must flush the current command
    * buffer and reissue the map.
    */
   void *
   (*surface_map)(struct svga_winsys_context *swc,
                  struct svga_winsys_surface *surface,
                  unsigned flags, boolean *retry);

   /**
    * Unmap a guest-backed surface.
    * \param rebind  returns a flag indicating whether the caller should
    *                issue a SVGA3D_BindGBSurface() call.
    */
   void
   (*surface_unmap)(struct svga_winsys_context *swc,
                    struct svga_winsys_surface *surface,
                    boolean *rebind);

   /**
    * Create and define a DX GB shader that resides in the device COTable.
    * Caller of this function will issue the DXDefineShader command.
    */
   struct svga_winsys_gb_shader *
   (*shader_create)(struct svga_winsys_context *swc,
                    uint32 shaderId,
                    SVGA3dShaderType shaderType,
                    const uint32 *bytecode,
                    uint32 bytecodeLen);

   /**
    * Destroy a DX GB shader.
    * This function will issue the DXDestroyShader command.
    */
   void
   (*shader_destroy)(struct svga_winsys_context *swc,
                     struct svga_winsys_gb_shader *shader);

   /**
    * Rebind a DX GB resource to a context.
    * This is called to reference a DX GB resource in the command stream in
    * order to page in the associated resource in case the memory has been
    * paged out, and to fence it if necessary after command submission.
    */
   enum pipe_error
   (*resource_rebind)(struct svga_winsys_context *swc,
                      struct svga_winsys_surface *surface,
                      struct svga_winsys_gb_shader *shader,
                      unsigned flags);
};


/**
 * SVGA per-screen winsys interface.
 */
struct svga_winsys_screen
{
   void
   (*destroy)(struct svga_winsys_screen *sws);
   
   SVGA3dHardwareVersion
   (*get_hw_version)(struct svga_winsys_screen *sws);

   boolean
   (*get_cap)(struct svga_winsys_screen *sws,
              SVGA3dDevCapIndex index,
              SVGA3dDevCapResult *result);
   
   /**
    * Create a new context.
    *
    * Context objects encapsulate all render state, and shader
    * objects are per-context.
    *
    * Surfaces are not per-context. The same surface can be shared
    * between multiple contexts, and surface operations can occur
    * without a context.
    */
   struct svga_winsys_context *
   (*context_create)(struct svga_winsys_screen *sws);
   
   
   /**
    * This creates a "surface" object in the SVGA3D device.
    *
    * \param sws Pointer to an svga_winsys_context
    * \param flags Device surface create flags
    * \param format Format Device surface format
    * \param usage Winsys usage: bitmask of SVGA_SURFACE_USAGE_x flags
    * \param size Surface size given in device format
    * \param numLayers Number of layers of the surface (or cube faces)
    * \param numMipLevels Number of mipmap levels for each face
    *
    * Returns the surface ID (sid). Surfaces are generic
    * containers for host VRAM objects like textures, vertex
    * buffers, and depth/stencil buffers.
    *
    * Surfaces are hierarchial:
    *
    * - Surface may have multiple faces (for cube maps)
    *
    * - Each face has a list of mipmap levels
    *
    * - Each mipmap image may have multiple volume
    *   slices for 3D image, or multiple 2D slices for texture array.
    *
    * - Each slice is a 2D array of 'blocks'
    *
    * - Each block may be one or more pixels.
    *   (Usually 1, more for DXT or YUV formats.)
    *
    * Surfaces are generic host VRAM objects. The SVGA3D device
    * may optimize surfaces according to the format they were
    * created with, but this format does not limit the ways in
    * which the surface may be used. For example, a depth surface
    * can be used as a texture, or a floating point image may
    * be used as a vertex buffer. Some surface usages may be
    * lower performance, due to software emulation, but any
    * usage should work with any surface.
    */
   struct svga_winsys_surface *
   (*surface_create)(struct svga_winsys_screen *sws,
                     SVGA3dSurfaceFlags flags,
                     SVGA3dSurfaceFormat format,
                     unsigned usage,
                     SVGA3dSize size,
                     uint32 numLayers,
                     uint32 numMipLevels,
                     unsigned sampleCount);

   /**
    * Creates a surface from a winsys handle.
    * Used to implement pipe_screen::resource_from_handle.
    */
   struct svga_winsys_surface *
   (*surface_from_handle)(struct svga_winsys_screen *sws,
                          struct winsys_handle *whandle,
                          SVGA3dSurfaceFormat *format);

   /**
    * Get a winsys_handle from a surface.
    * Used to implement pipe_screen::resource_get_handle.
    */
   boolean
   (*surface_get_handle)(struct svga_winsys_screen *sws,
                         struct svga_winsys_surface *surface,
                         unsigned stride,
                         struct winsys_handle *whandle);

   /**
    * Whether this surface is sitting in a validate list
    */
   boolean
   (*surface_is_flushed)(struct svga_winsys_screen *sws,
                         struct svga_winsys_surface *surface);

   /**
    * Reference a SVGA3D surface object. This allows sharing of a
    * surface between different objects.
    */
   void 
   (*surface_reference)(struct svga_winsys_screen *sws,
			struct svga_winsys_surface **pdst,
			struct svga_winsys_surface *src);

   /**
    * Check if a resource (texture, buffer) of the given size
    * and format can be created.
    * \Return TRUE if OK, FALSE if too large.
    */
   boolean
   (*surface_can_create)(struct svga_winsys_screen *sws,
                         SVGA3dSurfaceFormat format,
                         SVGA3dSize size,
                         uint32 numLayers,
                         uint32 numMipLevels);

   /**
    * Buffer management. Buffer attributes are mostly fixed over its lifetime.
    *
    * @param usage bitmask of SVGA_BUFFER_USAGE_* flags.
    *
    * alignment indicates the client's alignment requirements, eg for
    * SSE instructions.
    */
   struct svga_winsys_buffer *
   (*buffer_create)( struct svga_winsys_screen *sws, 
	             unsigned alignment, 
	             unsigned usage,
	             unsigned size );

   /** 
    * Map the entire data store of a buffer object into the client's address.
    * usage is a bitmask of PIPE_TRANSFER_*
    */
   void *
   (*buffer_map)( struct svga_winsys_screen *sws, 
	          struct svga_winsys_buffer *buf,
		  unsigned usage );
   
   void 
   (*buffer_unmap)( struct svga_winsys_screen *sws, 
                    struct svga_winsys_buffer *buf );

   void 
   (*buffer_destroy)( struct svga_winsys_screen *sws,
	              struct svga_winsys_buffer *buf );


   /**
    * Reference a fence object.
    */
   void
   (*fence_reference)( struct svga_winsys_screen *sws,
                       struct pipe_fence_handle **pdst,
                       struct pipe_fence_handle *src );

   /**
    * Checks whether the fence has been signalled.
    * \param flags  driver-specific meaning
    * \return zero on success.
    */
   int (*fence_signalled)( struct svga_winsys_screen *sws,
                           struct pipe_fence_handle *fence,
                           unsigned flag );

   /**
    * Wait for the fence to finish.
    * \param flags  driver-specific meaning
    * \return zero on success.
    */
   int (*fence_finish)( struct svga_winsys_screen *sws,
                        struct pipe_fence_handle *fence,
                        unsigned flag );


   /**
    ** BEGIN new functions for guest-backed surfaces.
    **/

   /** Are guest-backed objects enabled? */
   bool have_gb_objects;

   /** Can we do DMA with guest-backed objects enabled? */
   bool have_gb_dma;

   /**
    * Create and define a GB shader.
    */
   struct svga_winsys_gb_shader *
   (*shader_create)(struct svga_winsys_screen *sws,
		    SVGA3dShaderType shaderType,
		    const uint32 *bytecode,
		    uint32 bytecodeLen);

   /**
    * Destroy a GB shader. It's safe to call this function even
    * if the shader is referenced in a context's command stream.
    */
   void
   (*shader_destroy)(struct svga_winsys_screen *sws,
		     struct svga_winsys_gb_shader *shader);

   /**
    * Create and define a GB query.
    */
   struct svga_winsys_gb_query *
   (*query_create)(struct svga_winsys_screen *sws, uint32 len);

   /**
    * Destroy a GB query.
    */
   void
   (*query_destroy)(struct svga_winsys_screen *sws,
		    struct svga_winsys_gb_query *query);

   /**
    * Initialize the query state of the query that resides in the slot
    * specified in offset
    * \return zero on success.
    */
   int
   (*query_init)(struct svga_winsys_screen *sws,
                       struct svga_winsys_gb_query *query,
                       unsigned offset,
                       SVGA3dQueryState queryState);

   /**
    * Inquire for the query state and result of the query that resides
    * in the slot specified in offset
    */
   void
   (*query_get_result)(struct svga_winsys_screen *sws,
                       struct svga_winsys_gb_query *query,
                       unsigned offset,
                       SVGA3dQueryState *queryState,
                       void *result, uint32 resultLen);

   /** Have VGPU v10 hardware? */
   boolean have_vgpu10;

   /** To rebind resources at the beginnning of a new command buffer */
   boolean need_to_rebind_resources;
};


struct svga_winsys_screen *
svga_winsys_screen(struct pipe_screen *screen);

struct svga_winsys_context *
svga_winsys_context(struct pipe_context *context);

struct pipe_resource *
svga_screen_buffer_wrap_surface(struct pipe_screen *screen,
				enum SVGA3dSurfaceFormat format,
				struct svga_winsys_surface *srf);

struct svga_winsys_surface *
svga_screen_buffer_get_winsys_surface(struct pipe_resource *buffer);

#endif /* SVGA_WINSYS_H_ */