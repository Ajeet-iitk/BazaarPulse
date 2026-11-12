/*
 * Fast Data Engine for LifeSheet
 * Compile with MinGW GCC:
 *   gcc -O3 -shared -o sheet_engine.dll sheet_engine.c
 * Or native executable:
 *   gcc -O3 -o sheet_engine.exe sheet_engine.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char date[32];
    char place[64];
    char item[64];
    char desc[256];
    char remark[256];
    double parsedPrice;
} PriceRecord;

// Fast price scanner
double extract_price(const char* desc) {
    if (!desc) return 0.0;
    const char* ptr = desc;
    while (*ptr) {
        if ((*ptr == 'R' && *(ptr+1) == 's') || (unsigned char)(*ptr) == 0xE2) { // "Rs" or UTF-8 currency symbol
            while (*ptr && !(*ptr >= '0' && *ptr <= '9')) ptr++;
            if (*ptr >= '0' && *ptr <= '9') {
                return atof(ptr);
            }
        }
        ptr++;
    }
    return 0.0;
}

// QuickSort comparator for sorting by item name
int compare_by_item(const void* a, const void* b) {
    const PriceRecord* r1 = (const PriceRecord*)a;
    const PriceRecord* r2 = (const PriceRecord*)b;
    return strcmp(r1->item, r2->item);
}

int main() {
    printf("LifeSheet C High-Performance Data Processing Module Loaded.\n");
    return 0;
}
