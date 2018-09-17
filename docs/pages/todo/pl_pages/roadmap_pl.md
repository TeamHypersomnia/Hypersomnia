---
title: Roadmap (PL)
tags: [planning]
hide_sidebar: true
permalink: roadmap_pl
summary: Drobny scratchpad planów
---

## Roadmap

W kolejności:

- Bomb mode
	- GUI: Buy menu
- Edytor
    - Wskrzeszenie odtwarzania rozgrywki do testów determinizmu (będzie konieczne do networkingu)
- Dekoracje
    - Statyczne dymki na scenie, np. z wiatraków
- Port powrotny na Windowsa
    - Zainstaluję Windows 10 i na nim spróbuję zbudować.
    - Q&A
        - Testowanie edytora razem z @Spicmir 
		- Kooperacja w tworzeniu mapy laboratorium
- Networking
    - W międzyczasie @Spicmir rzeźbi mapkę laba - przy networkingu nie będę dotykał już struktur danych gry aby mapa była kompatybilna
        - W razie czego zatrzymam się na chwilę i sklepię lua import/export żeby dało się upgrade'ować mapy
	- Fixy do determinizmu w Box2D
- Dalsze poprawki do gameplayu
	- (Low prio) Dashing
	- (Low prio) Nawalanie gołymi rękoma? Co jak zabraknie nam ammo i many? Całkiem możliwe, bo postacie mogą być bardzo żywotne
- Ogarnięcie od strony społecznościowej
    - Nagra się porządne podsumowanie z moim komentarzem głosowym, tak jak pierwsze dwa.
    - Wtedy gra będzie już poniekąd grywalna więc trzeba będzie:
        - Poprawić CI żeby wszystko wyglądało ładnie na zielono
        - Zaktualizować wiki
        - Porobić paczki
        - Postawić testowy serwer 24/7 dla żądnych testów przechodniów internetowych
- Ogarnięcie cross-platform networkingu
	- Trzeba będzie zmienić silnik na PlayRho który jest portem Box2D z fixed-point liczbami
	- Zamiana klas z <random> na własne generatory, jeśli okażą się niekonsystentne między platformami
	- W przeciwnym wypadku linuksowcy będą mogli grać tylko z linuksowcami, a windowsiarze tylko z windowsiarzami.
		- Pytanie pomocznicze: czy to wada?
- Dalej, kto wie...

## Roadmap (tymczasowo porzucone)

- Ścieżki

## Roadmap (odfajkowane)

- Edytor
	- Specyfikowanie offsetów wypadania łusek per-obrazek
- Poprawki do gameplayu
	- Jakakolwiek forma przeładowywania broni
	- Automatyczne załadowywanie pocisku do komory gdy jedna ręka trzyma niezaładowaną broń a druga jest pusta
- Bomb mode
	- Podział na połowy
		- Limit rund
		- Strzałka z napisem "Winner!" na koniec
	- GUI: Wybór drużyny
- Ikonki obok nicka w scoreboardzie (def, paka, dead)
- Defuser kit
- Fix: Transfer przypisanych itemów do GUI w hotbarze, przy starcie nowej rundy
- Dekoracje
    - Animacje na scenie
    - Dźwięki przyklejone do obiektów
        - Np. szum komputerów, szum wiatraków, szum chłodni
    - Metadane dla rozgrywki
        - Spawnpointy
        - Obszar dla kładzenia paki
    - Do testbeda: akwarium z rybami
