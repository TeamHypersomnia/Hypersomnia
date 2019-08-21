---
title: Bug database
hide_sidebar: true
permalink: bug_database
summary: Notable bugs.
---

- Fix a fatal desync-causing bug: handle_new_session overwrote the crosshair_sensitivity with default values AFTER receiving the public settings update from the server.
We'll now separately handle session-channeled and solvable-channeled information.

-
[xcb] Unknown sequence number while processing reply
[xcb] Most likely this is a multi-threaded client and XInitThreads has not been called
[xcb] Aborting, sorry about that.
X connection to :0 broken (explicit kill or server shutdown).
Hypersomnia: xcb_io.c:643: _XReply: Assertion `!xcb_xlib_threads_sequence_lost' failed.
[1]    10087 abort (core dumped)  ../build/current/Hypersomnia --connect

#0  augs::window::get_window_rect() const ()
[Current thread is 1 (Thread 0x7fb0d72465c0 (LWP 10703))]
#0  augs::window::get_window_rect() const ()
#1  augs::window::get_screen_size() const ()
#2  work(int, char const* const*) ()
#3  main ()

- most probably fixed by setting cursor paused synchronously, similarly to screen size case


- The server (rightfully) kicks the bots because it sees them as lingering characters for clients who are not yet set
	- That only means that if we ever get to implement bots,

- HRTF
	- Windows was crashing when HRTF was unsupported, but only on release. Now as HRTF can find the mhr files, it does not crash
	- But you need drivers for it to work properly, or a different frequency was set by default for my headphones
	- A drop in quality even for stereo sounds could be heard if 32khz mhr file was found in content/hrtf
		- It is particularly evident for the knife wielding sound


- Client was initializing clientTime to 0.0 and had to catch up to the current time, which is why we had delays for connecting.

- Why do we have player (+player) ?
	- Nothing important, just written a wrong player as assister

- Why suicides are shown even though they had not happened?
	- Because a loss of consciousness was mistakenly counted as a knockout.

- There might be some unspecified problems if the client does not keep up with the client simulation.
	- This is because we only advance once to keep better framerate.
	- Note that the client will anyway keep up because it always simulates ALL referential frames that it receives to catch up,
		- so it makes no sense to further strain it

- HUD shows bad bomb progress
	- It was due to client setup not keeping up with current time.

## Desync issues

- Resources:
	- https://techdecoded.intel.io/resources/floating-point-reproducibility-in-intel-software-tools/
	- https://software.intel.com/en-us/articles/getting-reproducible-results-with-intel-mkl
	- https://scicomp.stackexchange.com/questions/24423/small-unpredictable-results-in-runs-of-a-deterministic-model
		- Multithreaded computations on parallel cores. Modern computers typically have 2, 4, 8, or even more processor cores that can work in parallel. If your code is using parallel threads to compute a dot product on multiple processors, then any random perturbation of the system (e.g. the user moved his mouse and one of the processor cores has to process that mouse movement before returning to the dot product) could result in a change in the order of the additions. 
		- I think this is only a problem with Intel MKL
	- https://software.intel.com/en-us/forums/intel-c-compiler/topic/518464
	- https://www.hemispheregames.com/2017/05/08/osmos-updates-and-floating-point-determinism/
	- https://stackoverflow.com/questions/20963419/cross-platform-floating-point-consistency
	- http://nicolas.brodu.net/en/programmation/streflop/index.html
		- Even then, the IEEE754 standard has some loopholes concerning NaN types. In particular, when serializing results, binary files may differ. This problem is solved by comparing the logical signification of NaNs, and not their bit patterns.
		- ALignment of data and vector instructions. Modern Intel processors have a special set of instructions that can operate on (for example) for floating point numbers at a time. These vector instructions work best if the data is aligned on 16 byte boundaries. Typically, a dot product loop would break the data up into sections of 16 bytes (4 floats at a time.) If you rerun the code a second time the data might be aligned differently with the 16 byte blocks of memory so that the additions are performed in a different order, resulting in a different answer. 
			- Fortunately for us malloc aligns to 16-byte boundaries for us
			- what about static allocations?
	- https://stackoverflow.com/questions/26497891/inconsistent-float-operation-results-between-clang-and-gcc
	- http://christian-seiler.de/projekte/fpmath/
	- use openlibm or crlibm?
	- use fp strict on linux as well, there's a switch
	- ensure that fpmodel holds every frame
		- actually we do already 
		- the guy on Box2D forum checked only that nearest mode and something else?
			- It was _MCW_PC which is unsupported on x64 per the msdn docs and intel forums here:
				- https://software.intel.com/en-us/forums/software-tuning-performance-optimization-platform-monitoring/topic/277738
	- https://stackoverflow.com/a/26502092/503776
- flags to think about
	- ffast-math
- a client receives a state at N and does a complete reinference on it
- the entropy for N+1 contains a "player added" entry which forces a complete reinference
- shikashi, the clients did not reinfer state at N which the client has received
- order of collisions happening?
	- probably not
- Could it be that the predicted cosm is advanced by mistake instead of the referential one?
- Perhaps operator= of the physics system inadvertently modifies something of the world which it copies?
	- The island only ever exists temporally.
	- Problem: the stack allocator is rewritten. There are possibly dangling addresses.
- There is a case where Data and Shuncio were synchronized with each other, but both had non-canonical version of the world, while I had it canonical
- possibly bad packet payload?
	- why would a client on the same machine not desync?
- turn off fused add and multiply
- test if replaying editor session yields same results if forcing a re-serialize of the solvable every frame
	- and if serialized bytes are identical after re-serialization cycle
	- make sure to use Release on Windows
	- we can re-serialize random number of times to avoid the need for relaunch
		- sometimes 0
- to test reinference determinism, test if completely reinferring arbitrary number of times yields equal result as reinferring always just once
- perhaps reinference inadvertently modifies something of the solvable?
	- however, everybody reinfers all the same
- doesnt the joined player reinfer twice? once on load, another time on shall reinfer
- check windows dll dependencies for determinism

## Unexplained

### there was a request to transfer or wield an item that had an invalid slot set 

 but the target container id was set to some value.
didn't you confuse an item with target container id?
item was set but to some trash value that did not exist. maybe the target slot was wholely unset after all.

### One crash for dispatch at line 272 was due to queueing a non-existent entity for deletion.

- I can't really remember if it happened right after defuse or on round restart, 
	- but most probably it was at the moment of defuse.
- It was during reprediction.
- It would try to iterate over children of the entity.
- happened once during bomb plant but unexplained ever since.
 
- Basically all queues need to be flushed upon any non-standard alteration to signi
- We need to either restart at the very beginning or clear all queues upon restart
- what if bomb theme is queued for deletion the moment that the round is also set in pre solve?
	- Assuming the crash happened after round restart, anything could have happened because significant is invalidated completely by being assigned to
- Could some deletion queues remain dangling after we re-assign the cosmos on start of round?
	- Even so, why would it happen even before that reassignment (right in the moment of defuse)
	- Would it be due to deletion of lying items on the ground?

## Others 
- fix Common state crash
	- it was due to a stack overflow.

- Strange crash and glitches on windows related to STREFLOP
	- visibility glitch was due to fpclassify malfunctioning
		- we replaced it with isfinite and it's ok
	- crash was due to usage of reproducible funcs in make_rect_points
		- Does not occur when we use LOG_NVPS on the values, so it's probably an optimization problem
		- Still, it broke some calculations when a call to get_visible_world_area was made from within particles simulation system
			- it would overwrite the delta
			- well there's lots of UB in streflop

- why logic speed mult enters 0?
	- rewrite change didn't save the new value to values_after_change

- Watch out to always properly name the introspectors for types that specify introspect_base.
	- Our history with marks would silently not get introspected because of such error.

- Make field address has first checked if the type is trivally copyable.
	- It resulted in containers being always passed, element_index having never been considered.

- Check out for always_skip_in_editor

- Strangely disappearing finishing traces problem: transform was set in post construction which did not reinfer the npo node properly.
	- Thus the traces disappeared when they were spawned far from the origin.

- When resetting the work unique_pointer inside the editor folder, the editor current access cache must be refreshed.
We were getting crashes because we were just assigning to the unique ptr, effectively making a new allocation. 
	- This was by the way completely sub-optimal.

## Crash bez includa, a z includem działa

To pokazuje jak se można odstrzelić całą nogę w tym języku.
Miałem takiego buga, że jak do jednego pliku, 
nazwijmy go ``crash.cpp`` - nie wrzuciłem jednego includa - to miałem crasha w ``serializacja.cpp``.
Co ciekawe, funkcje z ``crash.cpp`` w ogóle nie były wykorzystywane podczas testowania crasha.

Więc mam se w kodzie takie generalne templatki do serializacji, ``write_bytes`` oraz ``read_bytes``, 
które z dowolnego obiektu robią ci wektor bajtów i vice versa.
Możesz też dla swojego typu zdefiniować w global scopie własne funkcje 
zwane ``write_object_bytes``/``read_object_bytes``.

Jeśli np. ``write_bytes`` wykryje ``write_object_bytes`` tam gdzie jest wywołana, 
to ta funkcja będzie wywołana zamiast np. domyślnego ``std::memcpy`` dla trywialnie-kopiowalnych typów.

Mam sobie jakiś tam zwykły ``foo.h``. 
Obok jest ``foo.hpp``, 
a w środku moje własne implementacje ``write_object_bytes``/``read_object_bytes`` dla klasy foo.
``foo.hpp`` z założenia includuję tylko w ``serializacja.cpp``, 
żeby nie wrzucać wszędzie indziej niepotrzebnego kodu.

Wróćmy do ``crash.cpp``. 
Wywołuję tam ``write_bytes`` na foo z pewnego trywialnego powodu. 
Nie ma tam jednak wywołania do ``read_bytes``.
I teraz jeśli do ``crash.cpp`` wrzuciłem ``foo.hpp``, to gra działała, a jeśli nie, to crashowała. 
Zapomniałem go wrzucić, a kod się i tak zbudował, bo domyślny behaviour mojego serializatora też będzie działać, 
a już na pewno powinien działać bez crasha, w przeciwnym wypadku jest wywalany ``static_assert``.

Rezultat?

W ``serializacja.cpp``, poprawnie includuję ``foo.hpp`` i wywołuję ``write_bytes`` oraz ``read_bytes`` 
z widocznymi w global scopie ``write_object_bytes``/``read_object_bytes``.
ALE, wcześniejsza kompilacja pliku ``crash.cpp`` striggerowała instantiację ``write_bytes`` 
BEZ customowego ``write_object_bytes`` z pliku ``foo.hpp`` (bo zapomniałem go wrzucić).

Rezultat? 
W ``serializacja.cpp``, ``read_bytes`` dla foo skompilował się z customowym ``read_object_bytes``, 
podczas gdy ``write_bytes`` dla foo skompilował się z domyślnym zachowaniem, 
mimo że w scopie był widoczny customowy writer,
tylko dlatego że wcześniej w ``crash.cpp``, ``write_bytes`` został już zinstantiatyzowany bez ``write_object_bytes``.

To poskutkowało w asymetrycznym cyklu read/write w ``serializacja.cpp`` - i mamy crasha.
Wszystkie includy i kod w ``serializacja.cpp`` były w 100% poprawne.

Rozwiązanie jest proste, trzeba po prostu wrzucić po ludzku do ``foo.h`` forward-deklaracje 
dla ``write_object_bytes``/``read_object_bytes``,
żeby każdy compilation unit który używa foo widział zawsze że ma on customowe i/o,
i żeby mi nie zinstantiatyzował błędnie żadnego timplejta.

No ale tego nie zrobiłem.

## Mysterious crash on Windows in Release

The game would crash randomly when shooting characters, but only in Release mode,
not even in RelWithDebInfo. The link-time optimizations might have been too aggressive somewhere.
Surprisingly, a deterministic repro has been successfully recorded in the Editor.
It was most probably a bug in MSVC's standard library.
After building the Release with /Zi, a procdump-generated .dmp
has revealed the crash to originate from an _Emplace_reallocate method of the vector.
Indeed, commenting out several push_backs has removed the crash completely.

I have therefore re-written a chunk of perfectly legal and ordinary code
to do literally the same thing but to avoid using push_backs,
which in the end turned out unnecessary to achieve the goal.

The problem was inside the audiovisual state's post solve function,
and the crash would sometimes occur on pushing back the new rings to be created.

This problem remains disconcerting because who knows when else might the application randomly crash.

### Bug still persists

We might need to switch to libcxx, provide a custom vector implementation or just avoid using it wherever possible.
We can use constant size vectors more often.
