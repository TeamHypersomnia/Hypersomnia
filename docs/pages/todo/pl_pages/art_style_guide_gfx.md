---
title: Art style guide (GFX)
tags: [scratchpads_pl]
hide_sidebar: true
permalink: art_style_guide_gfx
summary: Założenia dotyczące grafiki.
---

## Nomenklatura

- Obrazek **sztywny** to taki, który nigdy **nie będzie się obracał**. W praktyce, najczęściej będzie stał w miejscu.
	- Przykłady: dekoracje, ściany, podłogi.
- Obrazek **dynamiczny** to taki, którego ruch jest nieograniczony.
	- Przykłady: postacie, itemy, bronie, skrzynki jako przeszkody na scenie, bomba.

## Wskazówki ogólne

## Cieniowanie

Ze względu na rozbudowane, dynamiczne oświetlenie w grze, artysta **nie może założyć z której strony na światło pada na obiekt**, czy pada w ogóle, ani też jakiego koloru jest to światło.
**To spore ograniczenie** - zostajemy z możliwością nałożenia drobnego ambient occlusion, czyli cieniowania tylko miejsc które **z każdej strony** są mało dostępne dla światła.

Idealnym wzorem do naśladowania jest laboratoryjna podłoga - mamy charakterystyczne szramy które, przez to że są wnękami, są odrobinę ciemniejsze od reszty:

{% include image.html file="pages/todo/pl_pages/floor.png" %}

Oczywiście, ta technika musi być stosowana z umiarem aby rezultat nie wyglądał jak pillow shading.  

W przyszłości, to ograniczenie będzie rozwiązane przez wprowadzenie normal map - wtedy artysta będzie miał pełnię możliwości definiowania kształtu obiektu.
W praktyce, wszystkie dotąd istniejące grafiki to **diffuse mapy**.

## Kolorystyka

- Bronie, zbroje i ogólnie itemy, które są "natywne" dla danej frakcji, powinny przestrzegać **kolorystyki tej frakcji**.  
	- W kolejności preferowanej częstotliwości występowania:
		- Metropolia: <span style="color:violet">fioletowy</span>, <span style="color:cyan">cyjanowy</span>, <span style="color:red">czerwony</span>.
		- Atlantyda: <span style="color:green">zielony</span>, <span style="color:yellow">żółty</span>, <span style="color:cyan">cyjanowy</span>, <span style="color:orange">pomarańczowy</span>.
		- Opór: <span style="color:brown">brązowy</span>, <span style="color:red">czerwony</span>, <span style="color:orange">pomarańczowy</span>, <span style="color:yellow">żółty</span>.
	- Ten rozkład częstotliwości jest tylko *preferowany*; można go naginać wedle potrzeby.
- Jeśli item nie wiąże się z żadną frakcją, ale jest związany z "żywiołem" (np. broń plazmowa), to powinien przestrzegać kolorystyki tego właśnie żywiołu - np. plasma gun powinien mieć w większości zielone światła.
- Jeśli obrazek jest dekoracją specifyczną dla danego środowiska, to powinno pzestrzegać kolorystyki tego środowiska.
	- To już do ustalenia przez mapującego.
		- Przykładowo, laboratoryjne podłogi i ściany mogłyby mieć kolory białe i szare z cyjanowymi światłami.
- Jeśli item nie jest związany z żadną frakcją, ani żadnym żywiołem, ani nie jest to dekoracja specyficzna dla danego środowiska, to nie ma zasad kolorystyki.

## Animacje

### Postać


