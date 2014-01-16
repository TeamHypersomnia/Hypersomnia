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
	static Ret(__stdcall *cached_function)(Args...);
	
	static Ret invoke(Args... arguments) {
		return cached_function(arguments...);
	}
};

/* static member definition and initialization */
template<typename Ret, typename... Args>
Ret(__stdcall *wrap_unknown<Ret(__stdcall*) (Args...)>::cached_function)(Args...) = nullptr;

/* note I can't use macro here as I need to perform additional operation */
template <typename F>
auto wrap_ptr(F f) -> decltype(&wrap_unknown<F>::invoke) {
	wrap_unknown<F>::cached_function = f;
	return &wrap_unknown<F>::invoke;
}

struct A {
	int __stdcall my_method(double b) {
		return 2;
	}
};

namespace bindings {
	luabind::scope _opengl_binding() {
		return
			luabind::def("Clear", wrap(glClear)),
			luabind::def("glVertex4f", wrap(glVertex4f)),
			luabind::def("Uniform2f", wrap_ptr(glUniform2f)),
			luabind::def("Uniform3f", wrap_ptr(glUniform3f)),
			luabind::def("Uniform4f", wrap_ptr(glUniform4f)),
			luabind::def("Uniform1f", wrap_ptr(glUniform1i)),
			luabind::def("Uniform2f", wrap_ptr(glUniform2i)),
			luabind::def("Uniform3f", wrap_ptr(glUniform3i)),
			luabind::def("Uniform4f", wrap_ptr(glUniform4i)),
			luabind::def("GetUniformLocation", wrap_ptr(glGetUniformLocation)),

			luabind::class_<A>("A")
			.def("my_method", wrap_member(&A::my_method))
			;
	}
}