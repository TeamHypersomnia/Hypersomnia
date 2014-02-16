#pragma once
#include "stdafx.h"
#include "bindings.h"

template<typename Signature>
struct wrap_known;

template<typename Ret, typename... Args>
struct wrap_known<Ret __stdcall (Args...)> {
	template <Ret __stdcall functor(Args...)>
	static Ret invoke(Args... arguments) {
		return functor(arguments...);
	}
};

#define wrap(f) wrap_known<decltype(f)>::invoke<f>

template<typename Signature>
struct wrap_mem;

template<typename Sub, typename Ret, typename... Args>
struct wrap_mem<Ret(__stdcall Sub::*) (Args...)> {
	
	template <Ret(__stdcall Sub::*functor) (Args...)>
	static Ret invoke(Sub* subject, Args... arguments) {
		return (subject->*functor)(arguments...);
	}
};

#define wrap_member(f) wrap_mem<decltype(f)>::invoke<f>

template<typename Signature>
struct wrap_unknown;

template<typename Ret, typename... Args>
struct wrap_unknown<Ret (__stdcall*) (Args...)> {
	template <Ret (__stdcall** functor)(Args...)>
	static Ret invoke(Args... arguments) {
		return (**functor)(arguments...);
	}
};

#define wrap_ptr(f) wrap_unknown<decltype(f)>::invoke<&f>

struct dummy_GL {

};

#include "graphics/fbo.h"
#include "graphics/shader.h"

#include "misc/ptr_wrapper.h"
#include <luabind/tag_function.hpp>


void glUniformMatrix4fvWrap(GLint a, GLsizei b, GLboolean c, ptr_wrapper<float> d) {
	return glUniformMatrix4fv(a, b, c, d);
}

namespace bindings {
	luabind::scope _opengl_binding() {
		return
			luabind::class_<dummy_GL>("GL")
			.scope
			[
				luabind::def("glActiveTexture", wrap_ptr(glActiveTexture)),
				luabind::def("glEnable", wrap(glEnable)),
				luabind::def("glDisable", wrap(glDisable)),
				luabind::def("glClearColor", wrap(glClearColor)),
				luabind::def("glClear", wrap(glClear)),
				luabind::def("glBegin", wrap(glBegin)),
				luabind::def("glVertexAttrib2f", wrap_ptr(glVertexAttrib2f)),
				luabind::def("glVertexAttrib4f", wrap_ptr(glVertexAttrib4f)),
				luabind::def("glEnd", wrap(glEnd)),
				luabind::def("glBindTexture", wrap(glBindTexture)),
				luabind::def("glUseProgram", wrap_ptr(glUseProgram)),
				luabind::def("glGenerateMipmap", wrap_ptr(glGenerateMipmap)),
				luabind::def("glUniform1f", wrap_ptr(glUniform1f)),
				luabind::def("glUniform2f", wrap_ptr(glUniform2f)),
				luabind::def("glUniform3f", wrap_ptr(glUniform3f)),
				luabind::def("glUniform4f", wrap_ptr(glUniform4f)),
				luabind::def("glUniform1i", wrap_ptr(glUniform1i)),
				luabind::def("glUniform2i", wrap_ptr(glUniform2i)),
				luabind::def("glUniform3i", wrap_ptr(glUniform3i)),
				luabind::def("glUniform4i", wrap_ptr(glUniform4i)),
				luabind::def("glUniformMatrix4fv", &glUniformMatrix4fvWrap),
				luabind::def("glGetUniformLocation", wrap_ptr(glGetUniformLocation))
			].
			enum_("constants")[
				luabind::value("GL_TEXTURE_2D", GL_TEXTURE_2D),
				luabind::value("GL_TEXTURE0", GL_TEXTURE0),
				luabind::value("GL_TEXTURE1", GL_TEXTURE1),
				luabind::value("GL_TEXTURE2", GL_TEXTURE2),
				luabind::value("GL_TEXTURE3", GL_TEXTURE3),
				luabind::value("GL_TEXTURE4", GL_TEXTURE4),
				luabind::value("GL_TEXTURE5", GL_TEXTURE5),
				luabind::value("GL_TEXTURE6", GL_TEXTURE6),
				luabind::value("GL_TEXTURE7", GL_TEXTURE7),
				luabind::value("GL_TEXTURE8", GL_TEXTURE8),
				luabind::value("GL_QUADS", GL_QUADS),
				luabind::value("GL_LINES", GL_LINES),
				luabind::value("GL_TRIANGLES", GL_TRIANGLES),
				luabind::value("GL_FALSE", GL_FALSE),
				luabind::value("GL_TRUE", GL_TRUE),
				luabind::value("GL_VERTEX_SHADER", GL_VERTEX_SHADER),
				luabind::value("GL_FRAGMENT_SHADER", GL_FRAGMENT_SHADER),
				luabind::value("GL_GEOMETRY_SHADER", GL_GEOMETRY_SHADER),
				luabind::value("GL_COLOR_BUFFER_BIT", GL_COLOR_BUFFER_BIT)
			],

			luabind::class_<fbo>("framebuffer_object")
			.def(luabind::constructor<>())
			.def(luabind::constructor<int, int>())
			.def("create", &fbo::create)
			.def("destroy", &fbo::destroy)
			.def("use", &fbo::use)
			.def("guarded_use", &fbo::guarded_use)
			.def("get_texture_id", &fbo::get_texture_id)
			.scope [
				luabind::def("use_default", &fbo::use_default)
			],

			luabind::class_<shader>("GLSL_shader")
			.def(luabind::constructor<>())
			.def(luabind::constructor<GLuint, std::string>())
			.def("create", &shader::create)
			.def("destroy", &shader::destroy),

			luabind::class_<shader_program>("GLSL_program")
			.def(luabind::constructor<>())
			.def_readwrite("id", &shader_program::id)
			.def("attach", &shader_program::attach)
			.def("build", &shader_program::build)
			.def("use", &shader_program::use)
			.def("destroy", &shader_program::destroy)

			;
	}
}