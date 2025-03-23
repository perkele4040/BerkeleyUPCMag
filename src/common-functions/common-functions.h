//Funkcja printArray wypisuje w konsoli w jednej linii podaną tabelę liczb całkowitych.
int printArray(int arr[], int size);

//Funkcja swapInt zamienia wartości dwóch zmiennych typu całkowitego.
void swapInt(int* xp, int* yp);

//Funkcja validateSortingOutput przyjmuje tabelę liczb całkowitych i jej rozmiar
//oraz sprawdza poprawność posortowania zawartości tabeli.
//Wynik walidacji zwracany jest typem BOOL.
bool validateSortingOutput(int arr[], int size);

//Funkcja writeArrayToOutputFile przyjmuje wskaźnik do pliku .txt oraz tabelę liczb całkowitych i jej rozmiar.
//Funkcja zapisuje liczby do podanego pliku w nowych liniach oraz zwraca błąd w przypadku nieodnalezienia pliku.
void writeArrayToOutputFile(const char* filename, int* size, int arr[]);

//Funkcja loadArrayFromFile przyjmuje wskaźnik do pliku .txt zawierającego ciąg liczb całkowitych podanych w nowych liniach.
//Funkcja zwraca błąd w przypadku nieodnalezienia pliku lub problemów z alokacją pamięci.
//Liczby zwracane są w postaci tabeli INT.
//Alokacja pamięci na potrzeby tabeli przeprowadzana jest dynamiczne w zależności do rozmiaru wejścia.
int* loadArrayFromFile(const char* filename, int* size);