---
title: Bug database
hide_sidebar: true
permalink: bug_database
summary: Notable bugs.
---

- There might be some unspecified problems if the client does not keep up with the client simulation.
	- This is because we only advance once to keep better framerate.
	- Note that the client will anyway keep up because it always simulates ALL referential frames that it receives to catch up,
		- so it makes no sense to further strain it

- HUD shows bad bomb progress
	- It was due to client setup not keeping up with current time.

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
