---
title: The masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: We need to set our priorities straight.
---

- Research if godot/unity/ue have *separate* notions of "enabled" *and/or* "visible" node (or layer) or if it's only for visibility

struct pool_tuple
- One for resources and one for instances

# Resources and Nodes (instances)

- Do we need e.g. editor_sprite and editor_sprite_instance separately?
	- I think all nodes are by default understood as instances
	- And the "flavors" here are just resources
	- So we wouldn't have editor_prefab_id or editor_sprite_id, but just editor_resource_id
		- dynamically dispatched

- Also it's okay to have node ids in prefab resouces
	- Because not all nodes have to be explicitly on scene in layers
	- So prefabs in prefab resources would be prefab nodes too

- We're naming it editor_sprite_node and editor_sprite_resource

- editor_light_resource?
	- We could make using editor_light_resource = editor_light_node
	- Still, some things might be invariant like the resource id in a sprite
		- That won't even be stored in the node anyway
			- Really though? I think we should store at least the generic resource id
			- For others it might be a typed resource id
	- I think for simplciity, we'll have typed resource ids within each node type
		- I don't think an editor_node_sprite can have any different resource than sprite
	- So, do we manually specify in the nodes what is able to be overwritten?
		- One is for sure: the node will have properties that the resource doesnt have
			- pos, res id, etc
		- At the same time, the node won't have all resource properties per-instance
			- E.g. it would be stupid to give a different sprite id to a wandering pixels instance
				- (a wandering pixels resource will have an id of the sprite resource)
				- let's keep it named wandering_pixels because it's the most common usecase
		
- Question is, do we want instances to be able to override everything in the resource?
	- Pro editors do it of course
	- I think it was one of our pain points too
	- But how does a struct remember what to override?
		- Won't we have to basically introspect to get the new type?
			- We'll have to introspect or manually do it regardless if we specify it manually or just e.g. inherit
	- And the node does need a position so it's not necessarily the same struct
	- To not std::optionalize the node, we could say that each prop has e.g. a numerical invalid default like -infinity
		- Remember though that if it's something like 0 or -1 then it will default to the resource's value
			- Not a problem, we'll just keep good ranges and even if it happens it's not so bad

- resources will have ids to other resources, this will be 
	- but nodes will always have only one resource id (even prefab nodes and actual parts of prefabs)

- Note a prefab can be scripted or just a hierarchy of nodes
- Let's create a separate scripted_prefab which we'll firstly use for aquarium

- Ultimately, let's avoid implementing override behavior for now.
	- 1) for the sake of simplicity
	- 2) This won't break any state so we can easily introduce it later
	- 3) If you feel like you need an override somewhere, then your structure might need improvement
		- E.g. for light we can have just scale_light_strength instead of a whole new set of attenuations
		- There's also no problem to even have differently named "default_color" and "colorize"
	- This ultimately means we're doing things manually too, no inheritance/introspection

# Rethinking if we want state duality

Ultimately, we definitely want duality. See masterplan-detail.
- For now we can rebuild literally whole cosmos every time we make any change at all, we'll see later
- Nawet takie durnoty jak transform

# General guidelines

- Optymalizować na to co będziemy robić 99% czasu a nie na 1% corner casów
	- przy czym na razie ważniejsze jest UI i prostota architektury niż wydajność (za mała skala)

- Najważniejsze żeby mapa była jak najwcześniej już w dobym przenosnym formacie na przyszłość.
	- Potem mozna stopniowo ulepszac wydajność tam gdzie jest potrzeba
	- Więc PoC nie musi byc turboresponsywny na kazdym kroku

# Format danych

- I/O SCENY CAŁKOWICIE DO JSONA.
	- Póki trwa zapisywanie, w interfejsie nie da się nic zrobic.
		- To i tak powinien być ułamek sekundy przy tej skali.
	- Tiled tez ma taki workflow że przy zapisywaniu z automatu reeksportuje do ostatnio exportowanej lokacji
- Jedyne co będzie binarne to skompresowane wersje jsonów, w folderach .cache, dla przesyłania przez sieć.
- Nie bawimy się w binarki bezpośrednie na razie, nie ta skala. A bardziej oczywiste jest zachowywanie kompatybilności.

## Projekt w pamięci

### Layery

- Jak przetrzymujemy info o layerach?
	- Tak jeszcze żeby rozsądnie było przy reorderowaniu ich?

- Pointery do elementów?
- Jeśli chcemy trzymać binarki dla szybkiego i trywialnego wczytywania, być może lepiej będzie mieć poole i idy zamiast pointerów
- W przypadku starego edytora to była krótka piła bo wszystko poidowane było
- Na pewno nie chcemy zmieniać tego jak to jest serializowane do jsona
	- w sensie definicje nie lecą do layerów

- A binarnie 'być może' ma sens trzymać wektory identyfikatorów w każdym layerze

- To w ogóle będą będą generyczne obiekty więc na pewno nie będzie ich kolejności determinowała jedna tablica
- Jakbyś trzymał je w osobnych heterogenicznych tablicach to nie zakodujesz kolejności między dwoma różnymi
	- A to jest coś co intuicyjnie autor może sobie chcieć przekładać
- Dlatego ostatecznie każda warstwa musi mieć wektor generycznych idów
Ja myslę że tu lajtowo można przywalić taką tuplę poolów
I co i potem jaka kolejność serializacji np do jsona?
	- For each layer for each id?
	- potem na każdym idzie robisz elegancko dispatcha
Nic innego nie wymyślimy chyba, no po prostu musisz mieć identyfikatory jeśli chcesz kodować kolejność obiektów o różnym typie
- Czyli mimo że to jest nazwane "cache" to to są kesze jakby w **stosunku do jsona** a nie do binarnej reprezentacji
	- Cała reprezentacja binarna razem z poolami to można interpretować że do jest cache w stosunku do jsona
- Tylko że tego nie nazywałbym cache do końca bo jednak to *definiuje* order który będzie zapisany

Można zrobić że przy r/w jest pomijany obiekt cache
Można zrobić is base of, żeby nie stringowo to robić tylko w czasie kompilacji
json_ignored
Przywal konstexpr boola po prostu zeby klasy nie robic niepotrzebnie

Ok czyli byśmy w warstwach mieli obiekt cache żeby nie leciał do jsona

Obiekt może trzymać nazwe layera do którego sie znajduje to binarnie od razu wykorzystamy i bedziemy mieli fajny lookup
A bedzie od razu latwiejszy do serializacji do jsona

Czyli normalnie robimy templatki pooli jak dla kosmosu
Bo to jest tak naprawdę taka portable wersja kosmosu, mini opisowa

Damn tylko żebyśmy nie musieli handli calych robić znowu
Ale tu nie bedzie tyle interakcji chyba
Bedziemy dispatchowali na miejscu
editor_project on_node bedzie mial jakis i jazda
	- będziemy go passować wszędzie

I teraz można keszować sorting order dla każdego obiektu po każdej zamianie kolejności
Dzięki temu potem instantiatyzacja świata jest szybsza bo można po każdym typie na każdym poolu instantiatyzować
zamiast lecieć i dispatchować wszystko po kolei layerami
choć i to i to robi dispatch raz

## Pseudo-ids

- No jednak uważam że mega manualnie będziemy to robić tylko wiadomo do ostatecznych obiektów już read_json
	- jakieś idy nieidy i wg tego będziemy też zmieniali typ

- Musimy jakoś identyfikować prefaby w środku samego project.json
	- Na razie możemy to robić normalnie inkrementalnymi idami
	- Ale żeby było przyszłościowo, zrobimy to na stringu zamiast na incie
		- Bo potem być może byśmy chcieli używać pseudoidentyfikatorów od filenamu żeby było human readable
- Pamiętaj że to jest problem wyizolowany dla serializacji, równie dobrze moglibyśmy tam walić ścieżki pełne
- W środku w pamięci i tak będziemy trzymali albo pointer albo pool id

- Tylko jak to serializujemy? Czy trzymamy np. prywatny std::string id który jest wyłapywany przez serializacje, a pointer nie?
	- I potem wywołujemy resolva na wszystkich pointerach?

- Subresource idy też możemy tu mieć podobnie jak w godocie
	- Dla jakichś prefabów spawnpointów

- Nie jestem w ogóle pewien czy my chcemy trzymać ten id w tym samym strukcie co dane instancji
	- Raczej nie
- Czy chcemy architekturalnie zrobić faktycznie też że png jest prefabem czy to ma być tylko iluzja dla użytkownika?
	- A tak naprawdę bedziemy mieli różne typy instancji tylko pole z idem?
- To może być coś rodzaju
"instance_of": "22"
"properties:" {
}

- Żeby mniejsza indentacja była i mniej noisa to jednak najlepiej tak


- Id będziemy nadawać tylko obiektom które są gdzieś referowane
- Typ będzie inferowany już z definicji tego ida

Myślę że jakieś metadane może oddzielnie? Czy nie?
Pasowałoby wtedy jakoś to czytać manualnie
	Tak będzie chyba najlepiej
	Wywoływać po prostu read_json na wszystkich takich

# Interface

- Tam gdzie inne edytory mają Scene hierarchy my będziemy mieli po prostu Layers!
	- To będzie ten główny panel z lewej i dzięki temu będzie zawzse miał swoje miesjce na ekranie
	- On ważny jest przecież

- Można na start zrobić samą taką listę prostą jak w godocie zamiast filesystem docka z miniaturkami
	- Niekoniecznie nawet ludziom to się może podobać
	- A na start takie prostsze
	- **I mniej miejsca zajmuje na ekranie!** Dzięki temu mamy zwolnioną całą wertykalną przestrzeń która przewaznie i tak jest ograniczona już
	- I tak chcemy ten wertykalny zrobić
		- Choćby do samych folderów więc od razu można wrzucić pozycje z plikami (najwyżej do przekonfigurowania to będzie)
	- Do hierarchii z layerami będzie dokładnie to samo potrzebne

## UI zapisywania i odzyskiwania, ogólnie persystencji

- Promptujemy o zapisanie przy wyjsciu (inaczej ludzi bedzie zaskakiwalo)
	- Save before closing?
[Save] [Discard] [Cancel]
- Edytor panuje tylko nad jednym plikiem. Jeden save zapisuje wszystkie zmiany do sceny i wszystkich prefabów itp.


# Organizacja plików projektu

## Zapisywanie

- Możemy od razu writeoutować zawsze te hashe które trzymamy w pamięci, dla prędkości 99% czasu
	- A potem dopiero sprawdzać czy się coś nie zmieniło podczas działania apki
		- praktycznie niemożliwe bo by musiał jakiś proces w tle zmienić żeby nie wyłapało podczas aktywacji

## Konwencja nazw folderów i plików

Tylko alfanumeryczne z _
- Nie tylko ze wzgledu bezpieczenstwa ale portowalności na inne systemy 
- Maksymalna ścieżka: 256
- To wszystko będzie sprawdzane przez edytor i wywalało błąd podczas edytowania
	- Please rename and/or move the following files:

## Root

- .cache (\_)
	- project.json.lz4
	- project.json.lz4.stamp
- gfx (^)
- sfx (^)
- scripts (^)
- project.json
	- bez project.about.json osobno; to info będzie w środku na samym samym początku
	- i tak musimy mieć osobno prefabs = {} i nodes = {} więc about = {} nie wprowadzi nam dodatkowej indentacji
- project.old.json
- project.tmp.json (\*)
- project.autosave.json (\*)
- project.signature (\_)

(\*) - istniejace tylko podczas działania aplikacji; poza tym niewidoczne dla uzytkownika
(\_) - zawsze bezpieczne do wywalenia bez utraty danych
	- caveat: project.signature może być bezstratnie wywalony u twórcy, ale nie u tego kto ściągnął
		- potem by nie bylo jak zaktualizowac
(^)  - pliki dostarczane przez użytkownika; edytor nic tam nie zapisuje

Note: tu już dla prostoty nie zapisujemy historii.

## Role plików

- gfx, sfx, scripts na start puste, wygenerowane automatycznie, to tylko dla użytkownika
- project.json - plik sceny

### lz4

- .cache 
	- project.json.lz4 - Silnie rekompresowane przez serwer na kazdy start
	- project.json.lz4.stamp - timestamp pliku project.json na ktorym project.json.lz4 byl wygenerowany

- Server na start kompiluje wszystkie mapy do cache/project.9.lz4
	- Synchronicznie zeby byc gotowym od razu
		- chyba ze integrowany server to jakis async job z gui i dopiero jak sie skonczy to odpalamy server
	- Na startupie komunikat: Compressing NON-OFFICIAL maps for transmission...
		- bo zarowno user jak i community
		- oficjalnych nie bedzie musial bo wszyscy maja i nie trzeba ich przesylac
	- A skad wie ze trzeba przekompresowac? stampy tak samo jak w cache/ to dziala klasycznie

- Przy przesyłaniu mapy serwer->klient bedziemy rozstrzygali na podstawie opcoda typu pliku czy cos idzie skompresowane w lz4
	- I to nawet bez wzgledu na to czy binaryzujemy mape przed wyslaniem
	- Wiec to jakby czesc protokolu

### project.signature - signing & verification

- Co signuje autor?
	- Tylko project.json
		- Są już hashe

#### Proces signowania przez edytor

- Kiedy i jak wywołujemy signing?

Dwa przypadki:

- a) PO ZAPISANIU project.json. 
	- Asynchronicznie żeby użytkownik mógł już robić rzeczy, pasek na dole móże wyświetlać tylko status.
	- Jedyne co to będzie powstrzymywało kolejny zapis aż to się dokończy, bo nie ma jak zinterruptować keygena.
- b) NA AKTYWACJĘ OKIENKA po wykryciu zmian w hashach po przeliczeniu resource_hashes.
	- I ten policzony na nowo można od razu wykorzystać
	- To wstrzyma też ctrl+s na moment.
	- Wtedy nie musimy pokazywać gwiazdeczki po aktywacji okienka (jak zrobimy to dobrze)
		- bo i tak bedzie wygenerowane z ostatniego zapisanego do dysku

Sygnatura ma być tylko w stosunku do ostatnio zapisanej wersji. Autosave tutaj nie będzie nic wywoływał ani grał roli.

Procedura signowania:

- 1) standardowe ponowne obliczenie resource_hashes.bin, DOKŁADNIE tak jak na aktywację okienka.
	- Iterujemy faktyczne ścieżki w filesystemie (nie te co istnieją w resource_hashes.bin)
		- Sprawdzamy czy istnieje wpis w ostatnio wygenerowanym resource_hashes.bin w pamięci
		- Jeśli tak, i się zgadza timestamp, pomijamy hashowanie; bierzemy ten obliczony
			- I tutaj omijamy ponownego hashowanie wszystkiego żeby nie tracić czasu. Dlatego że to się dzieje na każdy ctrl+s.
				- Ponownie hashować będziemy tylko project.json
		- Jeśli nie to liczymy hash jeszcze raz (mało prawdopodobne bo przy aktywacji okienka to robimy), nic straconego.
- 2) Nowy wygenerowany resource_hashes.bin i tak jest writeoutowany (po prostu nie sprawdzamy czy jest taki sam).
- 3) Liczymy tekstową posortowaną listę ścieżek i hashy.
	- Bierzemy content binarny (std::map) dokładnie ten który poszedł do pliku resource_hashes.bin
	- Czytamy z niego tylko te dwie potrzebne dane, porzucając timestamp
	- Czyli robimy dokładnie to samo co byśmy liczyli przy weryfikacji sygnatury - to ważne
- 4) sign
	- Wczytujemy z dysku project.json
		- Żeby nie być zależnym od tego co mamy niezapisane jeszcze (w przypadku gdy signing został zainicjowany przez aktywacje okienka a nie ctrl+s)
		- a tego co last poszło do jsona na dysk i tak nie będziemy mieli w pamięci wtedy
			- Ale imo to byłby dobry integrity check
	- Dla signa wywołanego aktywacją możemy też trzymać ostatnio zapisany do dysku json w pamięci, to nie jest problem
		- Ale 99% przypadków to będzie writeout dla którego i tak będziemy od razu to co teraz wygenerowaliśmy izapisaliśmy
	- Konkatenacja project.json + ściezek
	- Możemy od razu to przemielic przez blake3 i ten rezultat dac do ssh-keygena zeby bylo szybciej i przez stdin od razu
		- Nawet nie musimy tu być zależni od ssh-keygena (wolno sie bedzie na windowsie odpalać)
		- tweetnacl + blake3 i będzie bardzo szybko działało
- 5) writeout project.signature

Zauważmy że najpierw jest writeoutowany resource_hashes.bin a potem dopiero sygnatura.
	- W przeciwnym wypadku jeśli sygnatura by się dobrze zapisała a resource_hashes niezbyt, to...?
	- W sumie nie wiem dlaczego na razie to jest ważne, ale może jest

### resource_hashes.bin

- Jednak razem z jsonem?
	- To jednak robi sens że triggerujemy potrzebę zapisania gdy zmienią się zasoby
		- Dlatego że wtedy wszystko podpisujemy jeszcze raz
	- Co z integralnością historii wtedy jak wszystko jest razem?
		- Teoretycznie nic nie powinno sie popierdzielić i powinno się dać dalej undować jeśli:
			- W całej reszcie pliku identyfikujemy zasoby po ścieżkach a nie hashach
				- Tylko wtedy co jak przesuniemy zasoby?
				- Ogólnie w takim razie w undoredo musimy trzymać jakieś identyfikatory typu nawet pointer do prefaba/id prefaba
				- i tak musimy jakoś identyfikować te prefaby
					- Hm to chyba właśnie po ścieżkach
					- Właśnie i to jest pytanie
					- Repeating paths for each and every object seems redundant but maybe it's for the best?
						- We force those to be short anyway
		
- Struktura
	- std::map<augs::path_type, entry>
		- Będzie przy okazji zapisane od razu posortowane, teoretycznie możnaby to wektorem potem wczytać
- Jeden wpis:
	- Ścieżka do pliku (klucz)
	- Wartość:
		- hash pliku blake3
		- timestamp pliku gdy był ostatnio shashowany

- Dlaczego trzymamy to w pliku, kto z tego korzysta?
	- Edytor na starcie wczytuje i dzięki temu może sprawdzić czy pliki nie zostały przesunięte
	- Serwer na starcie nie musi hashować wszystkiego od nowa
		- Serwer po prostu tak samo liczy nowego resource_hashes.bin w pamięci tak jak edytor przy aktywacji okienka
			- Tylko korzysta z hashy w tym pliku ktore sa z timestampami więc ma dowód że są aktualne, żeby nie musiał hashować od nowa
		- Gdyby ktoś nie zdążył zapisać dobrze to serwer zawsze może trywialnie sprawdzić integralność wszystkich hashy bo będą timestampy
		- I jak policzy resource_hashes to normalnie liczy sygnature z tego i sprawdza ją tak jak edytor przy tworzeniu jej
	- Teoretycznie serwer by mógł sobie przehashować po swojemu od nowa - dlatego to tylko w .cache jest - ale można od razu to wykorzystać

- .cache
	- resource_hashes.bin

- to mapa hashy WSZYSTKICH zasobów w projekcie (gfx/, sfx/, scripts/) posortowana - leksykograficznie, nie naturalnie!
	- leksykograficzna dlatego ze to nie ma byc dla czlowieka tylko dla komputera zeby deterministycznie policzył hash
- Binarna bo tu layout się nigdy nie zmieni, bez const size vectorów bo to nie leci przez sieć (nie bezpośrednio)

### Autosave

- Co 2 minuty i co deaktywację okienka
- Asynchronicznie
- project.autosave.json
	- Martwimy się autosavem tylko jednego pliku: project.json!
		- Reszta - jak widać z tabelki w ## Organizacja plików projektu - jest albo
			- a) Odzyskiwalna przez edytor po usunięciu (.cache, resources.json, sygnatura)
			- b) Poza odpowiedzialnością edytora (gfx, sfx, scripts)

- Tu już chyba nie ma więcej do kminienia

### Odzyskiwanie - proces

Źródla odzyskiwania (w kolejności aktualności, a zatem i w kolejności próbowania).
Przy wczytywaniu projektu próbujemy:

1) project.tmp.json - nowy pisany plik
	- zawsze będzie aktualniejszy od autosave, bo jeśli istnieje, to znaczy że nas wywaliło zaraz przed końcem zapisywania 
		- Choć to mało prawodpodobne
2) project.autosave
	- 99% przypadków crasha
3) jeśli istnieje project.json to tu sie zatrzymujemy i nic nie odzyskujemy
4) project.old.json - plik zapisany przed poprzednim

- 1) i 2) istnieją tylko podczas dzialania aplikacji (czyli jak po zamknieciu widac to znaczy ze cos sie zjebalo)
- 2) i 4) dwa wbrew pozorom powinny być oddzielne
	- Jeśli autosave jest nieaktualny i wywali nas przy pisaniu do niego nowego pliku, to tracimy wszystkie niezapisane zmiany

#### Interfejs po odzyskaniu

- Jeśli którykolwiek z 1) 2) istnieje to nakładamy go jako nową zmianę
	- Ale chcemy zeby dalo sie zinspektowac to co zostalo zrecoverowane wiec najlepiej tak:
		- Message box z okejka
		- "Automatically recovered lost changes from project.autosave.json.
			You can cancel the recovery by pressing Undo."

- Jeśli 4) to znaczy ze nie ma 3), wiec jest jest tylko jedna wersja bez tej nowej odzyskanej
	 - dajemy komunikat ze couldnt load project.json, loaded previously saved version instead.

- Jakas tez dopiska typu "Save your project as soon as possible!"

- Ten autosave jest bezpieczny tak naprawde dopoki nie cofniesz zmiany i nie zostawisz tego przez jakis czas
	- Ale szczerze mowiac po co autosavowac ostatni znany zapisany stan?
	- Nie autosavujmy jak wiemy ze jestesmy na ostatnio znanym zapisanym stanie wlasnie zeby uniknac tego cornera i byc moze innych tez

- No to jak nie autosavujemy calej historii w przeciwienstwie do starego rozw., co sie dzieje jak cofniemy do ostatniej zapisanej wersji
	- teraz jak nie zapiszemy to co?
		- póki nie zapiszemy przez Ctrl+S to autosave nie jest usuwany! więc wyświetli się to samo
		- jedyne co to jak zostawimy na dwie minuty na tym otwartą aplikacje to autosave sie wywali bo jesteśmy na ostatnio zapisanym savie
			- no to nic, po wczytaniu juz nie bedzie dostepny ten autosave wtedy
			- ale dlatego piszemy zeby szybko zapisac, zreszta domyslnie przeciez bedzie pokazana nowa wersja i ona bedzie szla z automatu do autosave
		- Myśleliśmy czy autosave mógłby niby być obliczany z najdalszej wersji, niekoniecznie obecnej
			- Ale to nie zadziała dobrze, mylące by było
			- Jak nie zapisujesz historii to coś trzeba i tak "stracić" i lepiej stracić tą "najdalszą" wersję a zachować tą explicitly wybraną

# Przesyłanie map

- Rundy komunikacji:
	- 0) DL: Reliable message: community_arena_meta (spokojnie sie zmieści)
		- name <= 30 bytes
			- Stąd wczytujemy i sanityzujemy nazwe mapy
				- alfanumeryczna_ i <= 30 znakow (pełna nazwa mapy może być dłuższa)
					- Pełna nazwa może być dowolna, tutaj nie jest wysyłana
		- version - wersja gry <= 32 bytes
		- last_changed_timestamp <= 32 bytes
			- czy zamiast tego po prostu kompletny hash przed podpisaniem?
			- tak bedzie bardziej stateless
			- okej tylko z drugiej strony chcielibyśmy wiedzieć czy jeśli jest różny to jest nowszy czy starszy
			- więc jednak timestamp będzie lepszy
			- tylko... co jak się zmieni sam obrazek np.?
				- to i tak trzeba podpisać jeszcze raz
				- to może lepiej timestamp + sygnature?
				- no właśnie tylko corner: skąd wiesz czy wersja jest nowsza czy starsza jeśli zmieni się sam hash jakiegoś obrazka?
					- jeśli nie nastąpiło zapisanie to timestamp zostanie ten sam
					- to by sporo przemawiało za trzymaniem hashy w jsonie
			- i swoja droga tutaj od razu odrzucimy formy ataku typu "nowa cyberaqua" bo jak mamy podpis tego public keya na ta nazwe mapy z takim hashem to nic sie wiecej nie sciagnie
		- public key = <= 100 bytes
		- to już nam mówi wszystko czy jest aktualna, nowa, stara, albo czy duplikat z taka sama nazwa
		- also nawet jeśli to wyślemy masterserverowi to klientowi też chcemy bo miedzy tym co jest na ms a na serwerze mogła się zmienić mapa
		- Swoją drogą ten reliable też wysyłamy każdemu już połączonemu klientowi za każdym razem jak zmieniamy mapę (na customową bo jak na oficjalną to nie trzeba)
			- Flaga będzie odpowiednia w komendzie zmiany mapy czy mapa jest oficjalna czy customowa
	- 1) Przerwa dla klienta na to żeby wysłał potwierdzenie że chce pobierać, jeśli mapa mu się nie zgadza z jakąś która jest na dysku
		- proste, klient u siebie w handlerze nowych plikow robi po prostu ifa czy juz została potwierdzona chęć, jeśli nie, to znaczy że server malicious
		- W przypadku zamiany NIE USUWAMY STAREJ MAPY a zmieniamy jej nazwę na .old, np. de_cyberaqua.old
			- Zarówno przy zweryfikowanej aktualizacji i całkowitej zamiany na mapę innego autora
			- w razie gdyby nawet autor chciał zrobić jakiś dowcip albo ktoś mu wykradł klucze, będziemy bezpieczni
	- usuwamy folder  de_cyberaqua.new
	- tworzymy folder de_cyberaqua.new
		- Myśleliśmy żeby *kopiować* cały obecny (jeśli istnieje) zamiast tworzyć nowy pusty
			- Dlatego ze dla inkrementalnego update potrzebujemy kopii obecnych plików 
		- Ale nie jest to potrzebne. Przy aktualizacji kopiujemy jeden po jednym tyko rzeczy ze starego które wykryliśmy że są potrzebne
		- W ten sposób nie musimy wywoływać funkcji która usuwa pliki potencjalnie na wadliwym inpucie
		- Usuwamy tylko poprzedni folder .new przed rozpoczęciem aktualizacji - a to jest  b. dobrze zsanityzowane
	- 2) DL: muzyka na ładowanie (lol)
	- 3) DL: (secure) same ścieżki które serwer sczytał z resource_hashes.bin 
		- sanityzujemy je na kliencie, jak wykryjemy jakiś przekręt to od razu disconnect
		- hashujemy też na kliencie więc nie ma sensu wysyłać hashy/timestampów
	- 4) DL: project.json
	- 5) DL: project.signature
	- 6) DL: wszystkie pliki
		- Klient musi samodzielnie shashować wszystkie pliki żeby obliczyć sygnature, nie jesteśmy się w stanie wcześniej rozłączyć w przypadku ataku
			- jak nie to przerywamy połączenie
	- Klient sprawdza czy sygnatura sie zgadza
	- Jak tak to:
		- przemianowujemy de_cyberaqua     -> de_cyberaqua.old  (jeśli stary istniał)
		- przemianowujemy de_cyberaqua.new -> de_cyberaqua

Sprawdzamy sygnature i git, można grać

- Serwer nie daje nam nazw tych głównych plików projektu (json, signature). 
	- Sami je obliczamy razem z rozszerzeniami z nazwy mapy i bezpiecznie zapisujemy.
	- Wiemy jaka jest kolejność. Beda tylko opcody na typ pliku i bedziemy weryfikowac czy zgadza sie ich kolejność
- hashujemy skonkatenowane 1) + 2) i sprawdzamy przeciwko 3)
- jak jest dobrze to sanityzujemy sciezki i zaciągamy pliki:

Note: Trzeba będzie wstrzymać przesyłanie solvable streama i uważać czy nas nie wywali od nieaktywności przez ten czas
Dopiero jak odbierzemy to wtedy server od razu wysyła initial solvable state aktualny

## Ściąganie nowej wersji mapy

- Tylko jak sygnatura sie zgadza
- Mając wszystkie hashe zapisane w resource_hashes.bin, trywialnie można zrobic zaciąganie tylko nowych/zmienionych plików
	- czy usuwamy teraz już niepotrzebne zasoby?
		- myśleliśmy czy zachowywać je gdyby autor chciał zrobić dowcip, ale i tak zachowujemy stara mape jako .old, więc nie trzeba
		- będzie minejszy clutter
		- tu chyba bardziej martwiliśmy się o opsec żeby nie podawać do funkcji usuwającej user inputa
			- ścieżki do usunięcia będą pochodziły z danych już na dysku zweryfikowanych więc lajtowo
			- ale czy na pewno? Jakby komuś się faktycznie udało złą ścieżkę wrzucić np.
		- **Nic nie będziemy musieli usuwać.**
			- Tworzymy przecież nowy folder, de_cyberaqua.new.
			- I konstruujemy go od nowa.
			- Po prostu kopiujemy istniejące pliki ze starego, jeśli odpowiadający hash został znaleziony.
				- Będziemy wysyłali listę hashów też najpierw zanim wszystkie pliki oczywiście
			- także opt-in a nie opt-out

## Path Sanitization

- Ważne! Najpierw tworzyć wszystkie foldery, wtedy nie da rady zrobić symlinków nawet jakby ktoś chciał

Uwaga: warto sanityzowac też za każdym razem nazwę mapy gdybyśmy ściągneli jakąś z neta
- Najlepiej nie otwierać wcale mapy która ma za długą nazwę albo jakieś niealfanumeryczne_ znaki bo to ewidentnie nie stworzone w edytorze
	- Edytor nie pozwoli takiej stworzyć przecież

Potrzeba sanityzacji ścieżek zasobów występuje potencjalnie w dwóch miejscach

- 1) Na pewno przy ściąganiu mapy z serwera, dlatego że programatically tworzymy foldery i ściągnięte pliki w filesystemi
	- Więc dajemy potencjalnie untrusted input do filesystem api
- 2) Po otworzeniu mapy ściągniętej z jakiejś strony
	- to jest subset 1) tak naprawdę przecież
	- ale chodzi o to że z warsztatów jakieś ściągnięte mapy też mogą mieć jakieś dziwne ścieżki wpisane
		- tylko że to już nam nic nie popsuje bo tutaj tylko czytamy pliki i nigdzie ich nie wysyłamy

Kiedy dokładnie sanityzujemy ścieżke?
- Gdziekolwiek jest I/O z jsona
	- Zarówno z pliku
	- jak i ze streama sieciowego
- A gdzie w kodzie?
	- po prostu w tym monolitycznym pliku do readwrita
	- to w miare manualnie bedziemy robic wiec bedzie kontrola nad tym

- Każdy path to jeden string z max size 256 żeby netcode był łatwiejszy
	- robimy na tym strtok albo std::view::split 
		- ta implementacja nawet jak bedzie zbugowana to nie bedzie tragedii
		- i tak zweryfikujemy porzadnie wektor z rezultatami, tam kazdy musi byc alfa_numeryczny i tyle
		- to juz niemozliwe bedzie do zbugowania
	- osobno extension! Wtedy nie trzeba nawet sprawdzac kropek
	- ale mozna dla prostoty po prostu zrobic replace all "." na "/" i potem ten split po "/"
		- tylko zas trzeba uwazac zeby ktos nie nazwal z dwoma kropkami, to chyba konwencja powinna wymagac
			- bo inaczej sie to zamieni w folder wtedy
- i teraz tak
	- tworzysz wszystkie foldery od razu żeby symlinków potem nie dało sie zrobić
	- konstruujemy pelny natywny path ktory przy okazji jest natywny:
		- full_path = full_path / path_part;
	- a zamienianie "\" na "/" to jest problem i/o jsonowy, to jest zupelnie niepowiazane i w innym miejscu juz (w serializacji)
	- jak to w miare recznie bedziemy robic to bedziemy mieli nad tym kontrole


# Organizacja wszystkich projektów

Ostatecznie:

- content/arenas - oficjalne
- user/downloads/arenas - sciagniete
- user/projects - lokalne projekty

# Serializacja

- Hybryda monolityczno-generyczna. See example.project.json
- Na razie najlepiej dwie monolityczne funkcje ręcznie napisane do serializacji też żeby było jasne jak co idzie i z jaką nazwą

# Filesystem dock

- ad hoc atlasy
	- tam można wrzucać zawsze mapke z idami dla obecnego folderu/calego filesystemu na start
