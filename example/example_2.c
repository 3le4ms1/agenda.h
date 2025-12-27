#include "example_2.h"
void use_agenda_int(agenda(int) *da) {
    for(int i = 0; i < 100; i++) {
        da_push_back(da, &i);
    }
}
