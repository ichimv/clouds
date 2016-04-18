#include "fullscreen_quad.h"

#include "GL/glew.h"
#include "log.h"
#include "texture.h"

#include <stdlib.h>

/* http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/ */

void fs_quad_init(FS_Quad* q, int screen_width, int screen_height, Shader s)
{
	q->screen_width = screen_width;
	q->screen_height = screen_height;
	q->shader = s;
	texture_activate(&q->texture);
	texture_activate(&q->texture);
	q->texture.index--;

	/* Create framebuffer */
	glGenFramebuffers(1, &q->framebuffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, q->framebuffer_object);

	/* Create textures to render to */
	glGenTextures(2, q->color_buffers);

	for (GLuint i = 0; i < 2; i++) {
		/* Send an empty texture to OpenGL and set some filtering */
		glBindTexture(GL_TEXTURE_2D, q->color_buffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, q->screen_width, q->screen_height, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		/* Set color attachment */
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, q->color_buffers[i], 0);
	}
	
	/* Depth buffer */
	//glGenRenderbuffers(1, &q->depth_render_buffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, q->depth_render_buffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, q->screen_width, q->screen_height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, q->depth_render_buffer);

	/* Set the list of draw buffers */
	GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, draw_buffers);

	/* Check for errors */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		log("Error: Framebuffer not created!");
	}

	/* Create fullscreen quad */
	float quad[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
	
	glGenVertexArrays(1, &q->vertex_array_object);
	glBindVertexArray(q->vertex_array_object);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void fs_quad_set_as_render_target(FS_Quad q)
{
	glBindFramebuffer(GL_FRAMEBUFFER, q.framebuffer_object);
	glViewport(0, 0, q.screen_width, q.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void fs_quad_render_to_post(FS_Quad q, FS_Quad post)
{
	glBindFramebuffer(GL_FRAMEBUFFER, post.framebuffer_object);
	glViewport(0, 0, post.screen_width, post.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(q.shader.shader_program);

	glActiveTexture(GL_TEXTURE0 + q.texture.index);
	glBindTexture(GL_TEXTURE_2D, q.texture.object);
	GLuint buffer = glGetUniformLocation(q.shader.shader_program, "diffuse_buffer");
	glUniform1i(buffer, q.texture.index);

	glBindVertexArray(q.vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void fs_quad_render_to_screen(FS_Quad q)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, q.screen_width, q.screen_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(q.shader.shader_program);

	glActiveTexture(GL_TEXTURE0 + q.texture.index);
	glBindTexture(GL_TEXTURE_2D, q.texture.object);
	GLuint buffer = glGetUniformLocation(q.shader.shader_program, "HDR_buffer");
	glUniform1i(buffer, q.texture.index);

	glBindVertexArray(q.vertex_array_object);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}