#include<stdio.h>
#include<upc_strict.h>
#include <time.h>
#include <stdlib.h>
#include<stdbool.h>

int main()
{
    printf("Watek %d: Hello World!\n", MYTHREAD);
    upc_global_exit(0);
}
/*
Analiza zostanie przeprowadzona dla następujących kategorii przykładowych zadao o różnym
charakterze obliczeniowym:
     Algorytmy sortujące - bąbelkowe, szybkie
     Problemy optymalizacyjne – dyskretny problem plecakowy, optymalizacja wartości
        funkcji algorytmem genetycznym
     Przetwarzanie obrazów
oraz przez wybrany schemat wykonania:
     Z komunikacją pełną (broadcast, exchange) lub ograniczoną (scatter, gather)
     Z dostępami do pamięci kontrolowanymi (ALLSYNC, strict) lub wolnymi (NOSYNC,
relaxed)
*/