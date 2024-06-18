---
title: Bug database
hide_sidebar: true
permalink: bug_database
summary: Notable bugs.
---

- Dangling clients
	- Doesn't react to afk/network timeouts - likely downloading maps?
	- When pressing Scramble teams, it automoves them to spectators again?
	- Even with afk limits set to massive
	- Was because of clients crashed while downloading, the download disabled afk and all network timeouts
	- we don't even need to prevent timeout kicks during download because
		- a) we're sending keepalives every 0.5s if it's external
		- b) every chunk request resets the last payload time


- Known browser settings issues when web build doesnt work
	- Disabled WebRTC
	- Disabled hardware acceleration - Low performance
	- Chrome has ANGLE backend set to D3D9

- web port for some reason downloads vrt3 and hyperactive over UDP, is this a file size issue? maybe some http request gets rejected
	- One US server actually had broken external arenas provider link - it was empty

- vrt 3 -> hyperactive ensure vars != null crash
	- get_scoreboard_caption was trying to get a caption when no mode was in existence
	- This points us to other probable instances of this error
	- some view logic might be trying to get the mode information even when none exists

- some random connectivity problem
	- restarting servers helped
	- we had empty space under "webrtc state", as if there was no message set yet?
- SOLUTION: we didn't clear the websocket from active_connections map if the webrtc id was not yet assigned.

- bug: fake "DataChannel closed" kick message.
    - ok i know what happens
        - "kick" CANNOT be called inside receive packet override!!!
            - the message could be queued to an already freed client id if an actual yojimbo disconnect packet arrived right before it!!

    - Server doesnt even kick the player, it looks like there are some pending goodbye packets that get sent while we reconnect?
    - maybe we should timestamp those kick messages
        - no, lets fix this properly
    - HAPPENS EVEN ON NATIVE CLIENT CONNECTING TO A SERVER
        - HOW THE HELL does a packet meant for a webrtc client and up sent to a native client?!!!
    - wouldn't execute_async prolong the existence of dc even long after its disconnected?
        - no we're only passing packets vector

- major: external ids are wrongly reused on webrtc server
	- peer disconnects right away after establishing connection and someone else takes the same id
	- so offers end up being directed to the same peerconnection object
	- we should remove the mapping as soon as connection is established
	- or simply generate secure guids for clients connections - will be even safer as nobody will be able spoof offers
		- but still remove that mapping as there's no point keeping it/accepting new offers to the peer connection
- Maybe this is why we had problems with connectivity earlier:
- web client couldnt reconnect when there was a problem with transporting signalling messages due to a bug in signalling server
    - but this could happen due to other reasons and we'd like to be able to reconnect successfully
    - set some DC/PC timeouts on the server when we fail to connect the peer via webrtc

- latent bug: new client can take slot of a freshly disconnected client and a character will be orphaned forever
	- can amortize by assigning consecutive client indices even when server is empty
		- check how it works now

- Space station crash on web 
	- Had to decrease texture sizes. Maybe we should have some scaling fallback
	- Or just increase memory 

- server likely shouldn't stop when changing tab
    - do workers still run?
    - we're out of luck because packets will still not be sent so it will still stall/timeout

- why the f is there nat traversal when connecting to 127.0.0.1?
    - it works like this with a local masterserver - 127.0.0.1 is reported as external ip and is subsequently found on the server list as an entry when connecting.
	- Fixed by disabling for internal addresses

- test what happens when server list is empty
    - works

- fix memory leaks when closing webrtc connections (both client and server)
    - done: using shared_ptr/weak_ptr, destructing and all important ops executed in the main thread

- Nasty interpolation glitch when connecting -
	- Referential cosmos needs to be snapped as well because we begin from spectator view which shows referential cosmos by default.
	- the only "bug" left is when you go spectate during warmup since your player disappears and you go back to spectating at origin (0, 0) pos

- high dpi problems?
	- https://thecharliecrisp.medium.com/supporting-high-dpi-devices-with-webassembly-emscripten-and-glfw-26c3d2934ce5
	- used clientWidth rounded, works fine

- Discord streaming issue + Pepsik bug (broken textures when launching with Steam overlay0
	- Quoting 99de1cc851d33915eacb3b6166397008d296fad2:
		settable_as_current_mixin would skip calling glBindTexture if the
		texture was already bound, even if it was a while ago - it appears that
		for GL to work properly, we must re-bind the texture right before using
		calling glTexImage2D - perhaps it's because of glActiveTexture or glBindBuffer calls in between.
		Importantly, it's no longer an issue if we just always
		bind the texture/fbo/shader etc., and there is no difference in
		framerate whether we skip re-binding or not.

- (Windows) Print screen in fullscreen works if we disable double-buffering.
	- This may be the reason why it works in some other games, maybe they have doublebuffering disabled?

- audit properly what happens upon death
	- it's consistently the only time the clients crash
	- sprawdzic na nagraniu czy to sie dzieje na spekcie czy od razu
		- chodz moze byc trudno stwierdzic bo nie od razu sie disconectuje
		- mozna sprawdzic jaki jest limit czasowy tez
- SOLUTION: Again, we were dividing by zero in health color calculation, this time in sentience_component.cpp (previous was in draw_sentiences_hud.cpp)
	- Interestingly SIGFPE was thrown even though we were dividing by a floating-point zero
		- but maybe there were some optimizations in play, who knows

- Why the f... is the cursor still on when in the game
	- active flag wasn't initialized to true 

- The delay on debug is probably because HRTF tries to catch up when the tick is sent
	- shouldn't be a problem on release

- Demo replay issues
	- Something screwed with global time?
	- Fix haste blinking
	- Fix explosives hud invisible on demo
	- The problem was with interpolation ratio.

- Fatal bug: a mutable has_been_drawn could be written whilst GUI
was processed, resulting in a potentially different cosmos being chosen
for viewing, mid-way.

This would cause the should_draw_gui check to pass whereas a dead
character would later be chosen as a subject, causing a crash.
	- **With regards to:**
		- We should really review spectator thoroughly, especially with regards to what happens during disconnects or hard re-predictions

		- Rare crashes on disconnects of other people
			- Maybe some untimely payload comes in after removed_player is handled?
				- and we assume that the player still exists within the scene?
			- untimely payloads aren't just limited to avatars, we still have pings for example
			- spectator fucks up?


- Strange performance problem when paused in editor?
	- We had Unity and OBS turned on...

- The inventory bug with atlas being shown in place of a backpack was due to there being an old Brown backpack sprite on fy_minilab


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
	- A drop in quality even for stereo sounds could be heard if 32khz mhr file was found in detail/hrtf
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
