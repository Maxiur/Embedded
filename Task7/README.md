## Jak uruchomić

### 1. Uruchomienie aplikacji
Aby uruchomić generator, wykonaj poniższe polecenie w terminalu:
```bash
./<nazwa_programu>
```

### 2. Parametryzacja
Program działa w trybie interaktywnym. Po uruchomieniu zostaniesz poproszony o sekwencyjne wprowadzenie następujących parametrów konfiguracyjnych:

1. **Liczba zadań** – całkowita liczba węzłów w generowanym grafie (tasks).
2. **Liczba koprocesorów sprzętowych (HC)** – liczba dostępnych jednostek dedykowanych.
3. **Liczba procesorów ogólnego przeznaczenia (PP)** – liczba dostępnych jednostek uniwersalnych.
4. **Liczba kanałów komunikacyjnych** – liczba magistrali łączących procesory.
5. **Wagi krawędzi** – czy krawędzie w grafie mają posiadać wagi określające koszt komunikacji (`1` – włączone, `0` – wyłączone).
6. **Nazwa pliku wyjściowego** – nazwa pliku docelowego (bez rozszerzenia), do którego zostanie zapisany wygenerowany model systemu. Program automatycznie utworzy plik tekstowy we wskazanym formacie.
