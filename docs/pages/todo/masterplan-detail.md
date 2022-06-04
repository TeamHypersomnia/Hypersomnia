---
title: The masterplan detail
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: Detailed explanations for the decisions made in masterplan.md
---

## Autosave

- .cache generalnie powinien byc safe do wyjebania wiec moze autosave trzymajmy obok i ignorujmy?
	- no dobra tylko ze historia tez teoretyczni nie jest safe do wyjebki
	- chyba ze historii nie zapisujemy? 
		- chyba bym nie zapisywal bo:
			- po updejcie nagle i tak sie rozpierdoli a jak bysmy zapisywali w jsonie to bylaby mega ogromna
			- o tyle szybszy bedzie zapis a historia sie bedzie rozrastac mocno
## Signing

- Dlaczego signujemy wszystkie zasoby?
	- Skrypty też później będą konieczne dla integralności mapy
	- Więc równie dobrze możemy (i będziemy) signować obrazki/dźwięki
- Dlaczego signujemy wszystkie nieużywane zasoby?
	- Po to żeby nie trzeba było lazy wrzucać znowu joba hashowania jak nowy obiekt uzywany obiekt damy na mape
		- lepiej zrobic to raz a dobrze
	- wtedy jesteśmy całkowicie niezależni od historii/stanu projektu tylko od dostarczonych rzeczy przez użytkownika
	- i wtedy leci do cache i to moze byc nawet binarka z hashami i timestampami plikow hashowanych
	- zawsze przed podpisywaniem sygnatury robimy i konczymy skana, upewniamy sie czy jest skonczony job hashowania
	- nawet lepiej bo ten co bedzie sprawdzal sygnature bedzie zmuszony wygenerowac se hasze samemu z wszystkiego co dostal i sprawdzic se takze cleaner
	- wiec to deterministic z rzeczy danych przez uzytkownika a nie stanu/edytora
	- i tak 99% czasu aktualnie uzywane pliki beda sie pokrywaly z tym co jest resource_hashes.bin
		- jak serw tego nie ma zaczynajac hostowanie to se moze policzyc nawet sam i to bez czytania pliku json

- Zatem autor signuje tylko mape i resources (hasze), ale niesanityzowane (zeby nie bylo problemow z determinizmem jakims)
	- jakby signowal same hashe w jsonie to nie signowałby sciezek

## Plik resources

Note that if we hash files and keep them in the json, we need to re-save that json... so that it holds regenerated hashes
It sucks a bit and complicates workflow.
ACTUALLY we can keep those in a separate file! For which there is no history or anything like that.
This is why we need resource_hashes.bin

Plik którego jedynym zadaniem jest przetrzymywanie hashy wszystkich używanych zasobów.

- Do hashowania nie robimy naturalnego orderu tylko leksykograficzny
- Warto i tak hashować wszystkie na raz zamiast wybrane 

Ale zastanów się czy to w ogóle trzeba trzymać bo to wszystko jest obliczalne z samego jsona
Jedyny powód dla którego to miałoby siedzieć na dysku to jest chyba jakbyś przesunął pliki podczas gdy edytor jest wyłączony
- Nie tylko, optymalizacja też czasu wczytywania, hashowanie wszystkich plików może trwać

Po co nam hashe:
- Autor musi mieć jak signować zasoby a nie będziemy signować bezpośrednio wszystkich pngów/wavów itp tylko jeden plik z dwóch jsonów
- Łatwe wykrywanie przeniesionych plikow i proponiwanie redirecta
- Łatwe wykrywanie które pliki są do aktualizacji
- Czyli 99% czasu normalnego funkcjonowania edytora nie jest nam potrzebny ten plik
	- Najwiekszy corner to jakby sie zle zapisal i podczas gdy edytor jest wylaczony ktos przesunal plik
		- ale wtedy tez nie ma tragedii bo sie po prostu pokaze blad w edytorze ze te sciezki zginely i user musi je przywrocic
- To tez jednoczesnie z sygnatura bedzie wypisywane zeby wlasnie serwer tez nie musial hashowac wszystkiego po kolei samodzielnie
- Jak hostujesz serwer to oprocz rekompresji mozna tez sprawdzic czy digital signatures sie zgadzaja
	- zeby wlasnie ten corner case obsluzyc ze mapa sie dobrze zapisala a sygnatura nie
- jeszcze myslalem czy by sygnature nie zintegrowac do srodka tego jsona jako base64
	- tylko ze wtedy znowusz bedzie sie zmieniac ona za kazdym razem jak sie zmieni png jakis na dysku

Także to chyba bardziej helper

Poprawna sygnatura może być w całości obliczona z jsona + hashy jeśli są
	- No dobra ale key też jest potrzebny więc musimy podpisywać od razu po zapisie

Serwer przy hostowaniu sprawdzi czy sygnatury wszystkie się zgadzają i da prompta żeby zapisać jeszcze raz plik
	- Also serwer będzie mógł przyjąć jako argument że arena ma pochodzić z folderu projektów

Na pierwszy rzut oka to brzmi jak coś co może polecieć do .cache, bo hashe oczywiście są regenerowalne.
Problem w tym że to zależy też od stanu samej mapy, żeby wiedzieć co hashować.
Ewentualnie jeszcze możemy hashować wszystko jak leci zawsze tak jak jest na dysku.
Bez względu na to czy to jest używane jeszcze czy nie.
Wtedy to jest niezależne od stanu w edytorze! I staje sie pelnoprawnym cache
- No dobra ale nawet jeśli przyjmujemy że generujemy tylko na zapis do dysku to też jest pełnoprawny cache
- Coś nas niepotrzebnie boli że przesyłamy coś co jest w cache.
	- Ale przecież dosłownie skompresowaną mapę przesyłamy nawet z cache. A ona jest generowana z ostatniego zapisanego stanu.



- Dobra a jakby osobno byl w .cache
	- Zawsze jest obliczalny z obecnie zapisanego jsona
	- resource_hashes.json
	- i ustalamy lexicographic order po sciezkach

- Wbudowany
	- Ok reimport assetów musi triggerowac koniecznosc zapisu sceny wtedy
		- Czy to by było aż takie złe?
			- Troche nieintuicyjne byłoby jak cofniemy do stanu ktory ostatnio zapisalismy a tam dalej gwiazdka
			- no i jakos dziwnie ze rownolegle do historii jest zmiana ktorej nie da sie cofnac tylko jest globalna
			- bo ten plik czesciowo jest zalezny od historii a czesciowo nie jest i to jest takie
		- A bysmy mieli jeden plik mniej
		- Osobno resources = {} po prostu obok prefabs z hashami
			- autosave by mial wtedy ogarniete np. nowe sciezki juz bez potrzeby regeneracji

- Osobny
	- Kiedy tworzony
		- Na żywo aktualizowany poza procesem zapisywania mapy
			- Po to żeby zmiana w png nie triggerowała konieczności zapisu samej sceny
		- Za każdym razem gdy zapisywana jest mapa (ale nie z autosavem)
			- Bo zmienia sie potencjalnie scena

Chyba jednak nie robimy osobno pliku resources.

- Pro osobnego
	- Nie wali nam workflowa zapisywania mapy jak cos sie zmieni na dysku
	- jasno powiedziane ze calkowicie osobne od historii zamiast pol na pol (istnienie sciezek tak/hashe nie)

- Con osobnego
	- Dziwne kminienie kiedy generowac
	- Bardziej skomplikowany workflow podpisywania
	- Musisz trackować ostatnio zapisaną na dysk wersje żeby wygenerować z niej poprawne ścieżki te które są wymagane faktycznie
		- Dobra ale generujesz tylko w momencie zapisania

- Pro wbudowanego

- Con wbudowanego
	- Generuje koniecnzosc zapisu ktorej sie nie da cofnac

## Przesyłanie map

- Note: mozemy chciec przesylac przez neta jednak jsony zamiast binarnych wersji bo:
	- jak zweryfikujemy sygnature gdy bedziemy aktualizowali mapke ze to jest tego samego kolesia?
		- serializacja musialaby byc w pelni deterministyczna co do bita a tego nie jestesmy w stanie zagwarantowac przy floatach
	- jak ktos bedzie chcial modowac gierke i zmienic abi to przypał jak sie bedzie laczyl

- Pierwsze co poleci to pare bajtów z metą o obecnej mapie
	- Chcemy wcześnie odrzucać stare wersje map i przypadkowo stworzone przez kogoś innego mapy o tej samej nazwie (wtedy public key bedzie inny)
		- 99% w ktorym inna wersja mapy bedzie odrzucana to jak community serwer zahostuje uczciwie stara wersje mapy (bo jeszcze nie zaktualizowal)
		- więc możemy na tym polegać spokojnie bo potem i tak będzie weryfikowane to
	- ale to nie musi być oddzielny plik też, moze byc czescia project.json i przesylane przez siec tylko to co potrzebne z tego jako reliable message
	- chcieliśmy tylko wcześniej żeby to było oddzielnie
	- ale to będzie signowane też więc już wygodniej serio będzie w środku mieć
	- jedyny powód dla którego chcieliśmy mieć to oddzielnie to jakaś strona jakby chciała sobie wczytać
		- ale to na lajcie może sobie raz sparsować ten plik albo tylko zacząć
		- jest lookahead parser w rapidjsonie chyba
		- to też będzie pierwsze w kolejnosci w tym pliku

- Martwiliśmy się że jak bedzie zbyt skomplikowana struktura plików w projekcie, to bedzie trzeba duzo rzeczy ignorować i bedzie error-prone
	- ALE Przy przesyłaniu bedziemy uzywac whitelista niz blacklista do ignorowania - project.resources.json
		- Będziemy w stanie stwierdzic ktore pliki sa potrzebne
		- To bedzie oczywiste z contentu jsona tez
		- Obczaj ze nawet skryptom bedziemy dawac w pliku w jsonie jako input liste plikow do wczytania
			- zeby im nie eksponowac i/o calego i bedzie latwiej zsandboxowac to
		- Takze projekt jak najbardziej wie co mu bedzie potrzebne
	- Ogólnie ściąganie i tak nie będzie piękne bo najpierw trzeba będzie poprosić o sygnaturę
		- Choć to głównie przy updatowaniu
			- Np. Sprawdzić czy nie jest zrevokowany key jakiś
		- Więc Najpierw ściągasz resources file
	- Więc się nie ma co tym martwić, o "tancowanie z ignorami" albo czy cache sie wysyla itp

## Sanityzacja sciezek

- Dobra czyli sanityzacja i podczas wysyłania
	- żeby wiedziec gdzie wrzucić pliki i z tego obliczyc hashe
- I podczas działania aplikacji
	- Ale tu już jest bezpiecznie.. choć tu już bezpieczniej bo tylko czytamy dany plik i nikomu nie wysyłamy
		- Nie executujemy nic i tak
	- Więc może na razie niekoniecznie nawet

- Pasowałoby sanityzować też nowo wczytaną mapę ściągniętą
	- Bo skąd wiadomo że też nie jest walnięta
- Czyli właściwie na każdej instancji w ogóle uzywania jakiejkolwiek ścieżki sanityzować
	- tak bezpieczniej w ogóle
	- no to chyba nie jest problem też
	- replace dots with "/" i potem normalnie sprawdzasz
	- ewentualnie nie trzymać rozszerzenia w ścieżkach w ogóle? to by było cool nawet
	- ok ale i tak sanityzacje sciezekj akby co musimy robic juz przy wysylaniu bo musimy wiedziec gdzie wstawic pliki
		- tak czy siak te hashe mozemy a nawet musimy policzyc sami wiec nie musimy przesylac przez neta ich
	- wtedy bysmy mogli bez komplikowania netcodu robic sanityzacje dopiero po sciagnieciu mapy jsona
		- i disconnecta od razu robic jak sie wywali

- Path sanitization
	- w jsonie moga byc roznie dziwne sciezki wpisane do wczytania
	- i jak ktos przysyla plik to tez moze glupoty wpisac w filename
	- najbezpieczniej byloby przeslac cale jako archiwum a w jsonie poslugiwac sie guidami ale znowusz tak nie robimy
		- a czy nie moglibysmy dla czytelnosci w jsonie normalnie do wersjonowania trzymac "gfx/some_folder/to"?
			- mozna tak samo sparsowac to przeciez na podstawie '/' i potem sprawdzic czy tylko alfanumeryczne
				- na poczatku jeszcze moze byc slash...
				- ok ale manualnie konstruujemy sciezke z samych alfanumerycznych komponentow
		- aha tylko co z symlinkami
			- ktos by se mogl stworzyc binarny symlink i potem ustawic jego jako docelowy folder
		- mozna niby sprawdzac czy sciezka nie ma tego co sciagnelismy ale eh

- secure_send_resources
	- Dobra ale to możemy wysłać po prostu jeden string z max size 256 żeby netcode był łatwiejszy
		- i zrobić na nim strtok albo std::view::split 
		- ok w kazdym razie do zrobienia bo najwazniejsze ze ustalamy konwencje
		- i i tak jeszcze ostatnia linia obrony to bedzie ten absolute file path

	- W jednym miejscu resolvujemy już wszystkie ścieżki do plików żeby nie było zaskoczeń
	- po prostu przez sieć leci plik project.resources.json przetłumaczony do bezpiecznej struktury
		- dosłownie wektor alfa_numerycznych wyrazów
			- taką konwencję robimy i chuj
	- Wtedy przed wysłaniem jsona konwertujemy go tak żeby wszystkie ścieżki zawierały hashe
		- albo niekoniecznie, możemy sobie potem bezpiecznie mapować
		- jeśli ścieżka jest taka jak być powinna to 1:1 deterministycznie się odtworzy z builder_secure_resources
			- wtedy to co w jsonie jest posłuży jako string do mapowania z tym co jest w resources json
	- taki resources json nawet jest dobrą czytelną specyfikacją tego co potrzebuje mapa

- Obczaj że to jest jeszcze pół biedy bo my tu `piszemy` a nie czytamy więc najgorsze co sie stanie to sie wjebie do jakiegoś folderu dziwnego
	- bez roota/admina i tak nie narobi
	- i nic nie nadpisze bo zawsze bedziemy sprawdzać czy istnieje plik wcześniej
	- https://snyk.io/blog/exploring-3-types-of-directory-traversal-vulnerabilities-in-c-c/
	- https://portswigger.net/web-security/file-path-traversal
	- https://stackoverflow.com/questions/55610499/can-filesystemcanonical-be-used-to-prevent-filepath-injection-for-filepaths-pa
	- https://gamedev.net/forums/topic/334548-file-automatic-download-security/334548/

## Rozkminy wczesniejsze

- Problem: autoupdate breaks unsaved changes (autosave) to maps saved in binary, including history.
	- Dlatego żeby nie robić tańców wszystko trzymamy od razu w jsonie.
		- a historii w ogóle nie zapisujemy
	- Autoupdater musi sprawdzić czy istnieją autosavy
		- Ale tylko w folderze z projektami; nie pozwalamy gdzie indziej edytować na ten moment
		- Jeśli tak to po prostu cancel update
	
- Okazuje sie ze zhashowanie wszystkich plikow i zapisywanie hashy w jsonie bedzie calkiem przydatne
	+ Gdyby hashować wszystkie resourcy w jsonie to potem nie trzeba będzie hashować całego folderu żeby sprawdzić czy sygnatura jest git
	+ Łatwiej edytorowi potem rozpoznac jakby zmienila sie nazwa pliku gdzie on sie znajduje i zaproponowac redirecta
	- Gitara bo nawet mamy blake2/sha256 w libsodium a to juz mamy
		- albo nawet to: https://github.com/BLAKE3-team/BLAKE3/tree/master/c
			- duzo okejek ma i latwo zbudowac bedzie takze lajt
			- najlepiej od razu bedzie to wziac
	- Teraz czy tak samo robimy dla specjalnych plików np. z efektami?
		- Czy w ogóle trzymamy je oddzielnie jako pliki?
			- Myśle że nie i pokażemy je po prostu w UI jakoś ładnie jako specjalne obiekty które się nie zaliczają ani do gfx ani do sfx
			- a w filesystemie rzeczy tylko z konieczności
			- niech siedzi w tym jednym pliku wszystko co jest potrzebne do deterministycznej integralności mapy (to sie jeszcze przyda)
		- no skrypty i tak by pasowało chyba w oddzielnych plikach
			- bo przeciez jakos trzeba je edytowac w sensownym edytorze
		- ale to tylko skrypty bo one sa zewnetrznie edytowane
		- czyli nic za zapisywanie czego jest odpowiedzialny edytor bym nie zapisywal w oddzielnych plikach dla prostoty
			- latwiej bedzie tez zrobic safe writing of files z tym replacem i bakiem

## Wszystkie używane formaty tekstowe

- Używamy JSON:
	- Do kompatybilnego zapisu map.
		- Rapidjson wyglada pro i szybko bedzie przekompilowywal
		- json ma takie zalety ze jest wszedzie
			- i na razie nie piszemy oddzielnego zgeneralizowanego readwrite, na razie ad-hoc
	- Czemu nie LUA?
		- Nawet jesli sandboxujemy skrypty to przynajmniej jeden mniej wektor ataku bedzie jak przynajmniej te nieskryptowe dane porobimy w jakims jsonie.
		- Json bedzie mega szybszy do parsowania i mniej error prone
		- Json bedzie latwiej wczytac w jakims toolu webowym czy tam katalogu mapek
			- A tak to by taki tool tez musial sandboxowac lua albo konwertowac se, bez sensu
	- Czemu nie TOML
		- Owszem to była dobra rozkmina żeby już używać tylko jednego poziomu głębokości ale to już zależy tylko od nas
			- a w razie czego mamy taką możliwość
				- do świateł się przyda (moze)
					- tu nawet i tak mozna sekwencjami te atenuacje robic
						- albo i nie
				- wall_attenuation_quadratic zamiast wall: attenutation: { quadratic = 2 } itp
				- bo i tak bym trzymal chyba wszystko na jednym poziomie

- Uzywamy LUA:
	- Do user configow (default_config.lua i user/config.lua) 
		- bo one sa trusted i przydaja sie takie rzeczy jak pobieranie rcon passworda

- Uzywamy SANDBOXED LUA (pozniej). Jednym slowem dokakszamy sandboxing ZEBY NAWET NIE TRZEBA BYLO OSTRZEGAC.
	- Do client-side/server-side skryptowania. Dlaczego nie angelscript?
		- lua będzie lepsza bo będzie bardziej portowalna do przyszłych wersji
			- Wiesz jj2 już się nie zmieni
		- latwiejsza do nauczenia sie dla ludzi
		- sol2 sandboxing?
			- tak da sie bo jest sol::environment i mozna go dac do dofile itp
	- Nawet jesli chcemy tylko server-side, i tak chcemy sandboxowac lua.
		- Dlatego ze skrypty serwerowe i tak bedzie mozna sciagac z neta, i trzeba bedzie je jakos testowac. To chcemy w edytorze miec.
		- Swoja droga wtedy juz prawdopodobnie nie beda potrzebne purely server-side skrypty albo beda minimalne bo client-side script bedzie caly gameplay ogarnial

- Dopoki nie mamy client/server scripting nie przejmujemy sie niczym
	- bo i nawet jak ktos sciagnie mapke z neta to tylko w jsonie
		- i nawet bysmy mogli przesylac jsony zamiast binarek
		- i tak ustalilismy ze przesylamy binarnie i dekompilujemy po stronie klienta
			- a swoja droga serializacja szybsza duzo niz deserializacja bedzie

## Organizacja plikow w projekcie

- Btw. ctrl+s to juz writeout do wszystkich plikow zmodyfikowanych bez wariowania z rozpatrzaniem plikow osobno

- Wydaje mi się też że przy exporcie powinien być jeden json i już olać to bawienie się w jsony sąsiadujące z pngami
	- Łatwiejsza logistyka
	- I tiled chyba też tylko jeden json exportuje

## Orgnaizacja wszystkich projektów

- Po deliberacji chcemy jednak osobny folder na downloady 
	- żeby było zupełnie jasne dla mniej doświadczonych co im zżera miejsca najwięcej

- community maps musi isc do usera! bo tylko user i logs sa zachowywane przy upgradowaniu gierki
	- czemu nie osobno obok usera?
		- moznaby sie klocic ze user a community to jednak rozne rzeczy
			- ale jednak fajnie bedzie miec w jednym folderze wszystko co jest potrzebne do migracji uzytkownika

- no i ok, wszystkie downloady w userze to juz ustalone
	- pytanie czy do user/community/arenas, czy user/arenas? Czy user/builder razem ze wszystkim?

- Najbardziej jasne bedzie user/downloads/arenas, user/downloads/mods itp

## General

To nie jest sublime text! To ma byc standard do ktorego wszyscy sa przyzwyczajeni

- Wszystkie mozliwe pliki odzyskiwania trzymamy obok project.json. Czemu?
	- W razie paniki bedzie od razu widac ze cos ocalało (zamiast schowane w .cache)
	- Rootowy folder to dobre miejsce dla tych paru waznych rzeczy. 
		- I tak tam nic nie bedzie oprocz pliku json i folderow gfx/sfx/scripts wiec byloby pusto


- Dobra prosty json i tyle zawsze jeden a binarkami sie w ogole nawet nie przejmujmy teraz
	+ zero kminienia workflowow w developmencie, tym bardziej ze bedziemy mega duzo iterowali i testowali
	+ zero kminienia czy update dobrze konwertuje, zawsze te same pliki bez zbednego pierdolenia i backupowania
	+ also watpie zeby disk io byl tu problemem bo 
	+ normalnie folder wysylasz caly od razu
		+ niezaleznie od tego przez neta i tak mozna kompilowac ten jeden plik i przesylac go binarnie
		+ lol nawet niekoniecznie bo lz4 sciska do 4% bajtow z tego (testowy int.lua: 10mb -> 400kb)
			- na -9 robi 1.86%...
			- mysle ze nawet juz nie bedzie sie oplacalo tego binaryzowac
			- na samych bajtach robi 31% co prawda
	+ pamietaj ze binarki i to wszystko naprawde mozna zrobic pozniej
		- proste rzeczy co najpierw tez bedzie mozna od strzala zrobic:
			- np. po updacie mozna rekompilowac zawsze od razu wszystkie mapy zeby sie szybko wczytywaly do normalnej gry i nie ma spiny o przypal jakis
				- mozna np. ze main menu setup skanuje wszystkie zawsze mapy i sprawdza wersje binarnych plikow (to ez bo czytasz kilka pierwszych bajtow)
		- wiec z wczytywaniem do gry nigdy nie bedzie problemu
		- a teraz zapisywanie szybkie
			- mozna dorobic fast-save binary mode dla tych co tworza duze mapy i tez bezprzypałowo
	+ nie musisz debilnie wstryzymywac updata bo nic sie nie rozwali
		- autosave jest usuwany tylko na writeoucie
	+ czemu to nie ma znaczenia:
		- roznica w szybkosci wczytywania w ogole nie bedzie miala znaczenia dla malych mapek
			- prawdopodobnie jeden png bedzie sie wczytywal dluzej niz cala mapa z kilkoma obiektami
		- doslownie to bedzie ulamek dlugosci wczytywania co obrazki
			- worst case scenario dla cyberaqua to mysle jest 1 MB (sprawdzalem i cos ~1600 obiektow jest, to z jednym obiektem ~655 B)
				- zanim takie duze mapy spolecznosciowe bedziemy mieli to wiesz
				- z tym exportem 10megabajtowym jest taki shitshow bo wszystkie defaulty sa serializowane nawet dla glupiego blank image
		+ zawsze mozna przywalic lz4 kiedystam
		+ i raczej wlasnie w druga strone bym robil czyli na start najprostsza wersja bez kminienia kompatybilnosci a jak ktos chce high performanc to sobie moze odpalic experimental binary saving mode
		- autosave tez bym robil w json

# Uncategorized

## Filesystem dock

- ad hoc atlasy
	- tam można wrzucać zawsze mapke z idami dla obecnego folderu/calego filesystemu na start

- poszukac jakiegos przykladu imgui filesystem z miniaturkami
	- ale nie ma biedy też bo zawsze można manualnie prosty sklepać 
	- wtedy wieksza kontrole nad drag and dropem na mape tez bysmy mieli

- also mozna dac tick na official content jak w unrealu show engine content

- Na razie bym sie nie spieszyl z tym wywalaniem tego edytora starego bo bedziemy referowac sie do niego jesli chodzi o gui
- Warto przemyslec asynchronicznosc dla szybkosci iteracji jakby ktos modyfikowal grafiki w zewnetrznym programie i chcial od razu widziec jak wyglada w grze

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

## Scene state/"prefab" hierarchy

- Traktujemy każdy png, wav jako prefab od razu

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
b) We will store the map files *TEXTUALLY* (in json) to be able to easily convert the maps
Being able to nicely version the maps is a plus.

### Upgrading textual maps from deprecated formats

We will somehow ask the user if they want to convert the map to the higher version,
if deprecated names are found in the textual format.

We might need incremental update procedures. 
Perhaps we won't do this with just a simple find/replace (or it might affect the values),
but with some json node traversal logic so that only keys are affected.
- Anyways, this is for later! Once we actually have lots of community maps.
