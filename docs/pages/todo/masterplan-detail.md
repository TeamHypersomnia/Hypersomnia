---
title: The masterplan detail
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: Detailed explanations for the decisions made in masterplan.md
---

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

- Do hashowania nie robimy naturalnego orderu tylko leksykograficzny

- Warto i tak hashować wszystkie na raz zamiast wybrane 


Plik którego jedynym zadaniem jest przetrzymywanie hashy wszystkich używanych zasobów.

Ale zastanów się czy to w ogóle trzeba trzymać bo to wszystko jest obliczalne z samego jsona
Jedyny powód dla którego to miałoby siedzieć na dysku to jest chyba jakbyś przesunął pliki podczas gdy edytor jest wyłączony

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
### Rozkminy wczesniejsze

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

## Ściąganie map

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
