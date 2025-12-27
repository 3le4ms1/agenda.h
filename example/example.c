// Prevent annoying warnings
#ifdef _WIN32
#    define PSIZE "%lld"
#else
#    define PSIZE "%ld"
#endif // _WIN32

#include <stdio.h>
#define AGENDA_IMPLEMENTATION
#define AGENDA_SHORT_PREFIX
#define AGENDA_HARDEN_WITH_MAGIC

#define USE_EXAMPLE_2
#ifdef USE_EXAMPLE_2
#include "example_2.h"
#else
#include "../agenda.h"
#endif // USE_EXAMPLE_2


#define array_len(items) (sizeof(items)/sizeof(items[0]))

int main(void) {
    #ifdef AGENDA_HARDEN_WITH_MAGIC
    printf("---- MAGIC NUMBER ----\n");
    uint32_t magic_str = 0 | AGENDA_MAGIC_NUMBER;
    printf("magic: %X, %c%c\n", AGENDA_MAGIC_NUMBER, (char)magic_str, (magic_str >> 8));
    printf("magic: %X, %s\n", AGENDA_MAGIC_NUMBER, (const char*)&magic_str);
    #endif // AGENDA_HARDEN_WITH_MAGIC

    printf("---- agenda_init, agenda_get_header ----\n");
    da(int) array = da_init(int);
    printf("array: %p\n", array);
    printf("agenda_header: %p\n", da_get_header(&array));

    printf("---- agenda_push_back ----\n");
    printf(">>> agenda_push_back(&array, &x) 100x\n");
    for (int i = 0; i < 100; i++) {
        int x = i * 2;
        da_push_back(&array, &x);
    }
    printf("da_items_count(str): "PSIZE"\n", da_items_count(&array));

    printf("---- agenda_shrink_to_fit ----\n");
    printf("size: "PSIZE"\n", da_get_header(&array)->size);
    printf("capacity: "PSIZE"\n", da_get_header(&array)->capacity);

    da_shrink_to_fit(&array);
    printf(">>> agenda_shrink_to_fit(&array)\n");

    printf("size: "PSIZE"\n", da_get_header(&array)->size);
    printf("capacity: "PSIZE"\n", da_get_header(&array)->capacity);

    printf("---- agenda_pop_back ----\n");
    printf("da_items_count(str): "PSIZE"\n", da_items_count(&array));
    for (int i = 0; i < 4; i++) {
        da_pop_back(&array);
    }
    printf(">>> da_pop_back 4x\n");
    printf("da_items_count(str): "PSIZE"\n", da_items_count(&array));

    printf("---- agenda_reset, agenda_push_back_array ----\n");
    da_reset(&array);
    int numbers[] = {11, 12, 13, 14, 15};
    da_push_back_array(&array, numbers, array_len(numbers));
    printf("da_items_count(str): "PSIZE"\n", da_items_count(&array));

    printf("---- agenda last element available ----\n");
    size_t index = da_items_count(&array) - 1;
    printf("last element: %d\n", array[index]);

    #ifdef USE_EXAMPLE_2
    printf("---- USE_EXAMPLE_2: passing to functions defined elsewhere ----\n");
    da_reset(&array);
    printf(">>> use_agenda_int(&array)\n");
    use_agenda_int(&array);
    printf("da_items_count(str): "PSIZE"\n", da_items_count(&array));
    #endif // USE_EXAMPLE_2

    printf("---- agenda_deinit ----\n");
    da_deinit(&array);
    printf("array: %p\n", array);
    return 0;
}
