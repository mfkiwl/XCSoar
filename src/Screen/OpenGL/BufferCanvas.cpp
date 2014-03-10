/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2014 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/BufferCanvas.hpp"
#include "Screen/OpenGL/Scope.hpp"
#include "Screen/OpenGL/Compatibility.hpp"
#include "Globals.hpp"
#include "Texture.hpp"
#include "FrameBuffer.hpp"
#include "RenderBuffer.hpp"
#include "Init.hpp"

#include <assert.h>

BufferCanvas::BufferCanvas(const Canvas &canvas,
                           UPixelScalar _width, UPixelScalar _height)
  :Canvas({_width, _height}),
   texture(new GLTexture(_width, _height))
{
  assert(canvas.IsDefined());
}

void
BufferCanvas::Create(PixelSize new_size)
{
  assert(!active);

  Destroy();
  texture = new GLTexture(new_size.cx, new_size.cy);

  if (OpenGL::frame_buffer_object && OpenGL::render_buffer_stencil) {
    frame_buffer = new GLFrameBuffer();

    stencil_buffer = new GLRenderBuffer();
    stencil_buffer->Bind();
    PixelSize size = texture->GetAllocatedSize();
    stencil_buffer->Storage(OpenGL::render_buffer_stencil, size.cx, size.cy);
    stencil_buffer->Unbind();
  }

  Canvas::Create(new_size);
  AddSurfaceListener(*this);
}

void
BufferCanvas::Destroy()
{
  assert(!active);

  if (IsDefined()) {
    RemoveSurfaceListener(*this);

    delete stencil_buffer;
    stencil_buffer = nullptr;

    delete frame_buffer;
    frame_buffer = nullptr;

    delete texture;
    texture = nullptr;
  }
}

void
BufferCanvas::Resize(PixelSize new_size)
{
  assert(IsDefined());

  if (new_size == GetSize())
    return;

  texture->ResizeDiscard(new_size);

  if (stencil_buffer != nullptr) {
    stencil_buffer->Bind();
    PixelSize size = texture->GetAllocatedSize();
    stencil_buffer->Storage(OpenGL::render_buffer_stencil, size.cx, size.cy);
    stencil_buffer->Unbind();
  }

  Canvas::Create(new_size);
}

void
BufferCanvas::Begin(Canvas &other)
{
  assert(IsDefined());
  assert(!active);

  Resize(other.GetSize());

  if (frame_buffer != nullptr) {
    /* activate the frame buffer */
    frame_buffer->Bind();
    texture->AttachFramebuffer(FBO::COLOR_ATTACHMENT0);

    if (OpenGL::render_buffer_stencil == OpenGL::render_buffer_depth_stencil)
      /* we don't need a depth buffer, but we must attach it to the
         FBO if the stencil Renderbuffer has one */
      stencil_buffer->AttachFramebuffer(FBO::DEPTH_ATTACHMENT);

    stencil_buffer->AttachFramebuffer(FBO::STENCIL_ATTACHMENT);

    /* save the old viewport */

#ifdef HAVE_GLES
    /* there's no glPushAttrib() on GL/ES; emulate it */
    glGetIntegerv(GL_VIEWPORT, old_viewport);
#else
    glPushAttrib(GL_VIEWPORT_BIT);
#endif

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    old_translate = OpenGL::translate;
    old_size = OpenGL::viewport_size;

    /* configure a new viewport */
    OpenGL::SetupViewport({GetWidth(), GetHeight()});
    OpenGL::translate = {0, 0};
  } else {
    offset = other.offset;
  }

  active = true;
}

void
BufferCanvas::Commit(Canvas &other)
{
  assert(IsDefined());
  assert(active);
  assert(GetWidth() == other.GetWidth());
  assert(GetHeight() == other.GetHeight());

  if (frame_buffer != nullptr) {
    assert(OpenGL::translate.x == 0);
    assert(OpenGL::translate.y == 0);

    frame_buffer->Unbind();

    /* restore the old viewport */

    assert(OpenGL::translate == RasterPoint(0, 0));

#ifdef HAVE_GLES
    /* there's no glPopAttrib() on GL/ES; emulate it */
    glViewport(old_viewport[0], old_viewport[1],
               old_viewport[2], old_viewport[3]);
#else
    glPopAttrib();
#endif

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    OpenGL::translate = old_translate;
    OpenGL::viewport_size = old_size;

    /* copy frame buffer to screen */
    CopyTo(other);
  } else {
    assert(offset == other.offset);

    /* copy screen to texture */
    CopyToTexture(*texture, GetRect());
  }

  active = false;
}

void
BufferCanvas::CopyTo(Canvas &other)
{
  assert(IsDefined());
  assert(!active || frame_buffer != nullptr);

  OpenGL::glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  GLEnable scope(GL_TEXTURE_2D);
  texture->Bind();
  texture->DrawFlipped(other.GetRect(), GetRect());
}

void
BufferCanvas::SurfaceCreated()
{
}

void
BufferCanvas::SurfaceDestroyed()
{
  /* discard the buffer when the Android app is suspended; it needs a
     full redraw to restore it after resuming */

  Destroy();
}
