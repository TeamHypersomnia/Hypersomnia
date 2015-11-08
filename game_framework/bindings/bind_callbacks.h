#pragma once
#include <functional>

template<typename Signature>
struct get_member_stdfunction_signature {};

template<typename C, typename ReturnType, typename... Args>
struct get_member_stdfunction_signature<std::function<ReturnType(Args...)> C::* > {
	typedef std::function<ReturnType(Args...)> C::* member_ptr_type;
	
	template <member_ptr_type mem_ptr>
	static void set_callback(C* _this, luabind::object target_callback) {
		_this->*mem_ptr = [target_callback](Args... arguments) {
			return luabind::call_function<ReturnType>(target_callback, arguments...);
		};
	}

	template <member_ptr_type mem_ptr>
	static luabind::object get_callback(C* _this) {
		return luabind::object();
	}

};

#define bind_set_callback(memberstdfunc) get_member_stdfunction_signature<decltype(memberstdfunc)>::set_callback<memberstdfunc>
#define bind_get_callback(memberstdfunc) get_member_stdfunction_signature<decltype(memberstdfunc)>::get_callback<memberstdfunc>
#define bind_callback(memberstdfunc) bind_get_callback(memberstdfunc), bind_set_callback(memberstdfunc)
