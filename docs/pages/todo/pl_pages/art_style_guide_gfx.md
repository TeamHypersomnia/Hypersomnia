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
	- Zależy nam na tym aby obiekty się wyróżniały.
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
Za pomocą tych właśnie zmiennych gra poprawnie oblicza relatywne pozycjonowanie itemów do obrazka postaci.  

{% include important.html content="Jeden i ten sam *obrazek* może być używany przez wiele *klatek*." %}
{% include important.html content="Należy przestrzegać ustalanych poniżej liczb rysowanych obrazków, aby klatki poszczególnych części ciała perfekcyjnie współgrały ze sobą na ekranie." %}

{% include tip.html content="Niektóre podpunkty mogą się powtarzać co do słowa. To nie jest pomyłka, a zabieg celowy, aby uprościć pracę nad pojedynczą sekwencją." %}

Załóżmy bazową liczbę klatek animacji ruchu, **n = 5**.  
  
### Tors

{% include important.html content="Wszystkie klatki torsu powinny być **pozbawione głowy**. Ta jest dodana podczas rozgrywki. **Tors powinien wyglądać rozsądnie również bez głowy**, np. gdy zostanie odstrzelona. " %}

- **TODO: Przykłady muszą być poprawione w myśl tej zasady!**

#### Lista

1. Animacja chodzenia z pustymi rękoma.  
	Ilość rysowanych *obrazków*: **n = 5** lub **2n = 10**.  
	Długość kompletnego cyklu ruchu: **4n = 20** *klatek*.  

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/torso_bare.png" %}

	- Gra **samodzielnie** kontynuuje animację puszczając ją od tyłu...
		- ...a potem **odbijając ją w pionie** i odtwarzając jeszcze raz, tworząc **pełny cykl** chodzenia na dwóch nogach.
		- Pierwsza i ostatnia klatka odtwarza się dwa razy. To upraszcza nam ogromnie kalkulacje, ale również nie wygląda źle.
	- **Alternatywne rozwiązanie**: Dla każdej animacji, edytor eksponuje opcję *has_backward_frames*.
		- To pozwala artyście samodzielnie narysować animację powrotną.
		- Wtedy artysta musi narysować **2n - 1 = 9** obrazków do animacji chodzenia.
			- W edytorze: dziesiąta klatka powinna być dodana do edytora z obrazka pierwszej, aby cykl kończył się na takim samym obrazku, na którym zaczął.
			- Późniejsze odbicie w pionie i tak jest pozostawione grze.
	- Pierwsza klatka będzie wyświetlana dodatkowo dla **gracza który stoi w miejscu**.
	- Brak wymagań co do symetrii.

2. (Opcjonalnie) Animacja chodzenia z dwiema brońmi - **akimbo**.
	- Wszystko **tak samo** jak w punkcie 1 (chodzenie z pustymi rękoma), tylko...
		- ...na obrazkach obie ręce będą wyciągnięte do przodu!
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 1 (chodzenie z pustymi rękoma).

3. Animacja chodzenia z karabinem.  
	Ilość rysowanych obrazków: **n = 5**  
	Długość kompletnego cyklu ruchu: **2n = 10** *klatek*.  

	**TODO:** Gdy będziemy mieli już solidne animacje wszystkich postaw, możemy zwiększyć ilość obrazków (i klatek) dla tej animacji.

	Przykład poprawnej animacji:

	{% include image.html file="pages/todo/pl_pages/torso_rifle.png" %}

	Wszystkie animacje postawy z karabinem powinny odwzorowywać trzymanie broni tak, gdybyśmy strzelali **z biodra**.

	- Gra **samodzielnie** kontynuuje animację puszczając ją od tyłu...
		- ...ale już nie odbija jej w pionie na kolejny cykl. W tym wypadku to nie ma sensu.
	- **Alternatywne rozwiązanie**: Dla każdej animacji, edytor eksponuje opcję *has_backward_frames*.
		- To pozwala artyście samodzielnie narysować animację powrotną.
		- Wtedy artysta musi narysować **2n - 1 = 9** obrazków do animacji chodzenia.
			- W edytorze: dziesiąta klatka powinna być dodana do edytora z obrazka pierwszej, aby cykl kończył się na takim samym obrazku, na którym zaczął.
	- Pierwsza klatka będzie wyświetlana dodatkowo dla **gracza który stoi w miejscu**.
	- Brak wymagań co do symetrii.

4. Animacja strzelania z karabinu.  
	Ilość rysowanych obrazków: **x = dowolne!** (może być równa **n**, ale nie ma to znaczenia.)  
	Długość wynikowego cyklu: **2x**  

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/torso_rifle_shoot.png" %}

	Wszystkie animacje postawy z karabinem powinny odwzorowywać trzymanie broni tak, gdybyśmy strzelali **z biodra**.

	- Gra **samodzielnie** kontynuuje animację puszczając ją od tyłu.
	- **Alternatywne rozwiązanie**: Dla każdej animacji, edytor eksponuje opcję *has_backward_frames*.
		- To pozwala artyście samodzielnie narysować animację powrotną.
			- Ilość klatek pozostaje dowolna zarównież w tym przypadku.
			- Ostatnia klatka nie musi być dodana z pierwszego obrazka.
	- Brak wymagań co do symetrii.


5. Animacja strzelania podczas gdy w obu rękach trzymamy bronie - **akimbo**.
	- Wszystko **tak samo** jak w punkcie 3 (strzelanie z karabinu), tylko...
		- ...na obrazkach obie ręce będą wyciągnięte do przodu!
		- Rysowane obrazki powinny odwzorowywać odrzut dla **prawego (dolnego) ramienia**.
			- Lewą rękę należy utrzymać wyciągniętą do przodu (może przesunąć się nawet trochę dalej).
			- Tak, jak gdyby strzał padł wyłącznie z prawej ręki.
			- **W szczególności, nie ma wymagań co do symetrii.**
		- Jeżeli gra wykryje, że strzał padł z prawej ręki, to odtworzy tą animację normalnie, jak gdyby strzelało się z jednej broni.
		- Jeżeli gra wykryje, że strzał padł z lewej ręki, to **odbije animację w pionie** i odtworzy ją jak gdyby strzelało się z jednej broni.
		- Jeżeli gra wykryje że z obu broni padł strzał w krótkim czasie, to zawsze odtworzy animację dla tej ręki dla której strzał padł ostatni.
			- W chaosie strzelania z dwóch broni nie będzie to miało znaczenia czy oba ramienia odtwarzają się niezależnie.

{% include tip.html content="Pozostałe animacje torsu powtarzają wyżej opisane schematy." %}

6. Animacja chodzenia z pistoletem. 
	- Wszystko tak samo jak w punkcie 3 (chodzenie z karabinem), tylko na obrazkach będzie inna postawa.

7. Animacja strzelania z pistoletu. 
	- Wszystko tak samo jak w punkcie 4 (strzelanie z karabinu), tylko na obrazkach będzie inna postawa.

8. (Opcjonalnie) Animacja chodzenia z ciężkim karabinem maszynowym. 
	- Wszystko tak samo jak w punkcie 3 (chodzenie z karabinem), tylko na obrazkach będzie inna postawa.
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 3 (chodzenie z karabinem).

9. (Opcjonalnie) Animacja strzelania z ciężkiego karabinu maszynowego. 
	- Wszystko tak samo jak w punkcie 4 (strzelanie z karabinu), tylko na obrazkach będzie inna postawa.
	- W przypadku braku tej animacji nie będzie tragedii jeśli zastąpi ją animacja nr 4 (strzelanie z karabinu).


### Nogi

Jako, że nogi będą pod torsem, możemy być skłonni do tego aby nie wypełnić detalicznie środkowych elementów obrazków nóg.  
Jednakże, ponieważ nogi mogą być obracane dowolnie, musimy założyć że każdy piksel może nagle stać się widoczny.  
Obrazki nóg muszą być ciągłe, nie mogą być w środku puste ani mieć w środku przerywających krawędzi.  
Jako bonus, to sprawi, że nasze nogi będą kompatybilne również z "chudszymi" torsami.  

**TODO:** Poprawić animacje nóg w myśl tej zasady.

1. Animacja nóg podczas chodzenia **równoległego** do kierunku patrzenia.  
	{% include tip.html content="Oprócz tego, że nie rysujemy pierwszej klatki, procedura dla tej animacji jest analogiczna do animacji chodzenia z pustymi rękoma." %}
	Ilość rysowanych *obrazków*: **n - 1 = 4** lub **2(n - 1) = 8**.  
	Długość kompletnego cyklu ruchu: **4n = 20** *klatek*.  

	Artysta **nie rysuje obrazka dla pierwszej klatki**, ponieważ ta jest zawsze niewidoczna pod stojącym torsem.

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/legs_forward.png" %}

	- Gra **samodzielnie** kontynuuje animację puszczając ją od tyłu...
		- ...a potem **odbijając ją w pionie** i odtwarzając jeszcze raz, tworząc **pełny cykl** chodzenia na dwóch nogach.
		- Pierwsza i ostatnia klatka odtwarza się dwa razy. To upraszcza nam ogromnie kalkulacje, ale również nie wygląda źle.
	- **Alternatywne rozwiązanie**: Dla każdej animacji, edytor eksponuje opcję *has_backward_frames*.
		- To pozwala artyście samodzielnie narysować animację powrotną.
		- Wtedy artysta musi narysować **2(n - 1) = 8** obrazków do animacji chodzenia.
			- W edytorze: pierwsza i dziesiąta klatka powinny być puste.
			- Późniejsze odbicie w pionie i tak jest pozostawione grze.
	- Brak wymagań co do symetrii.

2. Animacja nóg podczas chodzenia **prostopadłego** do kierunku patrzenia.  
	Ilość rysowanych *obrazków*: lub **2(n - 1) = 8**.  
	Długość kompletnego cyklu ruchu: **4n = 20** *klatek*.  

	Artysta **nie rysuje obrazka dla pierwszej i ostatniej klatki**, ponieważ ta jest zawsze niewidoczna pod stojącym torsem.

	Przykład poprawnej animacji:  

	{% include image.html file="pages/todo/pl_pages/legs_strafe.png" %}

	- Uwaga: w przeciwieństwie do animacji chodzenia prosto, artysta *zawsze* samodzielnie rysuje animację powrotną.
		- Przy strafowaniu wygląda to na konieczne.
		- W edytorze: pierwsza i dziesiąta klatka powinny być puste.
	- Brak wymagań co do symetrii.

#### Lista

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


## Wskazówki ogólne

- Jeśli rysujemy jakieś urządzenie elektroniczne...
	- Warto zrobić wyświetlacze w oddzielnym obrazku/animacji, **w kolorach czarno-białych**.
		- Wtedy z poziomu gry da się zmienić kolory samego wyświetlacza, np. na czerwony który sygnalizuje awarię lub zniszczenie.
