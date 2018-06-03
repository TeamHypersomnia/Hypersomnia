---
title: Art style guide (GFX)
tags: [scratchpads_pl]
hide_sidebar: true
permalink: art_style_guide_gfx
summary: Założenia dotyczące grafiki.
---

## Kontur

- Preferujemy twardy, czarny obrys grubości jednego piksela na zewnątrz każdego obrazka.
	- Wyjątek: krawędze wzdłuż których obrazek układany jest w kafelki, np. na podłogach lub ścianach.
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

## Neon mapy

## Animacje

### Postać

### Bronie

#### Wystrzał

Jeżeli dodajemy do broni animację wystrzału, to nie powinna ona opierać się wyłącznie na drobnych zmianach koloru.  
Gra jest bardzo dynamiczna, dlatego animacja powinna:
- w jakiś sposób zmieniać kształt broni, lub
- rozszerzać istniejące elementy ujawniając jaskrawe światło pod spodem, lub
- dodawać kompletnie nowy element, najlepiej też jaskrawy, lub
- mieszać powyższe techniki.

To jest przykład animacji, która będzie praktycznie niezauważalna w całym chaosie rozgrywki:

{% include image.html file="pages/todo/pl_pages/vindicator.gif" %}

To jest za to przykład animacji która zmienia kształt broni i będzie bardzo zauważalna, osiągając zamierzony efekt:

{% include image.html file="pages/todo/pl_pages/gral3.gif" %}


### Dekoracje

- Może być kuszące, aby dużym dekoracjom robić animacje na całym ich obszarze. Niestety musimy liczyć się z ograniczoną przestrzenią atlasu.

## Wskazówki ogólne

- Jeśli rysujemy jakieś urządzenie elektroniczne, warto zrobić wyświetlacze w oddzielnym obrazku, aby z poziomu gry dało się zmienić kolory samego wyświetlacza, np. na czerwony - po zniszczeniu lub jakiejś awarii.
