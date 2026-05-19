## 1. Opis Projektu
Program realizuje algorytm zachłannego przydziału zadań ze struktury grafu skierowanego, niecyklicznego.

## 2. Specyfikacja Pliku Wejściowego
Format wejściowy jest w pełni kompatybilny z definicjami problemu syntezy systemów wbudowanych i zawiera sekcje:
* `@tasks` – Definicja liczby zadań oraz powiązań (krawędzi) wraz z wagami komunikacyjnymi.
* `@proc` – Liczba procesorów, ich bazowy koszt zakupu oraz typ (0 dla HC, 1 dla PP).
* `@times` – Macierz czasów wykonania poszczególnych zadań na dedykowanych maszynach.
* `@cost` – Macierz kosztów operacyjnych wykonania zadań.
* `@comm` – Definicja magistral/szyn komunikacyjnych (`CHAN`), ich kosztów aktywacji oraz wektorów podłączeń do procesorów.

## 3. Matematyczna Funkcja Celu
Wybór optymalnego zasobu w każdym kroku topologicznym opiera się o minimalizację globalnego kryterium $F$:

$$F(\text{cost}, t) = k_1 \cdot \text{cost} + k_2 \cdot t + k_3 \cdot \text{kara}$$

Gdzie funkcja opóźnienia czasowego (kara) zdefiniowana jest jako:

$$\text{kara} = \max(0, t - t_{\max})$$

Do kosztu całkowitego (`cost`) wliczane są:
1. Koszty uruchomienia zadań na wybranych jednostkach.
2. Jednorazowy koszt zakupu każdego procesora, który został użyty choć raz.
3. Koszt stały podpięcia aktywnego procesora do szyn komunikacyjnych według sekcji `@comm`.

## 4. Instrukcja Uruchomienia

Kompilacja

```bash
g++ -O3 main.cpp -o embedded_solver
```

## 5. Przebieg działania – program poprosi kolejno o podanie:

Nazwy pliku wejściowego (np. input.txt)

Współczynnika k1 – waga przypisana całkowitemu kosztowi systemu (np. 1.0).

Współczynnika k2 – waga przypisana czasowi wykonania (np. 3.0).

Współczynnika k3 – kara za każdą jednostkę czasu przekraczającą limit (np. 50.0).

Granicy czasowej (T_max) – maksymalny akceptowalny czas wykonania (np. 1200).

Maksymalnej wartości funkcji (F_max) – górny limit odrzucający rozwiązania (np. 20000).