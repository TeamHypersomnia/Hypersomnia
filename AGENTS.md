- Basic cpp footguns
	- Avoid `using namespace std;`
	- Use `static_cast<>` and the like instead of `(type)obj;`

- Prefer initialization with ``auto`` or ``const auto``.
	- `const auto abc = 2;` instead of `int abc = 2;`
	- Likewise with classes: `auto object = some_class(2, 3);`

- ```const``` wherever possible even at the cost of readability. 
	- In particular, ```const``` every possible function argument.
	- But don't ```const``` arguments taken by copy in the function declarations as this is redundant.

- Use tabs for indentation.

- Use uncapitalized ```snake_case``` everywhere.
	- Except for template parameters: use CamelCase for them.
		- `template <class T, class... Rest>`
		- `template <class T, class... Args>` for variadic functions, with `args...` as the argument name.
		- Prefer a descriptive name instead `T` if there's one template argument.

- [Linux kernel indentation style](https://en.wikipedia.org/wiki/Indentation_style#K.26R).
	- But ALWAYS use brackets after ``if``/``else``/``for`` and the like! Too much life has been wasted on the illusion that this line is really a single expression...
	- Example:
		```cpp
		if constexpr(std::is_same_v<T, int>) {
			// ...
		}
		else {
			// ...
		}
		```
	- In particular don't ever do this atrocity: `} else {`

- Put a single space after ``if``, ``while``, ``for``, etc. Like so:
	- ```if (expression)``` 
	- ```if constexpr(expression)```
	- ```do { ... } while (expression);```

- Put a single space between the operator and each operand, e.g. ``const auto abc = 2 + 2;``

- There isn't really a fixed maximum for the line's length, but keep it around 125 characters.

- Break the lines like so
	```cpp
	void something::long_function_definition(
		const int arg1,
		const double arg2
	) const {

	}

	long_function_call(
		a,
		b
		c
	);
	
	complex_call_with_lambda(other_func(hey_there(
		a,
		[](){
			return 1337;
		},
		c
	)));

	const auto long_assignment_with_ternary = 
		abc == 2 ?
		something :
		else
	;
	
	if (long_clause
		&& other_long_clause
		&& another_long_clause
	) {

	}
	else {

	}
	```

- If the initial variable value depends on several conditions, you can sometimes employ this trick to make the variable const:
	```cpp
	const auto complex_variable_made_const = [&]() {
		if (foo) {
			if (bar) {
				return 2;
			}

			return 1;
		}

		return 0;
	}();
	```
	
	I.e. you define a lambda and call it in the same expression (note the `();` at the end of the assignment).

- When declaring new structs that are meant to be synchronized through the network or serialized to the disk, write `// GEN INTROSPECTOR struct struct_name`:
	```cpp
	struct arena_mode_ai_state {
		// GEN INTROSPECTOR struct arena_mode_ai_state
		float movement_timer_remaining = 0.0f;
		float movement_duration_secs = 0.0f;
		...
		// END GEN INTROSPECTOR
	};
	``` 
	Take care to not put function declarations between these comments, only member field definitions.

- Write comments like this, even if they are single-line:
	```cpp
	/*
		This is a very long comment.
	*/
	```
	This makes it easier to later add lines to that comment.

- Use this if-scoping mechanism for variables:
	```cpp
	if (const auto stack_target = cosm[stack_target_id]) {
		// ...
	}
	```

	This combines dereferencing the handle and checking if it is set in one go, and constrains visibility of `stack_target` only to the scope where it's needed.

- If there is a very complex if clause, prefer to name the condition with a bool for clarity:
	```cpp
	if (const bool something_happened = something && happened) {

	} 
	```

- Cache the operation results in variables aggressively near the beginning of the function, e.g.:
	```cpp
	const auto character_handle = in.cosm[character_id];

	if (character_handle) {
		const auto character_transform = character_handle.get_logic_transform().pos;
		const auto pos = character_transform.pos;

		// ...
	}
	```
	
	instead of writing

	```cpp
	in.cosm[character_id].get_logic_transform().pos
	```

	every time you need the character's position.

- Try to avoid repeating yourself. When writing a lot of code, use lambdas generously and then call them in a facade:
	```cpp
	auto complex_condition = [&]() {
		if (something) {
			return foo;
		}
		
		return bar;
	};

	auto complex_operation = [&]() {
		// ...
	};

	auto complex_else = [&]() {
		// ...
	};

	if (complex_condition()) {
		complex_operation();
	}
	else {
		complex_else();
	}
	```

- use `a.has_value()` instead of `a` when checking `std::optional` to know it's not a boolean.

- use `a != nullptr` instead of `a` when checking pointers.

- To log values, use the type-safe log functions:
	- `LOG("some value: %x", some_value)` where "%x" stands for argument of any type.
	- `LOG_NVPS(var1, var2)`

- Some of the existing code might not follow the above principles, do not edit it to make it consistent unless explicitly asked.
