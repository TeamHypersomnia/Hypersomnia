---
title: The masterplan detail
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: Detailed explanations for the decisions made in masterplan.md
---

- Wszystkie mozliwe pliki odzyskiwania trzymamy obok project.json. Czemu?
	- W razie paniki bedzie od razu widac ze cos ocalało (zamiast schowane w .cache)
	- Rootowy folder to dobre miejsce dla tych paru waznych rzeczy. 
		- I tak tam nic nie bedzie oprocz pliku json i folderow gfx/sfx/scripts wiec byloby pusto

- Przy przesyłaniu raczej bedziemy uzywac whitelista niz blacklista do ignorowania - project.resources.json
	- Będziemy w stanie stwierdzic ktore pliki sa potrzebne
	- To bedzie oczywiste z contentu jsona tez
	- Obczaj ze nawet skryptom bedziemy dawac w pliku w jsonie jako input liste plikow do wczytania
		- zeby im nie eksponowac i/o calego i bedzie latwiej zsandboxowac to
	- Takze projekt jak najbardziej wie co mu bedzie potrzebne
	- wiec nie ma sie co martwic o "tancowanie z ignorami" albo czy cache sie wysyla itp

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
