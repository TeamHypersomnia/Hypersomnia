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

#include "../resources/fbo.h"

namespace bindings {
	luabind::scope _opengl_binding() {
		return
			luabind::class_<dummy_GL>("GL")
			.scope
			[
				luabind::def("glEnable", wrap(glEnable)),
				luabind::def("glDisable", wrap(glDisable)),
				luabind::def("glColor4f", wrap(glColor4f)),
				luabind::def("glClear", wrap(glClear)),
				luabind::def("glBegin", wrap(glBegin)),
				luabind::def("glEnd", wrap(glEnd)),
				luabind::def("glVertex2f", wrap(glVertex2f)),
				luabind::def("glBindTexture", wrap(glBindTexture)),
				luabind::def("glUseProgram", wrap_ptr(glUseProgram)),
				luabind::def("glGenerateMipmap", wrap_ptr(glGenerateMipmap)),
				luabind::def("glUniform1f", wrap_ptr(glUniform1f)),
				luabind::def("glUniform2f", wrap_ptr(glUniform2f)),
				luabind::def("glUniform3f", wrap_ptr(glUniform3f)),
				luabind::def("glUniform4f", wrap_ptr(glUniform4f))
			].
			enum_("constants")[
				luabind::value("GL_TEXTURE_2D", GL_TEXTURE_2D),
				luabind::value("GL_QUADS", GL_QUADS),
				luabind::value("GL_LINES", GL_LINES),
				luabind::value("GL_TRIANGLES", GL_TRIANGLES)
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
			]


			;
	}
}