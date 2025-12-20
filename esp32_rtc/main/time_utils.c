#include <stdio.h>
#include <string.h>
#include <time.h>

time_t get_build_time() {
    struct tm build_tm = {0};
    char month_str[4];
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    // __DATE__ ha il formato "Mmm dd yyyy" (es. "Dec 20 2025")
    // __TIME__ ha il formato "hh:mm:ss" (es. "11:42:00")
    sscanf(__DATE__, "%s %d %d", month_str, &build_tm.tm_mday, &build_tm.tm_year);
    sscanf(__TIME__, "%d:%d:%d", &build_tm.tm_hour, &build_tm.tm_min, &build_tm.tm_sec);

    // Convertiamo il nome del mese in indice (0-11)
    for (int i = 0; i < 12; i++) {
        if (strcmp(month_str, months[i]) == 0) {
            build_tm.tm_mon = i;
            break;
        }
    }

    // struct tm.tm_year richiede gli anni dal 1900
    build_tm.tm_year -= 1900;
    
    // mktime converte struct tm in time_t
    return mktime(&build_tm);
}