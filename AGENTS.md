# General programming rules

- **Do not change formatting style like tabs or other spacing conventions in the files you are editing.**

- Include order
	- Standard libraries first
	- Then in the order of basic to specific: augs -> game -> view -> application, e.g.:
		```cpp
		#include <vector>
		#include "augs/math/vec2.h"
		#include "game/cosmos/logic_step.h"
		#include "view/viewables/viewables_loading_type.h"
		#include "application/config_json_table.h"
		```

- Basic cpp footguns
	- Do not use `using namespace std;`
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

- Break the lines like so:
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
	; // take care to always have the ; a separate line!!!

	/*
		For multi-line boolean expressions and assignments,
		put the semicolon on its own line:
	*/
	const bool complex_check =
		(some_condition && other_condition) ||
		(another_condition && yet_another)
	;
	
	if (long_clause
		&& other_long_clause
		&& another_long_clause
	) { // take care to always have these two characters in a separate line!!!

	}
	else {

	}

	/*
		Long initialization:
	*/
	pathfinding.rerouting = pathfinding_progress {
		std::move(*new_rerouting),
		0
	};
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

- Where appropriate, use this if-scoping mechanism for variables:
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

- Use `::` prefix for all global function calls that are defined in the same file but outside of any namespace. This makes it clear the function is global and not a member of any class or namespace.
	```cpp
	const auto result = ::my_global_function(arg1, arg2);
	```

# Game architecture specific

- To create new entities, allocate_new_entity_access access is required; you must declare the function you need there and write a comment that justifies why and how you're going to create new entities.

- Prefer using vec2 (float) and vec2i (int) for coordinates so e.g.
	```cpp
	uint8_t get_cell(int cell_x, int cell_y) const;
	```

	would better be written as

	```cpp
	uint8_t get_cell(vec2u cell_pos) const;
	```

- Use vec2/vec2i arithmetic operations instead of operating on x and y separately:
	```cpp
	/* Good */
	const auto offset = (pos - bound_lt) / cell_size;
	
	/* Bad */
	const auto offset = vec2i(
		(pos.x - bound_lt.x) / cell_size,
		(pos.y - bound_lt.y) / cell_size
	);
	```

- Whenever performing 2D vector math, check if there's a function in `augs/math/vec2.h`:
	```cpp
	/* Good: use vec2::length() */
	const auto dist = (a - b).length();
	
	/* Bad: manual euclidean distance calculation */
	const auto dx = a.x - b.x;
	const auto dy = a.y - b.y;
	const auto dist = std::sqrt(dx * dx + dy * dy);
	```

- You can cast between `vec2` (float) and `vec2i` (int) freely:
	```cpp
	const auto float_vec = vec2(int_vec);
	const auto int_vec = vec2i(float_vec);
	```

- Use `vec2::square(side)` for creating square vectors instead of `vec2(side, side)`:
	```cpp
	/* Good */
	const auto cell_size_vec = vec2::square(cell_size);
	
	/* Bad */
	const auto cell_size_vec = vec2(cell_size, cell_size);
	```

- Use `reverse()` wrapper from `augs/templates/reversion_wrapper.h` for reverse iteration:
	```cpp
	/* Good */
	for (const auto& item : reverse(container)) { ... }
	
	/* Bad */
	for (auto it = container.rbegin(); it != container.rend(); ++it) { ... }
	```

- Use `callback_result::CONTINUE` or `callback_result::ABORT` from `augs/enums/callback_result.h` for callbacks that can continue or abort:
	```cpp
	for_each_item([&](const auto& item) {
		if (should_stop) {
			return callback_result::ABORT;
		}
		// process item
		return callback_result::CONTINUE;
	});
	```

- When implementing new functionality in the `augs/` folder, always put it in the `augs::` namespace.

- Separate generic algorithmic logic (BFS, A*, sorting, etc.) into `augs/algorithm/` headers. Game-specific code should call these generic functions with appropriate parameters. This keeps algorithms reusable and testable independent of game code.

## Common vec2 and ltrb functions (from augs/math/vec2.h and augs/math/rects.h)

- `float vec2::length()` - get the length of a vector
- `float vec2::degrees()` - get the angle of a vector in degrees
- `float vec2::radians()` - get the angle of a vector in radians
- `vec2 vec2::normalize()` - normalize the vector
- `type vec2::dot(vec2)` - dot product
- `type vec2::cross(vec2)` - cross product
- `type vec2::area()` - area (x * y)
- `static vec2 vec2::square(side)` - create a square vector (side, side)
- `static vec2 vec2::from_degrees(deg)` - create unit vector from degrees
- `vec2 ltrb::lt()` - left-top corner (alias for left_top())
- `vec2 ltrb::rb()` - right-bottom corner (alias for right_bottom())
- `vec2 ltrb::rt()` - right-top corner (alias for right_top())
- `vec2 ltrb::lb()` - left-bottom corner (alias for left_bottom())
- `vec2 ltrb::get_center()` - center of the rectangle
- `vec2 ltrb::get_size()` - size of the rectangle (width, height)
- `bool ltrb::hover(vec2)` - check if point is inside the rectangle
- `bool ltrb::hover(ltrb)` - check if rectangles overlap
- `T ltrb::w()` - width of the rectangle
- `T ltrb::h()` - height of the rectangle
- `T ltrb::area()` - area of the rectangle (w * h)
