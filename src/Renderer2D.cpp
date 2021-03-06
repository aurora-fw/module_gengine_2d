/****************************************************************************
** ┌─┐┬ ┬┬─┐┌─┐┬─┐┌─┐  ┌─┐┬─┐┌─┐┌┬┐┌─┐┬ ┬┌─┐┬─┐┬┌─
** ├─┤│ │├┬┘│ │├┬┘├─┤  ├┤ ├┬┘├─┤│││├┤ ││││ │├┬┘├┴┐
** ┴ ┴└─┘┴└─└─┘┴└─┴ ┴  └  ┴└─┴ ┴┴ ┴└─┘└┴┘└─┘┴└─┴ ┴
** A Powerful General Purpose Framework
** More information in: https://aurora-fw.github.io/
**
** Copyright (C) 2017 Aurora Framework, All rights reserved.
**
** This file is part of the Aurora Framework. This framework is free
** software; you can redistribute it and/or modify it under the terms of
** the GNU Lesser General Public License version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE included in
** the packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
****************************************************************************/

#include <AuroraFW/GEngine/Renderer2D.h>
#include <AuroraFW/GEngine/GL/Buffer.h>
#include <AuroraFW/Core/DebugManager.h>

#define AFW_RENDERER_MAX_SPRITES	60000
#define AFW_RENDERER_SPRITE_SIZE	AFW_RENDERER_VERTEX_SIZE * 4
#define AFW_RENDERER_BUFFER_SIZE	AFW_RENDERER_SPRITE_SIZE * AFW_RENDERER_MAX_SPRITES
#define AFW_RENDERER_INDICES_SIZE	AFW_RENDERER_MAX_SPRITES * 6

#define AFW_SHADER_VERTEX_INDEX 0
#define AFW_SHADER_COLOR_INDEX	1

namespace AuroraFW::GEngine {
	Renderer2D::Renderer2D(API::Renderer* renderer, uint width, uint height )
		: _renderer(renderer)
	{
		_vao = std::unique_ptr<API::VertexArray>(API::VertexArray::Load());
		_vbo = std::unique_ptr<API::VertexBuffer>(API::VertexBuffer::Load(AFW_NULLPTR, AFW_RENDERER_BUFFER_SIZE, API::Buffer::Usage::Dynamic));
		

		reinterpret_cast<API::GLBuffer*>(_vbo.get())->getGLBuffer();
		_vao->bind();
		_vbo->bind();
		glEnableVertexAttribArray(AFW_SHADER_VERTEX_INDEX);
		glEnableVertexAttribArray(AFW_SHADER_COLOR_INDEX);
		glVertexAttribPointer(AFW_SHADER_VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, AFW_RENDERER_VERTEX_SIZE, (const GLvoid*)0);
		glVertexAttribPointer(AFW_SHADER_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, AFW_RENDERER_VERTEX_SIZE, (const GLvoid*)(3 * sizeof(GLfloat)));
#ifdef AFW__DEBUG
		_vbo->unbind();
		_vao->unbind();
#endif // AFW__DEBUG

		uint indices[AFW_RENDERER_INDICES_SIZE];

		int offset = 0;
		for (int i = 0; i < AFW_RENDERER_INDICES_SIZE; i += 6)
		{
			indices[  i  ] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}

		_ibo = std::unique_ptr<API::IndexBuffer>(API::IndexBuffer::Load(indices, AFW_RENDERER_INDICES_SIZE));
	}


	void Renderer2D::submit(const Renderable2D* renderable)
	{
		const Math::Vector3D& pos = renderable->pos();
		const Math::Vector2D& size = renderable->size();
		ColorF color = renderable->color();

		//DebugManager::Log(_debug_count, ": ", color.a);
		_debug_count++;

		_buffer->vertex = pos;
		_buffer->color = color;
		_buffer++;

		_buffer->vertex = Math::Vector3D(pos.x, pos.y + size.y, pos.z);
		_buffer->color = color;
		_buffer++;

		_buffer->vertex = Math::Vector3D(pos.x + size.x, pos.y + size.y, pos.z);
		_buffer->color = color;
		_buffer++;

		_buffer->vertex = Math::Vector3D(pos.x + size.x, pos.y, pos.z);
		_buffer->color = color;
		_buffer++;

		_count += 6;
	}

	void Renderer2D::begin()
	{
		_vbo->bind();
		_buffer = reinterpret_cast<VertexData*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
	}

	void Renderer2D::end()
	{
		glUnmapBuffer(GL_ARRAY_BUFFER);
		_vbo->unbind();
	}

	void Renderer2D::present()
	{
		_renderer->draw(_vao.get(), _ibo.get(), _count);
		_count = 0;
	}
}