---
title: The outdated masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: What we've already determined won't work.
---

- Co signuje autor?
	- Tylko dwie rzeczy
		- project.json
		- Tekstowa lista par <ścieżka:hash> wszystkich zasobów w gfx/sfx/scripts 
			- posortowana LEKSYKOGRAFICZNIE, nie naturalnie, po ścieżce
				- tak bardziej deterministycznie
			- Skąd wzięta?
				- Z punktu widzenia edytora - zawsze będzie w pamięci ostatnio wypisany resource_hashes.bin
					- Oczywiście integrity check poleci przed podpisaniem 

- Coraz bardziej zaczynam sie sklaniac ku opcji ze normalne codzienne edytowanie to do bina jednego leci i tyle
	- Pros:
		- Less complexity z tym sprawdzaniem czy ktos nie wywalil/nie edytowal mi jakiegos jsona itp
		- Szybciej sie zapisuje, mniejsza frykcja dla tworcy do stworzenia czegos prostego
		- Mniejszy clutter w fs
		- Cały folder razu gotowy do wyslania przez siec bez tańcowania z ignorami.
			- Ignorujesz tylko folder .cache.
	- Cons:
		- Podwójny styl zapisywania
			- Bo areny w officialu będą zawsze jsonach a tu edytujemy na binach
		- Nie da sie natychmiastowo przekopiować png z całymi proptami do innego projektu
			- Ale to nie problem, zawsze możemy w projekcie walnąć exporta
			- Latwiejszym kopiowaniem mozemy zajac sie pozniej
	- Uwaga: tak samo oficjalne mapy możemy shipować binarne, czemu nie? Wystarczy w CI przeleciec wszystkie z jakas flaga --compile.
	- Czyli z tym approachem w praktyce trzeba tylko ogarnac u siebie jakis workflow z compatibility mode
		- Bo graczowi z automatu bedzie rekompilowało przy update
			- exported json leci do cache ale to nawet jeden plik
		- U nas moze wykrywac ze wczytanie pliku binarnego sie nie powiodlo w jakims catchu
			- I wtedy dialog pyta czy wczytac z ostatniego wyeksportowanego jsona
			- Compatibility mode moze z automatu wskakiwac jak edytujemy official arene (i mozemy to zrobic tyko jak nie jestesmy w prodzie)
	- Na jakichs serwisach z mapami ludzie mogą chciec hostować mapy w compatibility mode do kompilacji


- to uzasadniałem że lepiej optymalizować na to co będziemy robić 99% czasu a nie na 1% corner casów, że niby zapisywanie będzie szybsze
	- Gdy używamy jednego bina i 0 jsonów to cały workflow z edytowaniem i przesyłaniem jest szybszy i łatwiejszy dla twórcy
	- A więc COMPATIBILITY MODE
	- Tylko jak bardzo error prone bedzie recompile przy upgradach?
		- Musimy zdekompilowac wszystko jak wykryjemy nowa wersje i powrzucac do cache
		- "Exporting user and community arenas for the next version..."
- Ale nie: tu akurat bardziej wazna jest super intuicyjna prostota niz jakas turbo wydajnosc bo mapki na start beda bardzo male

- za to sam autosave mozna zapisywac w .lz4 bezprzypalowo
	- ale przedmaturalna optymalizacja

		- i zobaczy zresztą wtedy że sygnatura byłaby nie tak bo najpierw i tak jest zapisywany resource_hashes a potem dopiero robiony podpis, więc sygnatura by się najpierw nie zgadzała
			- "Recover lost changes from project.autosave|tmp.json?
				[Yes] [Discard]"
			- Nie promptujmy, chcemy zeby dalo sie zinspektowac jakie te zmiany byly
			- Zatrzymac autosave gdy prompt sie pyta!!! Najlepiej nie otwierac projektu zanim wybor zostanie dokonany
			  To revert back to the last saved version."

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
- user/editor/de_cyberaqua
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

## Organizacja plikow w projekcie

- Jeszcze co do rzeczy typu description i credits to mozemy miec takie podejscie ze sa oddzielne pliki na to i ze to tez jako pliki sie w samym edytorze otwiera
	- Zamiast w jakims menu z gory
	- Jeden osobny plik: project.about.json
		- Credits/description do szybkiego wczytania dla exploratora
	- Wiadomo ze te specjalne pliki undeletable

## Rozkminy wczesniejsze

- Przesył mapy od serwera do klienta bedzie BINARNY w calosci.
	- "skompilowane" i jeszcze przeleciane lz4 bo wtedy i mniejsze beda
	- mozemy zalozyc ze zawsze wszyscy beda aktualizowali do najnowszej wersji chcac grac
		- i serwer zawsze bedzie wysylal binarna najnowsza wersje a skad on sobie wzial wczesniejsza ze byl w stanie przekonwertowac do obecnej to juznie ma znaczenia
	- i tak zakladamy ze ABI identykos jest miedzy klientem a serwerem (albo nawet ze wersja ta sama)
		- wiec zakladamy ze zawsze binarny format mapy bedzie ten sam
			- musi byc przeciez dla determinizmu symulacji
		- wiec wtedy to jest normalna konwersja w te i z powrotem zawsze
			- wiadomo ze po sciagnieciu od razu "odpakowywujemy" do kompatybilnej wersji
			- i tak samo oficjalne mapki trzymamy wylacznie w tej kompatybilnej wersji
			- i tez wtedy nie musisz wysylac jsonow per kazdy png przez siec
		- od razu mozna porobic ze te abouty to sa z constant size stringami
- Nie: wysyłamy jsony. Less error-prone

- BINARY/JSON MAP REPRESENTATION AND LIFETIMES
	- No wiec teraz nas gryzie najbardziej ze jakbysmy wysylali przez siec mape..
		- ..to musimy jakies tańce z ignorowaniem plikow (tu zamiast jsona wyslij bin, tutaj tych jsonow w ogole)
		- zamiast po prostu wysylac jeden folder od strzala
		- tylko czy to jest az takie zle?
	- Jedno jest pewne: przez siec chcemy wysylac binarna wersje bo jsonowa bedzie o rzad wielkosci wieksza
		- nie ma czasu tego konwertowac zreszta za duzy load na serwer by byl
	
	- project.autosave/project.unsaved
	- project.compiled (zeby ladniej wygladalo podczas przesylania do moze de_cyberaqua.project albo de_cyberaqua.arena - potem de_cyberaqua.arena.json)
	- a wiec project.arena
		- od razu intercosm?
			- no niekoniecznie, bo jak disk io jest botleneckiem to intercosm moze byc wolniejszy od tej wersji pre. 
			- nie nie to niemoze byc intercosm bo jak przesylamy przez siec to chcemy po odbiorze miec mozliwosc dekompilacji tego.
				- zeby user tez mogl edytowac przeciez.
			- To po prostu ta network-ready wersja do przesylu.
	- project.history?
	- Folder .cache? Byc moze. Mamy dwie binarki wiec uzasadnione
	- tylko w sumie po co osobny projekt.bin (.compiled)? Czy nie mozemy miec tylko project.autosave?
		- i nawet miec tam historii i wszystko?
			- nie bo chcemy miec gotowy do wczytania jeden plik na potrzeby samej gry i do przesylu przez siec bez zbednych formalnosci
		- zatem .cache i wiecej niz jedna binarka
	- Najbardziej intuicyjnie --chyba-- jakby był ten folder .cache i nawet przez siec juz przesylamy same te jsony
	- kmina #1:
		- Na początku nie ma nic
		- 
	- kmina #2: konwertowac wszystko do jsonow itp ale tylko przy updejcie tak ze to nigdy standalone nie istnieje
		- i jak ktos chce mapy hostowac w jakichs workshopach to trudno niech postuje binarki zawsze aktualne

- When do we store jsons? When do we recompile maps?
	- In the beginning there was nothing
	- We'll always decompile into json and the editor will keep json files all the time
	- Probably all community/custom maps upon updating the game?
		- Then show all errors if any

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

- Does the same binary hold all the data for which there hasn't happened a writeout?
	- No, a compiled arena file is just that. Last saved arena.
	- The unsaved changes file = autosave file.
