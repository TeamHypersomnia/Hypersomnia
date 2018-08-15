---
title: Art style guide (GFX)
tags: [scratchpads_pl]
hide_sidebar: true
permalink: art_style_guide_gfx
summary: Założenia dotyczące grafiki.
---

## Kontur

- Preferujemy twardy, czarny obrys grubości jednego piksela na zewnątrz każdego obrazka.
	- Wyjątek: krawędze, wzdłuż których obrazek układany jest w kafelki, np. na podłogach lub ścianach.
	- Wyjątek: elementy, które są w dużej mierze światłem i które wystają na zewnątrz obrazka.
		- Np. główka elektrycznej pałki lub inne świecące elementy.
			- Te mają krawędź w odcieniu emitowanego światła, odrobinę przyciemnioną.
		- Np. obrazki cząsteczek lub smug pocisków.
			- Te nie mają krawędzi wcale.
- Zdajemy sobie sprawę z tego, że istnieją inne techniki kolorowania krawędzi, ale...
	- Zależy nam na tym aby granice obiektów były natychmiast widoczne.
	- Mniej mamy teraz na głowie, a zmiana koloru konturu jest czymś co można zrobić później, również po skończeniu pracy nad obrazkiem.

## Obrazki sztywne i dynamiczne

- Obrazek **sztywny** to taki, który nigdy **nie będzie się obracał**. W praktyce, najczęściej będzie stał w miejscu.
	- Przykłady: dekoracje, ściany, podłogi.
- Obrazek **dynamiczny** to taki, którego ruch jest nieograniczony.
	- Przykłady: postacie, itemy, bronie, skrzynki jako przeszkody na scenie, bomba.

### Artefakty z osamotnionymi pikselami
Jeżeli na obrazku **dynamicznym** potrzebujemy postawić jakiś "osamotniony" piksel który kontrastuje z resztą,  
to nie może być to *istotny* element obrazka.  

Powiedzmy, że chcemy dodać światełko do muszki broni.  
Może nas podkusić aby po prostu zostawić taki piksel:

{% include image.html file="pages/todo/pl_pages/bilmer_one_px.png" %}
Jest to jednak broń -  bronie są w grze często **obracane**.
Niestety, pixel-art ma to do siebie, że traci jakość podczas obrotu.  
Następujący artefakt może mieć miejsce:  

{% include image.html file="pages/todo/pl_pages/one_pixel_problem.png" %}

Jak widać, algorytm może czasami usunąć istotny dla nas jasny piksel.  
Rozwiązaniem tego problemu jest dodanie sąsiedniego piksela:

{% include image.html file="pages/todo/pl_pages/bilmer_two_px.png" %}

W tym przypadku, nawet gdy przy obrocie zostanie utracony jeden piksel, drugi zawsze będzie widoczny i artefakt nie będzie miał miejsca.  

Oczywiście, jeśli nagłe zniknięcie piksela nie psuje nam całokształtu wyglądu (element nie jest *istotny*), to możemy zostawić go bez takich sąsiadów.  
Problem nie aplikuje się do obrazków **sztywnych**, które z definicji nie będą nigdy obracane i nie będą miały szansy źle się wyrenderować.  

## Cieniowanie

Ze względu na rozbudowane, dynamiczne oświetlenie w grze, artysta **nie może założyć z której strony światło pada na obiekt**, czy pada w ogóle, ani też jakiego koloru jest to światło.
**To spore ograniczenie** - zostajemy z możliwością nałożenia drobnego ambient occlusion, czyli cieniowania tylko miejsc które **z każdej strony** są mało dostępne dla światła.

Idealnym wzorem do naśladowania jest laboratoryjna podłoga - mamy charakterystyczne szramy które, przez to że są wnękami, są odrobinę ciemniejsze od reszty:

{% include image.html file="pages/todo/pl_pages/floor.png" %}

Oczywiście, ta technika musi być stosowana z umiarem aby rezultat nie przypominał pillow shadingu.  

W przyszłości, to ograniczenie będzie rozwiązane przez wprowadzenie normal map - wtedy artysta będzie miał pełnię możliwości definiowania kształtu obiektu.  
Wszystkie dotąd istniejące grafiki można zatem uznać za **diffuse mapy**.

## Kolorystyka

- Bronie, zbroje i ogólnie itemy, które są "natywne" dla danej frakcji, powinny przestrzegać **kolorystyki tej frakcji**.  
	- W kolejności preferowanej częstotliwości występowania:
		- Metropolia: <span style="color:violet">fioletowy</span>, <span style="color:cyan">cyjanowy</span>, <span style="color:pink">różowy</span>, <span style="color:red">czerwony</span>.
		- Atlantyda: <span style="color:green">zielony</span>, <span style="color:yellow">żółty</span>, <span style="color:cyan">cyjanowy</span>, <span style="color:orange">pomarańczowy</span>.
		- Opór: <span style="color:brown">brązowy</span>, <span style="color:red">czerwony</span>, <span style="color:orange">pomarańczowy</span>, <span style="color:yellow">żółty</span>.
	- Ten rozkład częstotliwości jest tylko *preferowany*; można go naginać wedle potrzeby.
- Jeśli item nie wiąże się z żadną frakcją, ale jest związany z "żywiołem" (np. broń plazmowa), to powinien przestrzegać kolorystyki tego właśnie żywiołu - np. plasma gun powinien mieć w większości zielone światła.
- Jeśli obrazek jest dekoracją specifyczną dla danego środowiska, to powinien przestrzegać kolorystyki tego środowiska.
	- To już do ustalenia przez mapującego.
		- Przykładowo, wszystkie laboratoryjne podłogi i ściany mogłyby mieć kolory białe i szare, z cyjanowymi światłami.
- Jeśli item nie jest związany z żadną frakcją, ani żadnym żywiołem, ani nie jest to dekoracja specyficzna dla danego środowiska, to nie ma zasad kolorystyki.

## Kategoria: Postać

Poniżej znajduje się lista grafik i animacji które są konieczne aby stworzyć kompletne doświadczenie postaci na scenie.
Każda kolekcja utworzona z tej listy jest uniwersalna, w tym sensie, że będzie działać dla wszystkich możliwych kombinacji trzymanych przez postać itemów, broni, kapeluszy etc.  

Edytor umożliwia bardzo łatwe ustawianie metryk dla każdego obrazka postaci, np. gdzie ma głowę, plecy i dłonie.  
Dodatkowo, każdy item ma ustawiane offsety - "kotwice" - za które powinien być trzymany, np. dla broni można ustawić lokację spustu, a dla plecaka można ustawić jego wewnętrzną krawędź.  

Dzięki temu wystarczy tylko raz ustawić pozycję spustu dla broni - od tej pory będzie ona poprawnie trzymana już przez wszystkie rodzaje torsów którym poprawnie jest ustawiona pozycja dłoni.  
 
{% include important.html content="Jeden i ten sam *obrazek* może być używany przez wiele *klatek*. Jeśli artysta ustawia 20 klatek dla pełnego cyklu ruchu, może przydatnym być aby ustawić ten sam obrazek dla klatek nr 5 i 6, 10 i 11, 20 i 1." %}
{% include important.html content="Należy przestrzegać ustalanych poniżej liczb rysowanych obrazków, aby klatki poszczególnych części ciała perfekcyjnie współgrały ze sobą na ekranie." %}

{% include tip.html content="Niektóre podpunkty mogą się powtarzać co do słowa. To nie jest pomyłka, a zabieg celowy, aby uprościć pracę nad konkretną sekwencją." %}

Załóżmy bazową liczbę klatek animacji ruchu, **n = 5**.  
  
Dla każdej animacji ruchu, tj:

- Chodzące nogi.
- Chodzący tors z pustymi rękoma.
- Chodzący tors podczas trzymania karabinu, pistoletu, czegokolwiek.
- **Ale już nie tors podczas strzelania!**

Mamy następujące rozwiązania dla artysty:

- Artysta tworzy animację **n = 5** klatek w edytorze.
	- Gra **samodzielnie** kontynuuje animację puszczając ją od tyłu, tworząc cykl **2n = 10** klatek.
	- (Jeśli zaznaczono *flip_when_cycling*) Potem, gra **samodzielnie** kontynuuje animację odbijając ją w pionie i puszczając jeszcze raz, tworząc cykl **4n = 20** klatek.
- Artysta tworzy animację **2n = 10** klatek w edytorze - razem z animacją powrotną.
	- Gra zawsze puszcza animację od nowa gdy ta się skończy.
	- (Jeśli zaznaczono *flip_when_cycling*) Potem, gra **samodzielnie** kontynuuje animację odbijając ją w pionie i puszczając jeszcze raz, tworząc cykl **4n = 20** klatek.
- Artysta tworzy animację **4n = 20** klatek w edytorze - razem z animacją powrotną i animacją odbicia.
	- Gra zawsze puszcza animację od nowa gdy ta się skończy.

Gra samodzielnie wykrywa, co powinna zrobić w zależności od tego ile klatek jest dostępnych: 5, 10 lub 20.  

Brzegowe klatki mogą się powtarzać w całym cyklu **20** klatek.  
To upraszcza nam ogromnie kalkulacje, ale również nie wygląda źle.  

### Tors

{% include important.html content="Wszystkie klatki torsu powinny być **pozbawione głowy**. Ta jest dodana podczas rozgrywki. **Tors powinien wyglądać rozsądnie również bez głowy**, np. gdy zostanie odstrzelona. " %}

- **TODO: Przykłady muszą być poprawione w myśl tej zasady!**

#### Pozycjonowanie środka

Pozycja obrazka w grze jest zawsze zawieszona na *środkowym pikselu*.  
Koordynaty tego piksela określane są wzorem ``(floor(szerokość / 2), floor(wysokość / 2))``.  

Np. licząc od zera, w przypadku obrazka 3x3, jest to piksel o koordynatach (1, 1).  
W przypadku obrazka 2x2 jest to piksel w samym prawym dolnym rogu - też (1, 1).  

Aby postać nie przeskakiwała podczas gwałtownego przełączania animacji - np. strzał, chodzenie, strzał, chodzenie - należy upewnić się,  
że:

- Element wizualny, który wypada na *środkowym pikselu* pierwszej klatki **jednej** animacji, np. chodzenia...
- ...zawsze wypada na *środkowym pikselu* pierwszej klatki **drugiej** animacji, np. strzelania.

Świetnym kandydatem na taki element wizualny jest **dokładny środek głowy postaci**.  
Warto, dla ułatwienia pracy, zaznaczyć sobie ten środek czarnym pikselem - on i tak nie będzie widoczny pod głową postaci.  

Między klatkami pojedynczej animacji, środek wizualny może już trochę się przesunąć, np. naturalnym jest, że głowa może lekko odchylić się podczas strzału.

**TODO:** Dodać obrazki!

#### Tors który trzyma długie bronie

**Problem**: O ile w realistycznym chodzeniu z bronią może ona się kiwać do góry i na dół, o tyle w grze nie możemy tak jej huśtać - tutaj broń jest stale równoległa do podłoża i może się obracać tylko w jednej osi - na boki.  

Dlatego odległości między dłońmi na animacji muszą być **stałe**, aby broń nie "pływała" nam po rękach:  

{% include image.html file="pages/todo/pl_pages/plywanie.gif" %}

Naturalnym jest, że lewa dłoń będzie robiła większe ruchy od prawej, jako że lewa jest bardziej wyciągnięta do przodu.  
Oto sposób na to aby zachowywać stałą odległość między dłońmi podczas animowania, gdy chcemy aby jedna dłoń ruszała się mniej od drugiej:  

Załóżmy że czarna kreska to linia na której rysowana jest broń.  
Na zielono zaznaczono, gdzie wypada druga dłoń.  
Drugą taką samą linię zaczęto obracać wokół pierwszej dłoni. Tam, gdzie wypada teraz zielony punkt, powinna wypaść dłoń w następnej klatce - wtedy dłoń się poruszy, a odległość między dłońmi zostanie zachowana.  

Dzięki temu będzie wizualnie uzasadnione to, że broń w rzucie z lotu ptaka ma ciągle taką samą długość.  

{% include image.html file="pages/todo/pl_pages/linia_obrot.gif" %}

Najlepiej stworzyć sobie takie linie w edytorze, z kolorowymi kropkami po obu stronach, i nadzorować czy odległość ciągle wypada taka sama.

#### Lista

1. Animacja chodzenia z pustymi rękoma.  
	**Ilość klatek: n, 2n lub 4n (5, 10 lub 20).** 

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/torso_bare.png" %}

	Ta animacja będzie dodatkowo służyć jako domyślna animacja trzymania itemów, do których nie ma żadnej specjalnej animacji.  
	Istotnie, ta animacja nie wygląda źle z małymi itemami.  
	Później, jeśli będziemy mieli czas, możemy dorobić oddzielną animację na chodzenie z małymi itemami w rękach.  

	- Pierwsza klatka będzie wyświetlana dodatkowo dla **gracza który stoi w miejscu**.

2. (Opcjonalnie) Animacja chodzenia z dwiema brońmi - **akimbo**.
	- Wszystko **tak samo** jak w punkcie 1 (chodzenie z pustymi rękoma), tylko...
		- ...na obrazkach obie ręce będą wyciągnięte do przodu!
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 1 (chodzenie z pustymi rękoma).

3. Animacja chodzenia z karabinem.  
	**Ilość klatek: n, 2n lub 4n (5, 10 lub 20).** 

	Przykład poprawnej animacji:

	{% include image.html file="pages/todo/pl_pages/torso_rifle.png" %}

	- Pierwsza klatka będzie wyświetlana dodatkowo dla **gracza który stoi w miejscu**.

4. Animacja strzelania z karabinu.  
	**Ilość klatek: dowolna! (może być równa *n*, ale nie ma to znaczenia.)**

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/torso_rifle_shoot.png" %}

	- Uwaga: artysta musi **samodzielnie ustawić klatki powrotne** w edytorze.
		- To robi sens, ponieważ przy strzale ramię szybciej odskoczy niż powróci na miejsce.
	- Uwaga: pierwsza klatka strzelania zawsze powinna być **unikalna**, w szczególności: nie powinna być skopiowana z którejś klatki chodzenia.  
		- Na przykład: jeśli skopiujemy pierwszy obraz chodzenia z karabinem do pierwszej klatki strzału z karabinu...
			- ...to jeśli nasza postać strzeli podczas stania w miejscu, będziemy mieli opóźnienie od momentu strzału do jakiejkolwiek widocznej zmiany na ekranie (pierwsza klatka będzie taka sama).

5. Animacja strzelania podczas gdy w obu rękach trzymamy bronie - **akimbo**.
	- Wszystko **tak samo** jak w punkcie 4 (strzelanie z karabinu), tylko...
		- ...na obrazkach obie ręce będą wyciągnięte do przodu!
		- Rysowane obrazki powinny odwzorowywać odrzut dla **prawego (dolnego) ramienia**.
			- Lewą rękę należy utrzymać wyciągniętą do przodu (może przesunąć się nawet trochę dalej).
			- Tak, jak gdyby strzał padł wyłącznie z prawej ręki.
		- Jeżeli gra wykryje, że strzał padł z prawej ręki, to odtworzy tą animację normalnie, jak gdyby strzelało się z jednej broni.
		- Jeżeli gra wykryje, że strzał padł z lewej ręki, to **odbije animację w pionie** i odtworzy ją jak gdyby strzelało się z jednej broni.
		- Jeżeli gra wykryje że z obu broni padł strzał w krótkim czasie, to zawsze odtworzy animację dla tej ręki dla której strzał padł ostatni.
			- W chaosie strzelania z dwóch broni nie będzie to miało znaczenia czy oba ramienia odtwarzają się niezależnie.

{% include tip.html content="Pozostałe animacje torsu powtarzają wyżej opisane schematy." %}

6. Animacja chodzenia z pistoletem. 
	- Wszystko tak samo jak w punkcie 3 (chodzenie z karabinem), tylko na obrazkach będzie inna postawa.

7. Animacja strzelania z pistoletu. 
	- Wszystko tak samo jak w punkcie 4 (strzelanie z karabinu), tylko na obrazkach będzie inna postawa.

8. (Opcjonalnie) Animacja chodzenia z granatem. 
	- Wszystko tak samo jak w punkcie 3 (chodzenie z karabinem), tylko na obrazkach będzie inna postawa.
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 1 (chodzenie z pustymi rękoma).

9. (Opcjonalnie) Animacja rzucenia granatu. 
	- Wszystko tak samo jak w punkcie 4 (strzelanie z karabinu), tylko na obrazkach będzie inna postawa.
	- Ta animacja dodatkowo posłuży jako animacja wyrzucenia itema.

10. (Opcjonalnie) Animacja chodzenia z ciężkim karabinem maszynowym. 
	- Wszystko tak samo jak w punkcie 3 (chodzenie z karabinem), tylko na obrazkach będzie inna postawa.
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 3 (chodzenie z karabinem).

11. (Opcjonalnie) Animacja strzelania z ciężkiego karabinu maszynowego. 
	- Wszystko tak samo jak w punkcie 4 (strzelanie z karabinu), tylko na obrazkach będzie inna postawa.
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 4 (strzelanie z karabinu).


### Nogi

Jako, że nogi będą pod torsem, możemy być skłonni do tego aby nie wypełnić detalicznie środkowych elementów obrazków nóg.  
Jednakże, ponieważ nogi mogą być obracane dowolnie, musimy założyć że każdy piksel może nagle stać się widoczny.  
Obrazki nóg muszą być ciągłe, nie mogą być w środku puste ani mieć w środku przerywających krawędzi.  
Jako bonus, to sprawi, że nasze nogi będą kompatybilne również z "chudszymi" torsami.  

Artysta **nie rysuje obrazka dla pierwszej klatki nóg**, ponieważ ta jest zawsze niewidoczna pod stojącym torsem.  
Trzeba dodać w edytorze jakiś pusty obrazek na pierwszą klatkę.  

**TODO:** Poprawić animacje nóg w myśl tej zasady.

#### Lista


1. Animacja nóg podczas chodzenia **równoległego** do kierunku patrzenia.  
	{% include tip.html content="Oprócz tego, że nie rysujemy pierwszej klatki, procedura dla tej animacji jest analogiczna do animacji chodzenia z pustymi rękoma." %}
	**Ilość klatek: n, 2n lub 4n (5, 10 lub 20).** 

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/legs_forward.png" %}

2. Animacja nóg podczas chodzenia **prostopadłego** do kierunku patrzenia.  
	**Ilość klatek: 2n lub 4n (10 lub 20).** 
	Długość kompletnego cyklu ruchu: **4n = 20** *klatek*.  

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/legs_strafe.png" %}

	- Uwaga: w przeciwieństwie do animacji chodzenia prosto, artysta *zawsze* samodzielnie rysuje animację powrotną.
		- Przy strafowaniu wygląda to na konieczne.

### Głowa

Jeden sprite.  
Najlepiej, jeśli centrum obrazka głowy wypada dokładnie w środku głowy.  

## Kategoria: Bronie

### Wystrzał

Jeżeli dodajemy do broni animację wystrzału, to nie powinna ona opierać się wyłącznie na drobnych zmianach koloru.  
Gra jest bardzo dynamiczna, dlatego animacja powinna:
- w jakiś sposób zmieniać kształt broni, lub
- rozszerzać istniejące elementy ujawniając jaskrawe światło pod spodem, lub
- dodawać kompletnie nowy element, najlepiej też jaskrawy, lub
- mieszać powyższe techniki.

To jest przykład animacji, która będzie praktycznie niezauważalna w całym chaosie rozgrywki:

{% include image.html file="pages/todo/pl_pages/vindicator.gif" %}

To jest za to idealny przykład animacji która zmienia kształt broni i będzie bardzo zauważalna, osiągając zamierzony efekt:

{% include image.html file="pages/todo/pl_pages/gral3.gif" %}


## Kategoria: Dekoracje

- Może być kuszące, aby dużym dekoracjom robić animacje na całym ich obszarze. Niestety musimy liczyć się z ograniczoną przestrzenią atlasu.


## Konwencje nazewnictwa

- Ogólnie: ``[co_to_jest]_[indeks_klatki].png``
	- Torsy: ``[frakcja]_torso_[trzymana_rzecz]_[czynność]_[indeks].png``, np.:
		- ``resistance_torso_rifle_shoot_1.png`` - Pierwsza klatka animacji strzału z karabinu dla żołnierza Oporu.
		- ``metropolis_torso_heavy_walk_2.png`` - Druga klatka animacji chodzenia z ciężkim karabinem dla żołnierza Metropolii.
		- ``metropolis_torso_akimbo_walk_2.png`` - Druga klatka animacji chodzenia akimbo dla żołnierza Metropolii.
		- ``metropolis_torso_bare_walk_1.png`` - Pierwsza klatka animacji chodzenia bez niczego dla żołnierza Metropolii.
	- Głowy: ``[frakcja]_head.png``
		- Np.: ``resistance_head.png``
	- Naboje: 
		- Kartridż: ``[rodzaj]_charge.png``
		- Łuska: ``[rodzaj]_shell.png``
		- Pocisk: ``[rodzaj]_round.png``
		- Przykładowy zestaw:
			- ``steel_charge.png``
			- ``steel_shell.png``
			- ``steel_round.png``

## Wskazówki ogólne

- Jeśli rysujemy jakieś urządzenie elektroniczne...
	- Warto zrobić wyświetlacze w oddzielnym obrazku/animacji, **w kolorach czarno-białych**.
		- Wtedy z poziomu gry da się zmienić kolory samego wyświetlacza, np. na czerwony który sygnalizuje awarię lub zniszczenie.
