---
title: The outdated masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: What we've already determined won't work.
---

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
