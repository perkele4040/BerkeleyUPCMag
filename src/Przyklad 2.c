#include<stdio.h>
#include<upc_strict.h>
#include <time.h>
#include <stdlib.h>
#include<stdbool.h>

void drink(int cel, shared [] int * resources, upc_lock_t *mug_lock, upc_lock_t *tap_lock);

int main()
{
    upc_lock_t *mug_lock = upc_all_lock_alloc();
    upc_lock_t *tap_lock = upc_all_lock_alloc();    
    shared [] int *resources;
    resources = (shared [] int *) upc_all_alloc(1, 2*sizeof(int));
    int resources_init[2] = {4, 2};
    if(MYTHREAD == 0)    
        upc_memput(resources, resources_init, 2 * sizeof(int));
    printf("Liczba kufli %d, liczba kranow %d\n", resources[0], resources[1]);
    int goal;
    if(MYTHREAD == 0)
        printf("Pub sie otwiera\n");
    srand(time(NULL)+MYTHREAD);
    goal = rand() % 5 + 1;
    upc_barrier;
    printf("Watek %d: przyszedlem wypic %d kufli\n", MYTHREAD, goal);
    drink(goal, resouces, mug_lock, tap_lock);
    upc_barrier;
    if(MYTHREAD == 0)
        printf("Pub sie zamyka\n");
    upc_global_exit(0);
}

void drink(int goal, shared [] int * resouces, upc_lock_t *mug_lock, upc_lock_t *tap_lock)
{
    bool has_mug = false;
    bool is_full = false;
    for(int i = 0; i < goal; i++)
    {
        while(!has_mug) {
            sleep(1);
            upc_lock(mug_lock);            
            if(resources[0] > 0) {
                resources[0] -= 1;
                printf("Watek %d: biore kufel, zostalo %d wolnych kufli\n", MYTHREAD, resources[0]);
                upc_unlock(mug_lock);
                sleep(1);
                has_mug = true;
            }
            else                
                upc_unlock(mug_lock);
        }
        printf("Watek %d: mam kufel, ide do kranu\n", MYTHREAD);
        while(!is_full) {
            sleep(1);
            upc_lock(tap_lock);             
            if(resources[1] > 0) {
                resources[1] -= 1;
                printf("Watek %d: nalewam, zostalo %d wolnych kranow\n", MYTHREAD, resources[1]);
                upc_unlock(tap_lock);
                sleep(5);
                is_full = true;
            }
            else
                upc_unlock(tap_lock);
        }
        printf("Watek %d: nalalem, pije\n", MYTHREAD);
        upc_lock(tap_lock);
        resources[1] += 1;
        upc_unlock(tap_lock);        
        sleep(10);
        printf("Watek %d: wypilem, oddaje kufel\n", MYTHREAD);
        upc_lock(mug_lock);
        resources[0] += 1;
        upc_unlock(mug_lock);
        has_mug=false;
        is_full=false;
    }
    printf("Watek %d: wypilem juz wszystko, wychodze\n", MYTHREAD);
}
