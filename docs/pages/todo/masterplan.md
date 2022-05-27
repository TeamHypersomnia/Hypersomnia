---
title: The masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: We need to set our priorities straight.
---

# Builder

- Pamietaj ze PoC nie musi byc turboresponsywny na kazdym kroku
	- Mapa bedzie juz w dobym przenosnym formacie to jest najwazniejsze. Potem to mozna stopniowo ulepszac

- No i git przypomnielismy sobie jak dzialaja ad hoc atlasy
	- tam można wrzucać zawsze mapke z idami dla obecnego folderu/calego filesystemu na start

- Btw. ctrl+s to juz writeout do wszystkich plikow zmodyfikowanych bez wariowania z rozpatrzaniem plikow osobno

- poszukac jakiegos przykladu imgui filesystem z miniaturkami
	- ale nie ma biedy też bo zawsze można manualnie prosty sklepać 
	- wtedy wieksza kontrole nad drag and dropem na mape tez bysmy mieli

- Na razie bym sie nie spieszyl z tym wywalaniem tego edytora starego bo bedziemy referowac sie do niego jesli chodzi o gui
- Warto przemyslec asynchronicznosc dla szybkosci iteracji jakby ktos modyfikowal grafiki w zewnetrznym programie i chcial od razu widziec jak wyglada w grze

- also mozna dac tick na official content jak w unrealu show engine content
	- to bedzie read only wiadomo i nie przebudowywane nigdy tylko raz ew. wczytane do plikow i tekstur

- Ok czyli jeszcze raz - aktywacja okna:
	- Rebuild whole filesystem
		- po primo budujemy hierarchie dla eksplorera czyli
		- dla kazdego znalezionego pliku rekursywnie

		- ofc jak mam reprezentacje w pamieci z timestampem takim samym to nie musze przeladowywac
	- i co teraz
		- Sprawdzamy czy coś z istniejacych rzeczy nie zostalo wyjebane
		- (TYLKO JAK MAMY NIEZAPISANE ZMIANY NA NIM!) Jak timestamp sie zmienil to komunikat - changed on disk, do you want these files: reload (yes/cancel)


	- Mozna wczytac do pamieci wszystkie pliki obrazkow, nawet do tekstur bo i tak zakladamy ze na razie sie pomiesci wszystko
		- Mapy i tak juz beda w dobrym formacie wiec jak bedzie trzeba to ulepszymy performance pozniej bez strat w ludziach

- Czy trackujemy wszystkie rekursywnie w filesystemie? Czy tylko te ktorych uzywamy aktualnie na scenie?
	- To jest bardziej rozkmina chyba typu czy chcesz pokazywac dla wszystkich nawet nieuzywanych komunikaty o popsutych sciezkach
		- a poza tym to lazyloadowanie wszystkiego chyba niewiele ci da
		- latwiej bedzie i bardziej modularnie jak zbudujemy caly filesystem to mozemy od razu podac do logiki okienka
		- i tak chcemy te mapy cale ladowac do gry wiec jak tu sie bedzie za wolno wczytywac to tym bardziej do gry
			- potem jak bedziemy mieli giga giga mapy to mozemy robic to lazy wczytywanie
	- Do widoku filesystemu w dolnym panelu i tak musimy jakos ladowac te wszystkie png
		- ale to chyba oddzielna kwestia jest
		- i nie wiem czy bym i tak nie odswiezal tego widoku jesli chodzi o same filenamy przynajmniej a cachowac ewentualnie obrazki same?

- wydaje mi sie ze ten lazy loading niewiele nam da a aktywacja okna to jest dobra granica miedzy potencjalnymi zmianami w filesystemie
	- wiadomo ze i tak nie bedziemy generowali yamlow wszystkich od razu

- Co dokładnie przeładowujemy na aktywacje okna - procedura
	- W momencie rozpoczęcia przeładowywania będziemy mieli w pamięci reprezentacje yamlów tak jak ostatnio były writeoutowane 
		- i to info posłuży nam do łatwego znalezienia przekierowania
		- a jak akurat bedzie wtedy wylaczony edytor to ewentualnie w jakims keszu zeby dalo sie wykryc
	- i tylko skanujemy czy wywalony zostal ktorys z uzywanych na scenie
		- reszta ktora ktos dodaje zupelnie nas nie interesuje dopoki to nie jest wciepane na scene
	- Najpierw musimy przeskanować to czego nie ma żeby potem powiązać z rzeczami które zostały przeniesione a byłyby potraktowane jako nowe?
		- Choć tak naprawdę po prostu rebuild od nowa wyjdzie na to samo
	- tylko nas interesuje skan tych rzeczy co uzywamy
	- myslimy o tym tylko jako o edytorze dla tego pliku .project 
		- a reszta tych yamlow/plikow w ogole nie musi istniec w pamieci ich reprezentacja dopoki nie sa uzywane na scenie

	- Dobra to mając ten path system
		- Aktualnie jedyne co potrzebujemy przeladowywac na start to UZYWANE sciezki - wiec to musimy trackowac
	- A `nowe` sciezki tylko jak otwieramy w dialogu dany folder - czyli przeladowujemy tylko folder filesystemu obecnie widoczny
	- i od razu obczajamy last write time i przeladowujemy (ale tylko yamle bo przeladowywaniem pngow sie zajmuje viewable streaming)
		- wiec jakby tutaj pngow samych edytor nie widzi w kwestii stanu a one sluza tylko do tego zeby wiedziec ze trzeba wygenerowac nowy yaml


	- Musimy sprawdzić czy coś nowego się nie pojawiło więc i tak O(n) trzeba przelecieć rekursywnie cały folder
		- Więc od razu możemy pobrać write timy istniejących
		- I pamiętaj że "nowy" to może być zmieniona lokacja starego
			- takze sciezki nic nie mowia chyba
			- i by trzeba bylo ladowac mety cale
	- Co jak ktoś skopiuje yamla i zostawi ten sam meta id?
		- Wtedy ten ze starszym write time powinnismy brac ale tbh to nie ma znaczenia na razie
		- po prostu jeden z nich zostanie wybrany a potem jak ktos usunie z dysku i przeladuje znowu to i tak wroci do normy bo to powinno byc resilient takie wlasnie
		- a skoro zduplikowany to bedzie tak samo wygladalo wiec nawet uzytkownik nie odczuje ze cos jest nie tak
		- wiec nie przejmowalbym sie duplikatami na teraz
		- no dobra chociaz w sumie komunikat by sie przydal
	- 
- also uzywajmy relatywnych sciezek, w yamlu z animacja np.
	- latwiej wtedy przerzucic sama taka animacje czy folder z obiektami pokoju do innego projektu
	- praktycznie od strzala

- asynchroniczny scanner zwroci nam tylko jakby liste affected sciezek
	- ale to walic na razie, synchronicznie zrobimy bo na razie to małe jest i bedzie bardziej clean less error prone

- Implementation
	- albo jedziemy z tego co mamy w editor setupie i wywalamy niepotrzebne
	- albo od nowa
	- wydaje mi sie ze od nowa lepiej bo tu sie bedzie wszystko roznic praktycznie
		- na pewno nam się okienka przydadzą zaimplementowane niektóre i jakies skróty porobione
	- zostawiamy stare zrodla w projekcie?
		- pod starym master branchem mozna
		- nie ma co sie bawic chyba w zamienianie
		- to sie nam przyda chyba jednak jako taki inspector
			- tylko bym nie budowal go do release albo nawet w ogole domyslnie
		- faktem jest że nie opłaca nam sie za bardzo tego utrzymywać
			- reward jest zbyt niski na tak długie czasy kompilacji i poprawianie errorów
			- jakby kiedyś to było critical to wtedy można przywrócić

- Raczej nie robiłbym zapisywania per-plik
	- miejmy jeden monolityczny writeout który wszystkie niezapisane zmiany do plików wypuszcza
		- unity chyba tak samo robi
	- te zmiany na plikach nie są samodzielnie tak znaczace żeby trzeba było je oddzielnie zapisywać, bez sensu by to było
	- ofc w pamieci binarnie robimy tylko liste niezapisanych zmian
		- i ten blob bedzie lecial do pliku raz naczas i to bedzie nasz autosave
		- nie bedzie nic nawet do autosavowania jak wrzucisz mase nowych plikow bo im sie od razu robi writeout na nowych yamlach

- Zmienia nam się autosave
	- Zróbmy jeden blob 

- Zanim zrobimy prosty kejs ze spritami to musimy na pewno rozkminić:
	- Ta strukture prefabów
	- Czy traktujemy faktycznie obrazek od razu jako prefab czy prefab to jest osobna rzecz i ma referencje do obrazka
		- o tyle latwiej ze nie trzeba managowac tych durnych referencji
			- 99% casow to jest ze jeden obrazek jeden prefab
				- nie wiem jak w tiledzie to jest np zaimplementowane ale to chyba niewazne
			- Jeśli sie okaze ze duzo jest kejsow ze ten sam obrazek leci na duzo prefabow to mozna latwo z poziomu atlas packera to opcić
				- a png osobny zawsze git nawet jest podglad ladny w filesystemie
		- dobra tylko czym jest wtedy prefab z kilku obrazkow? specjalnym yamlem?
			- to grupa entitow tak naprawde/skrypt utworzenia
		- i od razu typ wybierasz obrazkowi czym jest i moze byc tylko jedna rzecza
			- czy np dekoracja fizyczna/niefizyczna czy typem broni
				- i sie zmieniaja parametry do edycji w zaleznosci od tego
	- Czy jak duplikujemy z edytora to duplikujemy obrazek? Czy tylko referencje do obrazka?
		- Wtedy nowy yaml po prostu tworzymy na dysku
		- a domyslnie potworzymy dla wszystkich nowych obrazkow

- I tak samo dzwieki sa od razu defaultowo wszystkie ustawiane jako ambience
	- bo efektow dzwiekowych raczej customowych bedzie mniej ale nawet jesli to nie ma problemu
	- normalnie bedzie sie dalo referencjonowac mety dzwiekow (tak jak normalnie by to bylo z flavorami)
		- w jakichs nie wiem customowych broniach
- Lepiej zmniejszac complexity kosztem paru niewygodnych corner casow niz na odwrot
	- czyli sie upewniac ze 0 corner casow ale skomplikowana architektura

- GUIDy mozna tworzyc za pomoca randombytes_buf z libsodium

- i kiedy reloadujemy
	- viewables streaming ma opcje rescan changes, to jest osobno do samego contentu png
		- sprawdza na podstawie write times i calej bazy danych sciezek
		- i to robi asynchronicznie za kazdym razem jak sie aktywuje okienko
	- tak samo mozemy zrobic z yamlami bo one tez moga byc edytowane w fsie
	- czyli rescanujemy caly filesystem asynchronicznie tylko jak aktywujemy
		- popup mozna pokazac "rescanning" w razie czego (ale jesli np. trwa to dluzej niz 500ms zeby nie migalo ciagle)
	- na aktywacje okienka mozemy na razie przebudowywac cala reprezentacje binarna moze

- Also i tak musisz zaplanować reprezentację binarną tych wszystkich guid->yaml mappingów
	- żeby to było przesyłalne przez sieć bo nie będziemy yamlów wysyłali całych tylko binarne reprezentacje
		- żeby choć jeden mniej wektor ataku był

- Ok ale teraz tak myślę że nadal możemy mieć obok pliki z yamlami ale jednak nie robić guidów
	- tylko wtedy od razu wykorzystujesz guid system którym jest filesystem
	- i już nie musisz sie martwić że nowy to może być przeniesiony tylko nowy to nowy i tyle
	- to nie psuje raczej przenośności do nowych projektów
		- bo jak przenosisz cos to jeszcze nie jest nigdzie referencjonowane wiec nie ma znaczenia czy to na razie ma juz guida czy sama sciezke
	- takze guidy by ci tylko zrobily przenosnosc w obrebie samego projektu ale to i tak godot i ue zakladaja ze przenosic sie bedzie w srodku edytora
	- + jest taki ze referencje od razu sa zrozumiale w innych plikach zamiast haszy dziwnych guidow
	- takze guidy sie praktycznie przydaja tylko do jednego kejsa przerzucania recznie w filesystemie a poza tym wcale
		- i komplikuja architekture
		- a zawsze mozna uproscic


		

- History actions vs filesystem
	- i czy zapisujemy na poziomie pojedynczych plikow

- Musimy miec i tak w pamieci reprezentacje jakas tego wszystkiego sczytanego
	- moze jakis bin file temporary tez na dysk moglby leciec ignorowany?
		- to pozniej bo najlepiej na razie testowac na clean

- godot tez uzywa plikow import takze polecimy tu tez z yamlami

- I co robimy jak wyjebie sie plik png/yaml z dysku
	- png yaml: to easy po prostu regenerujemy default
	- png: pokazujemy pytajnik albo czerwony kwadrat w miejsce sprita
	- zmiana nazwy png to wiadomo - rownoznaczne z wyjebaniem
	- zmiana nazwy yamla - meta hash ten sam - ale wtedy nie sczyta obrazka, importuje tamten na nowo chyba ze nazwe zmienimy
	- animacja yaml:
	
- Animacje (z punktu widzenia struk. prefabow)
	- To że się wywali png jakis przez przypadek albo inaczej nazwie to sie nic nie dzieje
		- po prostu sie pokaza pytajniki
		- liczy sie jakby to co na poczatku zostalo wygenerowane defaultowo
			- a to i tak jest tylko default
		- w animacji zostana metadane tych obrazkow ze sciezkami
	- Tu jest kwestia tylko wygenerowania defaulta
		- Bo wiadomo że w edytorze bedzie można potem przekładać framy i durations
			- zeby wlasnie nie kopiowac milion razy tych samych framow
			- ale bedzie sie dalo w obrebie tylko tej animacji zeby nie bylo syfu znowu z referencjami
		- Potem wywalenie yamla z dysku spowoduje reset wygenerowanie defaultowego yamla od nowa
			- To tez mogloby byc history action
	- Czy robimy yamle dla kazdej klatki? 
		- W sumie niepotrzebnie
			- bo i tak ksztalt fizyczny czy meta tego rodzaju bedzie jedna na cala animke
			- neon map raczej tez nie bedziemy osobnych robili bo to syf by byl
	- Gify też można exportować 
		- Chyba nawet exportowaliśmy gify jakieś i nie traciliśmy kolorów zreszta tu zawsze mamy tylko kilka
			- wiec powinno byc bezstratne
		- i od razu moze wykrywac dlugosci klatek dzieki temu i mniejszy syf na dysku
	- Repeated frames in an animation
		- nie problem bo w edytorze zrobimy przekladanie/dodawanie framow normalnie ale tylko w obrebie danej animacji
		- no to po tak jak wczesniej po prostu atlas bedzie wykrywal 
			- murmurhash albo crc32
	- Obrazki nazwane cos_1 cos_2 cos_3 moze od strzala wykrywac jako animacje i dorabiac dodatkowego yamla dla animacji
		(albo samego yamla dla samej animacji)
		- co jak ktos zmieni nazwe i rozbije?
			- to yamla i tak bym nie usuwal zeby w razie czego odzyskac
	- i to tez jest od razu prefab, defaultowo pewnie ustawiamy na dynamic decoration
		- also pamietaj ze to nie musi byc jeden do jeden z typami w abi
		- mozesz tu ustawic domyslnie zwykla animacje i nazwac ja static animation
		- i mozesz osobno miec np. organism
		- to ze w abi to samo to nas nieinteresuje - tutaj wazne zeby rozroznic tylko po to zeby nie pokazywac niepotrzebnych proptów od razu
	- Animacje jesli maja miec jakis, to muszą mieć jeden wspólny kolider zdefiniowany w danym yamlu bo nie bedziemy zmieniali ciala fizycznego w zaleznosci od klatki
		- ale na razie i tak nie wspieramy chyba animowanych cial fizycznych (w abi do poprawki)

- Bardzo interaktywne
	- Jak najedziemy na obiekt to podswietla sie obecna warstwa jesli jest widoczna
	- i vice versa jak najedziemy na warstwe to podswietlaja sie lekko wszystkie na tej warstwie
	- jak klikniemy warstwe to? moze kamera centruje na wszystkie? niekoniecznie bo bedzie chaos
	- jak klikamy na obiekt to pokazuje sie gdzie jest w warstwie


- Prosty kejs rozpatrzmy z samymi spritami fizycznymi + niefizycznymi bo to bedzie 99% complexity
	- default sprite to jest niefizyk normalny nic nierobiacy z png
		- jak zaczynasz dodawac jakies parametry smieszne to sie generuje yaml dla niego
	- jak na mapie klikniesz to osobno w inspectorze instance properties
	- a w tym panelu na dole mozesz kliknac duplicate prefab
		- tworzy sie wtedy osobny png dla tego ale trudno tak ma byc
		- lepiej tak bo wtedy instantly extensible to jest
	- also viewabli nie potrzebujemy wtedy tez chyba dla kazdego png
		- to sa tylko specjalne offsety dla torsów a nie dla spritow normalnych
		- sprity normalne tylko ksztaltow potrzebuja
			- ale to jest abi wiec w sumie wyjebka
		- z tym ze to jest kwestia abi wiec nas to wali czy to jest osobno binarnie czy razem - my wolimy razem zeby miec abstrakcje
			- traktujemy to w tym momencie jak c union albo variant
	- W tym momencie chyba zawsze jeden viewable def na jeden flavor.
		- w sensie nie moze byc kejsa zeby jeden viewable def byl wspoldzielony miedzy dwoma flavorami
		- bo niemozliwe jest zeby jeden png fizycznie wchodzil do dwoch flavorow, jeden flavor = 1 png/yaml

	- no i wchodzi rozkmina czy identyfikacja hashami w metach czy pathy ale chyba hashe lepsze
	- Ordering
		- Zawsze bedziemy mieli total ordering
			- wiec nie potrzebujemy robic dupereli z jakims szukaniem ktory jest najblizej srodka jesli kilka kandydatow
		- zawsze tylko jeden kandydat - ten najbardziej na wierzchu
		- mysle ze w abi juz nie ma sensu robic rozroznienia na layery tylko po prostu widoczne sortowac
		- bedziemy i tak mieli tylko pare render layerow sortowanych w abi
			- ground i foreground basically
		- nawet już dokładnie sortujemy w abi zwykłym std sortem w zaleznosci od sorting order takze git
		- ten sorting order po prostu bedziemy inkrementowali i on duzy zakres moze miec, tyle dokladnie ile jest na scenie ich

- ABI bedziemy dostosowywac podczas pisania edytora tak jak bedziemy potrzebowali
	- bo to sa zupelnie oddzielne modularne rzeczy
	- wiec ono sie bedzie zmieniac bez w ogole przypału na jakiejs kompatybilnosci i o to chodzi

- wyobraz se piszesz osobny edytor z importem tak chyba najlatwiej
	- tylko masz od razu na zywo widok na to jak bedzie w grze wygladalo i po tym uzytkownik klika
		- ale poza tym to tak samo jak osobny editor z jakas opcja importu

- ogarnianie specjalnych obiektow sie pozniej ogarnie od tego
	- also specjalne customowe obiekty ktore beda yamlami tez beda mialy swoja ikonke tak jakby byly obrazkiem
		- wiec to bedzie nierozroznialne czy to png czy cos innego
		- pngom beda dodawane yamle jak beda niestandardowe
			- np. fizyczne 


- Najbardziej nas martwi chyba:
	- Dualnosc stanu zeby bylo wygodnie/wydajnie
		- tutaj dwa approache
		- bo pytanie jest jak przeprowadzamy interakcje na prefabie jak instancja jest juz na mapie 
		- mozna to robic per-entity niby jakos ale to jest meh bo jeszcze prefaby mozesz miec z kilku
		- przepis jest w stanie zaproksymowac gdzie na mapie sie znajdzie na jakim obszarze
		- to nie jest skomplikowana gra wiec najlepiej jakby juz to ingame bylo
			- wszystkie sensowne editory (ue/unity) robia w jednym oknie juz i widac od razu jak wyglada
	- Gdzie granica między invariantem a komponentem bo to nieoczywiste
		- niektóre będą oczywiste jak np. do layerów, wszystkie takie pozycjonalne rzeczy to komponenty ofc
		- kolory?
	- Cała translacja prefabow w inwarianty to bedzie nieoczywiste
		- ale obczaj ze nie musimy byc tu jakos super efektywni
		- bo z definicji inwarianty mozna stworzyc z samej definicji mapy, one nie beda przesylane przez siec
			- tylko definicja mapy (a to i tak tylko za pierwszym razem) + solvable
		- no mozemy zrobic jakis slownik na bajtowe zawartosci i cos typu automatycznego copy on write
	- To obliczanie/kompilowanie fizyki

- Simplest by było jakbysmy wykorzystali obecny edytor i jakos sprytnie konwertowali
	- ale to nie jest przyszlosciowe
	- i nie skonwertujesz se prefabow


- floor/wall materials no to pewnie oddzielne yamle
	- tylko po prostu juz duzo wbudowanych bedzie ze ludziom nie bedzie trzeba nowych tworzyc raczej

## Interacting with the built game world representation

- It's only a matter of mapping the hovered game-world object to the source in-memory only recipe object
- We can always remember what game-world object was created for a given map object
	- and this way straightforwardly have a bidirectional mapping
- with a bidirectional mapping we pretty much have everything
- we process highlights and clicks on the world entity and e.g. show the corresponding map object in inspector
	- "Show meta" button in inspector when an instance is selected
		- this shows the prefab in the filesystem panel
		- not sure about the naming but I thing "meta" is alright, let's not imply straight away that it's yaml I guess?

## Layers

Layery pewnie oddzielne fizyczne i niefizyczne, bez specyfikacji czy dany png ma byc fizykiem czy nie
	- nienienie, przy pngu bedziemy wybierali jakis typ zeby pokazywac tylko relewantne propty
	- tylko tu jest pytanie czy mamy np. warstwy fore/back i w kazdej moga byc i fizyczne i niefizyczne
		- bo niby teoretycznie jakies fizyczne ksztalty moga sie zarowno nad nami jak i pod nami rysowac
	- czy po po prostu jedna warstwa specjalna na same fizyki
	- w jj2 np. taki jest podzial

## "Compiling" map representation

- Wszystko domyślnie jest spritem z pnga i zachowuje sie inaczej w zależności od tego na którym layerze siedzi?

- Okej na cyberaqua mamy manualnie postawione colidery czyli pewnie mielismy perf problems z lightingiem
- Nie musimy sie na razie zajmować aż tak budowaniem jednego dużego polygona
	- Dlatego że i tak można przyspieszyc mega ten algorytm 


### Konwersja na inwarianty i komponenty

- Czy my w takim ukladzie w ogole bedziemy chcieli inwarianty?
	- byc moze do rzeczy takich jak gracze
	- To ABI jest bardzo dobre bo nam mowi jasno co jest constant a co trzeba retransmitowac przez siec
- Te inwariany/komponenty traktujemy jako abi po prostu
- Załóżmy prosty kejs ze rozstawiamy sporo spritów
	- I tutaj zauważ że jeszcze w abi masz do wyboru czy to jest statyczna czy dynamiczna dekoracja
	- jak wybieramy typy przy konwertowaniu?
		- okej ale dobra izi bo wszystkie te sprity to sa static decoration a dynamic to sa ryby czyli nie mamy az takiego wyboru
		- wiec to jakos inferujemy z parametrow wybranych

- Every png defaults to sprite
	- A oprocz tego moga byc yamle z dowolnymi parametrami
## State duality

- Mozemy pozwolic ludziom zupełnie nieoptymalnie układac sciany z małych kwdaracikow
	- A potem mozemy zrobic liste vertexow ktore maja znaczenie dla lightingu
		- to jakis convex partition
	- Bo to fizyce nie przeszkadza tak naprawde
		- chociaz w sumie troche dla perfu.. dobrze by bylo
	- No dobra to lepiej po prostu chyba polaczyc verty i zrobic convex partition

## Scripting security (sandboxing)

- Creation scripts
	- Nice to have to allow anyone to arbitrarily modify
	- Could be avoided if we just transmit the map like a "bsp"
- Gameplay scripts
	- Those will mostly run on the server
		- Running on the client would anyway be costly due to client-side prediction
		- Shouldn't be too hard to make the network protocol extensible enough

- So we mostly need creation/prefab scripts to be secure
	- But this does not even need to be scripted
	- A procedure could be defined in binary terms in our own code without some silly loops and such

- nawet jak w yamlu bedziemy trzymali lokalnie do wersjonowania rzeczy to i tak przez siec w bajtach to moze leciec
	- wiec sie nie musimy tutaj martwic o vulnerability

- Ludzie raczej nie będą potrzebowali complex creation scriptów, może prędzej layery se beda robili z ktorych beda cos kopiowali
	- Prefab faktycznie może być oddzielnym yamlem czemu nie
	- Uważać trzeba na rekursje

- Specjalne obiekty i tak nie beda skryptowane edytorze bo chcemy max performance
	- one beda w c++

## Folder structure

- Project itself will be stored in yml
	- the about section I think might be separate for the purposes of quick browsing

- project.yml
- project.about.yml
- project.miniature.png

## Map format

### Safety

**Choice of the text serialization format matters ONLY INSOFAR AS STORING ON HDD/VERSIONING IS CONCERNED.**
Look - even if you have two separate data formats:
a) One for the map scheme from which the proper binary is built;
b) and the low-level binary one that is understood by cosmos.cpp

And you need to send a) - you can always send the in-memory binary representation.
The game will then safely be able to convert it back to yaml for its own purpose.

Therefore saving to/loading from yaml will be only relevant when saving maps that are easily versioned.

### Preserving compatibility  

Our main issue with storing the maps binary was that any change in the binary structure would break them.
This is why 
a) We will have a separate generator format from which the proper binary maps will be generated in runtime
b) We will store the map files *TEXTUALLY* (in yaml probably) to be able to easily convert the maps
Being able to nicely version the maps is a plus.

We will always ask the user if they want to convert the map to the higher version,
if a new version of the game is detected.

We might need incremental update procedures. 
Perhaps we won't do this with just a simple find/replace (or it might affect the string),
but with some yaml node traversal logic so that only keys are affected.
- Anyways, this is for later! Once we actually have lots of community maps.

