---
title: Obrazki w edytorze
hide_sidebar: true
permalink: obrazki_w_edytorze
summary: Takie tam.
---

Edytor obsługuje jeden format obrazka: ``.png``.

Obrazki na dysku będą używane do:

- Ustawiania wyglądu dla invariantu sprite'a (czyli po prostu sprite'om na scenie).
- Ustawiania wyglądu dla invariantu polygon'a (czyli po prostu nieregularnym obiektom na scenie).
- Ustawiania wyglądu dla elementów GUI, np. ikonek spelli, perków, pasków życia, many i świadomości.
- Składania klatek animacji. Animacje zaś będą używane do:
	- Stawiania animacji na scenie  - a jakże!
	- Przypisywaniu ich bardziej skomplikowanym obiektom w różnych miejscach, jak np. gracz.
	- Definiowaniu efektów cząsteczkowych - cząsteczki też mogą być (i niektóre testowe już są) animacjami.

## Lokacja plików

Każdy projekt będzie czerpał pliki obrazków z dokładnie dwóch folderów (i wszystkich ich podfolderów):

- ``hypersomnia/official/gfx`` - oficjalny, "kanoniczny" zbiór grafik. Szczegóły:
	- Wbudowane sceny testowe mają wrzucone obrazki **tylko z tego folderu**.
	- W fazie MMO znajdą się tam wszystkie obrazki dla oficjalnego serwera. 
	- W fazie fabularnej znajdą się tam wszystkie obrazki potrzebne fabule. 
	- Wiele społecznościowych map będzie mogło korzystać z grafik z tego folderu - i nie będą musiały trzymać swoich duplikatów.
		- Będą na tyle dobre że ludzie chętnie będą je brali!
- ``Sciezka/Do/Projektu/NazwaProjektu/gfx`` - zbiór grafik specyficznych dla tego projektu.

Podfoldery znajdujące się w powyższych dwóch folderach mogą być zorganizowane już dowolnie.

Z dźwiękami będziemy mieli ten sam schemat, tylko że zamiast folderu ``gfx`` będziemy mieli folder ``sfx``.

## W edytorze

Pomysł jest taki.
Gdziekolwiek w edytorze będziemy mieli do wyboru obrazek, będziemy mieli takie oto combo:

{% include image.html file="pages/todo/pl_pages/przykladowy_combo.png" %}

Będzie to miniaturkowy hierarchiczny widok na strukturę obu folderów: oficjalnego i specyficznego dla projektu.  

- To combo zawsze będzie aktualne z tym co obecnie jest na dysku.
- Obecny obrazek zawsze będzie podświetlony na niebiesko.
- Zamknięte combo zawsze będzie miało napis formatu ``nazwa_pliku.png (sciezka/do/pliku)``.
- Otwarte elementy drzewa będą zapamiętane (ImGui to ogarnia z buta).
	- Dodatkowo zawsze po otwarciu comba będziemy rozwijać te foldery w których jest obecnie wybrany plik - aby natychmiast był widoczny.

### Okno "Images"

Dodatkowo wprowadzone zostanie okno nazwane "Images".  
Znajdzie się tam to samo co w combo boxie z wyborem obrazka, z dwiema różnicami:  

1. **Pojawią się tam wyłącznie te obrazki które są gdzieś używane w edytorze.**
	- (Opcjonalnie) można sprawić aby edytor pokazywał szczegóły gdzie te obrazki są wykorzystywane (np. przez jakie sprity albo efekty cząsteczkowe).
2. **W tym oknie będą dodatkowe opcje customizacji:**
	- Np. tutaj będzie można ustawiać różne parametry obrazkom które będą zapisane w mapie.
		- Parametry dla generacji neon mapy.
		- Customowa ścieżka dla własnoręcznie namalowanej neon mapy (gdyby algorytm nie dawał satysfakcjonującego efektu).
		- Czy wygenerować desaturację (np. dla spelli do których nie ma wystarczająco many).
	- Zmiany będą natychmiast miały efekt.
		- Prawdopodobnie trzeba będzie sklepać asynchroniczną generację atlasu żeby się dało po ludzku manipulować parametry neon map.


- Uwaga: Jeśli w dowolnym momencie zniknie na dysku obrazek który jest wyświetlony pod "Images" - czyli zniknie obrazek gdzieś użyty w edytorze - to edytor natychmiast wypluje error.
	- Pojawi się error z textboxem:

            
            Please modify the contents of this textbox so that the paths point to existing files.

            [tu zaczyna się textbox]
            walls/1/corner.png
            walls/1/edge.png
            walls/1/corner_fancy.png
            
            [OK]
            

		który prosi abyś skopiował to, zmienił manualnie ścieżki do plików których edytor nie odnalazł (z jakiegokolwiek powodu) i przekleił z powrotem do textboxa po czym wcisnął OK.


	- Jeśli zniknie obrazek który nigdzie nie był użyty, to nic się nie stanie.

## Animacje

<!--
	Czy nie chcemy może mieć okna w którym mamy tylko używane animacje?
	Każde miejsce na wybór animacji prosiłoby o wybór zbioru plików z animacją.

	Potem dialog "Images" pokazywałby również wszystkie klatki tej animacji do dowolnego użytku później.
	Problem może powstać jedynie jeśli chcielibyśmy mieć więcej animacji z tego samego zbioru plików.
		Raczej nie będziemy potrzebowali wielu animacji z tego samego zbioru plików.
			Mnożniki prędkości będą ustawiane w obiektach które korzystają z animacji.
-->

Dla pojedynczej animacji, widoczne w edytorze będą następujące informacje:

- Lista klatek. Klatka zawiera:
	- Obrazek do wyświetlenia.
	- Czas trwania klatki w milisekundach.
	- (Opcjonalnie) efekt który ma zostać odegrany gdy nastąpi kolej tej klatki.
		- Np. efekt dźwiękowy i cząsteczkowy.
			- Możliwe że z offsetem, np. ``(x, y)`` o które należy przesunąć start cząsteczek żeby zgadzały się z tym gdzie pada but.
- Nazwa animacji. ZAWSZE będzie tworzona z nazwy plików z klatkami.
- ~~Nazwa własna animacji. Do zmiany w edytorze.~~
	- ~~Domyślnie nazwa pierwszej klatki + dopisek "animation".~~
		- ~~Np. zbiór plików zaczynających się od ``ustrojstwo_1_20ms.png``, dostajemy "Ustrojstwo animation".~~

### Okno "Animations"

Okno z widokiem na wszystkie animacje używane w projekcie.

- Można stąd edytować właściwości animacji.
- Można je usuwać, ale tylko jeśli **nigdzie w grze nie jest używana ta animacja**.
	- Najpierw trzeba będzie usunąć manualnie wszystkie byty (lub flavoury) które z niej korzystają, żeby nie było zaskoczenia.

#### Automatyczny import

Doszliśmy do wniosku że warto byłoby aby, mając zbiór plików rodzaju:

```
ustrojstwo_1_20ms.png
ustrojstwo_2_20ms.png
ustrojstwo_3_20ms.png
```

można było go automatycznie zaimportować do edytora jako animacja.
Będzie to możliwe z tego samego okna.

- Rozwijalne drzewo nazwane "Import" pod którym znowu jest cała hierarchia wszystkich obrazków na dysku.
	- Edytor wykryje, że kilka nazw ma ten sam pattern z numerami klatek i długościami w milisekundach.
		- Wtedy zwinie taką listę w jeden element obok którego pojawi się przycisk "Import"
