---
title: Kontemplacje
hide_sidebar: true
permalink: considerations_pl
summary: Takie tam.
---

## Dwojaka natura kształtów

Zastanawiam się...  
Obiekt który ma ciało fizyczne + sprite (skrzynka, gracz, ściana), ma **dwa** dane kształty:

1. Kształt ciała fizycznego
	- pozwalamy na dowolne wierzchołki
2. Kształt obrazka który go reprezentuje
	- w przypadku sprita prostokąt - bo to po prostu obrazek
	- ale mamy też support dla renderowania dowolnych, oteksturowanych wielokątów

W większości przypadków powyższe dwa będą się pokrywały, np. prosta kwadratowa skrzynka albo prostokątna ściana.  
Jeśli jednak obiekt będzie nieregularny, powiedzmy skrzynka z uchwytami na brzegach, to chcemy dać jej odrobinę mniejsze ciało fizyczne by gracz lepiej "dotykał" ten obiekt.  

Problem pojawia się, który kształt edytor powinien rozpatrywać przy:

- Najeżdżaniu myszką i zaznaczaniu obiektów.
- **Snapowaniu ich do grida, przy poruszaniu ich.**
- Edycji kształtów.

### Moje myśli

- Jeśli obiekt jest tylko niefizyczną dekoracją, to nie ma problemu, zawsze zaznaczamy/snapujemy tylko kształt obrazkowej reprezentacji tej dekoracji.
	- Np. podłoga, napis.
- Jeśli obiekt jest fizyczny i ma obrazkową reprezentację (skrzynia, ściana), to zawsze zaznaczamy/snapujemy kształt fizyczny, ponieważ on jest... ważniejszy.
	- A już szczególnie przy snapowaniu do grida.
	- Problem: jeśli ciało fizyczne jest mniejsze od obrazka, to mimo że myszka będzie już trochę nad obrazkiem, to najechanie myszką nie zostanie wykryte.
		- Trzeba najechać wewnątrz ciała fizycznego.
		- To na razie nie będzie tragedią (nie będzie często się zdarzać), a poprawienie tego teraz będzie dla mnie ogromnym bólem w dupie
		- Przyszłe rozwiązanie: **jeśli ciało fizyczne nie pokrywa się z obrazkiem**, to dodatkowo nad obrazkiem rysujemy przerywaną linię tam gdzie jest ciało fizyczne, aby było widać gdzie trzeba najechać myszą

#### Dwa tryby edycji kształtu
- ``v`` - tryb ogólny.
	- Dla niefizycznej dekoracji - luzik - edytuje kształt jej obrazkowej reprezentacji.
	- Dla fizycznego obiektu, edytuje kształt jego obrazkowej reprezentacji, ale... 
		- ...każde przeciągnięcie wierzchołka albo rozciągnięcie rozmiaru - **automatycznie uaktualnia kształt fizyczny**, aby się zgadzał z nowym kształtem obrazkowej reprezentacji.
- ``shift+v`` - tryb, gdzie zmieniamy kształt jedynie fizycznych reprezentacji każdego obiektu.

### Przykładowy workflow

- Wrzucamy nowego entita który jest skrzynką 64x64.
- Wciskamy ``v`` i rozszerzamy go dwukrotnie do 128x128 bo chcemy żeby był wielgachny.
	- W tym momencie ciało fizyczne też powiększyło się do rozmiaru 128x128.
- Wciskamy ``shift+v`` i trochę zmniejszamy ciało fizyczne, żeby gracz lepiej się z nią dotykał.
	- W tym momencie obrazek zostaje nietknięty - 128x128 - a ciało fizyczne zmieniliśmy na powiedzmy prostokąt 120x120.
		- Albo na w ogóle trochę nieregularny kształt.
- Zrobiliśmy co chcemy!
