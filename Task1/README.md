##  Generator Grafu Zadań

Program w C++ służący do generowania struktur grafowych.

## Wywoływanie

Aby wywołać program, należy przy uruchomieniu przekazać następujące argumenty funkcji, przykład:
``` bash
    <NazwaProgramu>.exe <tasks> <hc> <pp> <bus> <is_weighted> <output_name>
```

###  Opis argumentów

| Argument | Znaczenie |
| :--- | :--- |
| `tasks` | Liczba zadań (wierzchołków grafu). |
| `hc` | Liczba procesorów dedykowanych. |
| `pp` | Liczba procesorów uniwersalnych. |
| `bus` | Liczba szyn transmisyjnych. |
| `is_weighted` | Czy krawędzie mają mieć wagi (`1` - tak, `0` - nie). |
| `output_name` | Nazwa pliku wyjściowego (program sam dorzuci `.txt`). |

## Plik wyjściowy

Plik wynikowy podzielony jest na sekcje:

* **`@tasks`** – Lista sąsiedztwa (id, stopień wyjściowy, cele).
* **`@proc`** – Lista procesorów (koszt, parametr, typ: `0`-HC, `1`-PP).
* **`@times`** – Macierz czasów wykonania (*zadania × procesory*).
* **`@cost`** – Macierz kosztów operacyjnych.
* **`@comm`** – Specyfikacja szyn (nazwa, koszt, przepustowość, czy połączenie istnieje `0`/`1`).
