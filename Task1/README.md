# Generator Grafu Zadań

Program w C++ służący do generowania struktur grafowych

## Funkcjonalność
- **Generowanie DAG**: Gwarantuje brak cykli poprzez relację $i \to j$, gdzie $i < j$.
- **Spójność**: Każde zadanie (poza startowym) posiada przynajmniej jednego poprzednika, co eliminuje zadania "wiszące".
- **Modelowanie Zasobów**:
    - Procesory **HC** (Hardware Cores) – dedykowane.
    - Procesory **PP** (Programmable Processors) – uniwersalne.
    - Automatyczne sortowanie jednostek według kosztu zakupu.
- **Parametry Wykonania**: Losowanie macierzy czasów (`@times`) i kosztów (`@cost`) z uwzględnieniem limitów wydajności (wartość `-1` dla procesorów zbyt wolnych).
- **Topologia Komunikacyjna**: Generowanie szyn transmisyjnych (`@comm`) z gwarancją podpięcia każdego procesora do co najmniej jednej linii.

## Instrukcja kompilacji
Wymagany kompilator wspierający standard **C++17** lub nowszy.

```bash
g++ -std=c++17 main.cpp -o generator
```

## Wywołanie
./generator <tasks> <hc> <pp> <bus> <is_weighted> <output_name>

Opis argumentów:
    tasks – Liczba zadań (wierzchołków grafu).
    hc – Liczba procesorów dedykowanych.
    pp – Liczba procesorów uniwersalnych.
    bus – Liczba szyn transmisyjnych.
    is_weighted – Czy krawędzie mają mieć wagi (1 - tak, 0 - nie).
    output_name – Nazwa pliku wyjściowego (program doda .txt).

## Plik wyjściowy
Plik wynikowy podzielony jest na sekcje:
    @tasks: Lista sąsiedztwa (id, stopień wyjściowy, cele).
    @proc: Lista procesorów (koszt, parametr, typ: 0-HC, 1-PP).
    @times: Macierz czasów wykonania (zadania x procesory).
    @cost: Macierz kosztów operacyjnych.
    @comm: Specyfikacja szyn (nazwa, koszt, przepustowość, połączenia 0/1).