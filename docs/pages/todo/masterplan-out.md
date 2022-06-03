---
title: The outdated masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: What we've already determined won't work.
---

with project name being de_cyberaqua

- .cache
	- de_cyberaqua.autosave
	- de_cyberaqua.history
		- we can trivially have history with a single binary file holding everything
- de_cyberaqua.arena
- de_cyberaqua.arena.json (optional, only if once exported for compat)
	- Can show inside file explorer, but instead of inspector, you have a hint what to do with this, when last saved and last exported version.
- de

	- z drugiej strony.. tych rzeczy do ignora jest malo
	- a ktos kto mapke robi moze chciec zostawic dla potomnosci jakies nieuzywane obiekty?
		- tylko ze one i tak nie beda signowane wtedy
		- bez sensu, wysylamy tylko to co autor podpisuje faktycznie

- mozemy trzymac w jsonie tak
	- path = ["gfx", "some_folder", "to", "filename", "jpg"]
		- You would have to worry about the path being too long anyways
		- we only allow _abc...7890
			- but the editor will have to enforce it somehow
				- no prob, doable
		- last is always extension and we can also check if it's one of the allowed ones for that file
		- this is also good because we don't use slashes



	- the about section I think might be separate for the purposes of quick browsing
- But why not just keep a cache with binary for loading, save there directly, and always asynchronously save to json?
	- Do we really care that much about folder structure?
	- Well, it seems less error prone, but maybe it's illusory
	- for one, somebody could close the game while it's still saving (with binary it's less probable) and the update would break the binary save
	- when we rely exclusively on binary save it's less likely to happen, still, an extreme case
	- we could even force the game not to close until the json save is written out
		- in case of autosave too so it waits even if we have unsaved changes
		- hmm, i dont know
- Zero asynchroniczności na razie (procz autosave), prostota prostota

- COMPATIBILITY/EXPORTING WORKFLOW
	- Co ważne ten export for compatibility nie powinien już pytać o lokacje
		- Po prostu F12 albo File->Export for compatibility tworzy od strzała project.arena.json
	- Nic tu nie ma trudnego, po prostu jak bedziesz chciał wrzucić folder na serwis to serwis ci zwroci komunikat gdyby nie było jsona
		- "Press F12 in your project to export json for compatibility with future versions of the game."
	- Corner cases
		- Jakby sie cos przy updatowaniu zwaliło
			- Ale to jakos atomicznie mozna zrobic moze? 
				- Czy moze to weryfikowac na koncu ze jest git? - nie kontynuowac dopoki nie 
			- Mozna jeszcze zrobic ze przesuwamy binarny project.arena do project.arena-old zamiast nadpisywac
				- albo zostawiac w jsonie
		- Jakby ktos komus przyslal mapke z inna wersja
			- Bo edytował długo i w miedzyczasie wyszedl update
			- no to jest mega unlikely raczej



- content/arenas/de_cyberaqua
- user/arenas/de_cyberaqua
- user/builder/de_cyberaqua
czy
- user/projects/de_cyberaqua

raczej nie:

- content/arenas
- community/arenas
- user/projects

- content/official/arenas
- content/community/arenas
- user/arenas

or

- content/arenas
- community/arenas
- user/arenas

- Nad angelscriptem do samego skryptowania zastanowimy się potem bo nie ma przeszkód żeby do samego skryptowania potem zrobić angelscripta zamiast lua
	- Ale myślę że lua będzie lepsza bo będzie bardziej portowalna do przyszłych wersji
	- Wiesz jj2 już się nie zmieni

- myslalem jeszcze czy angelscripta nie uzyc bo w jj2 jest uzywany i tam tez chyba bezpiecznie sandboxuja
	- on jest domyslnie sandboxed co na plus
	- ale strasznie duzo roboty by bylo z tym wiec chyba posandboxujemy z lua

- Lua pros:
	- Nie musimy pisac ani dodawac do budowania bo juz zrobione
	- Bedziemy mieli juz jeden format do skryptow i do danych
		- nieprawda bo json
	- mozna nawet skrypty w lua pisac ktore konwertuja miedzy wersjami bo moga ladowac tablice i zmieniac nazwy keyom latwo
	- cos mi sie zdaje ze bedzie szybsza od yamla
- Yaml pros:
	- Ladniejszy

- Wlasciwie to czemu nie lua? Bysmy mieli gotowe od razu
	- Ogolnie my to potrzebujemy tylko do wersjonowania i backwards compatibility
	- Teoretycznie mozna zrobic ze zawsze community mapki sa sciagane w binarnych plikach
		- mozna tez zrobic ze serwer ma wyslac tobie w odpowiedniej dla ciebie wersji binarnej
			- ale wtedy rownie dobrze mozemy nie uzywac tych tekstowych dla kompatybilnosci wstecznej
				- bo to zaklada ze mamy procedury na binarne wersje
				- a jakbysmy wysylali tekstowo to po prostu latwo w nowej wersji porobic ify na kazda wersje
				- albo nawet konwertery jakies napisac

- Ok ale yaml przekonuje mnie tym że typ jest przez kod definiowany dzieki czemu syntax jest bardziej przejrzysty (brak "")
	- unity tez uzywa wiec da sie da sie
	- i struktury beda w razie czego

- Dobra to yaml
	- Czy potrzebujemy generalnych serializatorow do yamla?
		- prawdopodobnie bedziemy pisali reczne kiedys dla ewentualnej kompatybilnosci wstecznej
	- Regardless, introspektorow i tak potrzebujemy zeby przesylac mapy binarnie

- Może TOML?
	- Dobrze byłoby już nie komplikować z jakimiś strukturami strasznymi typu light.attenuation.linear itp
		- nawet jak cos po stronie C++ trzymamy typu light.color to zapiszemy tutaj jako light_color
		- jakby nawet komendy jakieś walił typu set light_color to też można łatwiej
		- problem tylko sie pojawia jak chcemy listy jakichs struktur
			- ale pomyślmy gdzie tak naprawde tego potrzebujemy
		- tak czy siak 99% casow to bedzie wlasnie jeden level i dobrze byloby wybrac taki format ktory pod to jest zdesignowany

- Binarny plik na wszystko zawsze bedzie jeden czy to autosave czy ten obecny skompilowany.
	- Actually no because it would also have to store history!
	- .cache raczej niepotrzebny bo nie bedziemy mieli tylu tych binarek zeby to uzasadnialo osobny folder
		- aktualnie mamy wiecej binarek
