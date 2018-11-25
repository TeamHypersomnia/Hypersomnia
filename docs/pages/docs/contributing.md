---
title: Contributing
tags: [getting_started]
summary: |
    Great portion of time spent developing Hypersomnia was to make the codebase possible to be understood by newcomers.  
    Ultimately, each fan should be able to improve the game with reasonable amount of effort.

    This article will run you through the basic procedure of making changes to the Hypersomnia source code. 
permalink: contributing
---

## Introduction

If you have not yet launched Hypersomnia, [do it now]({{site.repo_path}}#how-to-build).

## Coding conventions

Please notice that some of the following conventions are not necessarily what would have been decided had the project begun yesterday.
These have been established too long ago, when the project wasn't thought of quite seriously, and the codebase has grown too huge for the style to be changed now.

### Language

- Usage of modern C++ features is required where it is applicable. Make sure that the feature you use is supported by:
    - The latest stable GCC release (at the time of this writing, 7.2).
    - MSVC that ships with the latest Visual Studio Preview.

    Worry not though, if you mess up with either, we will know thanks to AppVeyor and TravisCI integration.

- Prefer initialization with ``auto`` or ``const auto``.
    - Except if you need [default initialization](https://en.cppreference.com/w/cpp/language/default_initialization) or non-movable types as move elision does not yet fully work in MSVC.

- Put ```const``` wherever possible even at the cost of readability. 
    - In particular, ```const``` every possible function argument.
    - But don't ```const``` in the function declarations, though this must still be corrected for much of the existing code.

- The rest is left to your imagination.

### Formatting

- Use tabs for indentation.
- Use uncapitalized ```underscore_case``` everywhere.
    - But do literally what you want with template parameters.
- [Linux kernel indentation style](https://en.wikipedia.org/wiki/Indentation_style#K.26R).
    - But ALWAYS use brackets after ``if``/``else``/``for`` and the like! Too much life has been wasted on the illusion that this line is really a single expression...
    - Example:

            if constexpr(std::is_same_v<T, int>) {
            	// ...
            }
            else {
            	// ...
            }
        
- Put a single space after ``if``, ``while``, ``for``, etc. Like so:
    - ```if (expression)``` 
    - ```if constexpr(expression)```
    - ```do { ... } while (expression);```
- Put a single space between the operator and each operand, e.g. ``const auto abc = 2 + 2;``
- There isn't really a fixed maximum for the line's length, but keep it around 125 characters. If you must break the function arguments, do it like so:

        
        function(
        	a,
        	b
        	c
        );
        
        func(other_func(hey_there(
        	a,
        	[](){
        		return 1337;
        	},
        	c
        )));

        void something::foo(
			const int bar,
			const double other
		) const {

		}
        

- The rest is left to your imagination.

### Adding external code

Generally, if writing gameplay code, you should not need to introduce any other external library at this point, maybe to the exception of some crazy math.
It is however entirely possible that such a need arises. In this case:
- Make sure that the library is compatible with [AGPL-3.0](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/LICENSE.md).
- Make sure that there is no library that too suits your need while being more lightweight. [See why](core_principles#using-external-code).
- Make efforts to use/configure that library *without* modifying its source code. 
    - If you absolutely must change the source code, first request to create a fork in [TeamHypersomnia]({{ site.organization_path }}).
    - If you succeed in keeping the external library intact, simply add it as a ```git submodule``` into ```src/3rdparty```.
- You are encouraged, but not required to, add a relevant flag to [```CMakeLists.txt```](CMakeLists) that allows to exclude that library from build.

Additionally, all modifications to the code not original to the Hypersomnia repository (whether a new one or already added) shall stay in accordance with their respective conventions.

If you paste snippets from sites like [stackoverflow.com](https://stackoverflow.com), make sure to attribute them by adding a commented link above the code.
