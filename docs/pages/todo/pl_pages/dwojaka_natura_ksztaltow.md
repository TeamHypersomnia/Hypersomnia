---
title: Dwojaka natura kształtów
hide_sidebar: true
permalink: dwojaka_natura_ksztaltow
summary: Takie tam.
---

Zastanawiam się...  
Obiekt który ma ciało fizyczne + sprite (skrzynka, gracz, ściana), ma **dwa** dane kształty:

1. Kształt ciała fizycznego
	- pozwalamy na dowolne wierzchołki
2. Kształt obrazka który go reprezentuje
	- w przypadku sprita prostokąt - bo to po prostu obrazek
	- ale mamy też support dla renderowania dowolnych, oteksturowanych wielokątów

W większości przypadków powyższe dwa będą się pokrywały, np. prosta kwadratowa skrzynka albo prostokątna ściana.  
Jeśli jednak obiekt będzie nieregularny, powiedzmy skrzynka z uchwytami na brzegach, to chcemy dać jej odrobinę mniejsze ciało fizyczne by gracz lepiej "dotykał" ten obiekt.  
Wtedy mamy dwie różniące się dane.

## Problem: najeżdżanie myszką, zaznaczanie obiektów i snapowanie do grida.

- Jeśli obiekt jest tylko niefizyczną dekoracją, to nie ma problemu, zawsze zaznaczamy/snapujemy tylko kształt obrazkowej reprezentacji tej dekoracji.
	- Np. podłoga, napis.
- Jeśli obiekt jest fizyczny i ma obrazkową reprezentację (skrzynia, ściana), to zawsze zaznaczamy/snapujemy kształt fizyczny, ponieważ on jest... ważniejszy.
	- A już szczególnie przy snapowaniu do grida.
	- Problem: jeśli ciało fizyczne jest mniejsze od obrazka, to mimo że myszka będzie już trochę nad obrazkiem, to najechanie myszką nie zostanie wykryte.
		- Trzeba najechać wewnątrz ciała fizycznego.
		- To na razie nie będzie tragedią (nie będzie często się zdarzać), a poprawienie tego teraz będzie dla mnie ogromnym bólem w dupie
		- Przyszłe rozwiązanie: **jeśli ciało fizyczne nie pokrywa się z obrazkiem**, to dodatkowo nad obrazkiem rysujemy przerywaną linię tam gdzie jest ciało fizyczne, aby było widać gdzie trzeba najechać myszą

## Problem: edycja jednego lub drugiego kształtu

- ``v`` - tryb ogólnej edycji kształtu:
	- Dla niefizycznej dekoracji która jest obrazkiem - nie robimy nic.
		- Nie będziemy zmieniać rozmiaru obrazka bo to nie ma sensu z pixelartami.
	- Dla niefizycznej dekoracji która jest wielokątem - modyfikujemy ten wielokąt.
		- Nawet jeśli to będzie pixel-art, można ustawić takie koordynaty uv dla tesktury, aby obrazek nie był rozciągnięty, tylko po prostu pocięty wierzchołkami obiektu.
	- Dla fizycznego obiektu którego obrazkową reprezentacją jest sprite, a więc zwykły obrazek, **edytuje kształt jego fizycznej reprezentacji**.
		- Nadal nie pozwala na modyfikację obrazka!
	- Dla fizycznego obiektu którego obrazkową reprezentacją jest wielokąt (np. nieregularna ściana jaskinii), **edytuje kształt jego obrazkowej reprezentacji**, ale...
		- ...każde przeciągnięcie wierzchołka albo rozciągnięcie rozmiaru - **automatycznie uaktualnia kształt fizyczny**, aby się zgadzał z nowym kształtem obrazkowej reprezentacji.
		- Podobnie jak z niefizycznymi dekoracjami, możemy na to nałożyć pixel-art w rozmiarze 1:1.

### Przykładowy workflow, 1

- Wrzucamy nowego entita który jest trójkątną paką ze spritem 32x32.
	- Generowane jest domyślne ciało fizyczne - prostokąt 32x32.
- Wciskamy ``v`` i przeciągamy myszką wierzchołki aby kształt ciała fizycznego zgadzał się z tym co jest na obrazku.
- Wciskamy ``v`` jeszcze żeby wyjść - zrobione.
	- Obrazek pozostał nietknięty.
		- Nie ma sensu powiększać albo pomniejszać zwykłego obrazka - będą to pixel-arty.

### Przykładowy workflow, 2

- Wrzucamy nowego entita który jest interaktywnym obiektem ze spritem 32x32.
	- Generowane jest domyślne ciało fizyczne - prostokąt 32x32.
- Wciskamy ``v`` i zmniejszamy trochę rozmiar ciała fizycznego (np. klawiszami - oraz + które pozwolą na dokładną zmianę o konkretną ilość pikseli)
	- Teraz gracz lepiej się będzie dotykał z tym obiektem.
- Wciskamy ``v`` jeszcze żeby wyjść - zrobione.
	- Obrazek pozostał nietknięty.
		- Nie ma sensu powiększać albo pomniejszać zwykłego obrazka - będą to pixel-arty.

### Przykładowy workflow, 3

- Wrzucamy nowego entita o typie "wielokątne ciało" które ma teksturę ściany nieregularnej jaskinii, 256x256.
	- Chcemy zrobić coś takiego:
		- {% include image.html file="http://3.bp.blogspot.com/_LCX1wUXnhKo/TVLjzmrUlzI/AAAAAAAAA0c/5mlguVmeJIA/s1600/ctf_scourge2_mid.png" %}
- Zaczynamy od prostokąta o rozmiarze źródłowego obrazka.
- Wciskamy ``v`` i modyfikujemy wierzchołki tego wielokąta - tworzymy jaskinię.
	- Z każdym poruszeniem, dodaniem, usunięciem wierzchołka, ciało fizyczne aktualizuje się aby się pokrywało 1:1 z wielokątem który będzie renderowany.
		- Dla takiego wielokąta nie ma sensu, aby ciało fizyczne nie zgadzało się w 1:1 z obrazkową reprezentacją.
- Mamy opcję aby zmapować obrazek na wierzchołki tak, aby został nałożony w rozmiarze 1:1 - gdyby to miał być pixel-art.
- Wciskamy ``v`` jeszcze raz żeby wyjść - zrobione.
