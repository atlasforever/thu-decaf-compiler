#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// DECAF Standard Library

#define LINE_MAX_SIZE 64
#define LINE_MAX_SIZE_STR "64"
#define INT_MAX_LENGHT 12
#define INT_MAX_LENGHT_STR "12"

void* _dcf_ALLOCATE(size_t size);
char* _dcf_READ_LINE();
int32_t _dcf_READ_INT();
int32_t _dcf_STRING_EQUAL(const char *s1, const char *s2);
void _dcf_PRINT_INT(int32_t i);
void _dcf_PRINT_STRING(const char *s);
void _dcf_PRINT_BOOL(int32_t b);
void _dcf_HALT(const char *msg);

void* _dcf_ALLOCATE(size_t size) {
    void *p = calloc(size, 1);
    if (!p) {
        _dcf_HALT("stdlib alloc fail");
    }
    return p;
}

char* _dcf_READ_LINE() {
    char *buf = malloc(LINE_MAX_SIZE);

    if (!buf) {
        _dcf_HALT("stdlib alloc fail");
    }
    scanf("%" LINE_MAX_SIZE_STR "s", buf);
    return buf;
}

int32_t _dcf_READ_INT() {
    char buf[INT_MAX_LENGHT] = {0};

    scanf("%"INT_MAX_LENGHT_STR"s", buf);
    return atoi(buf);
}

int32_t _dcf_STRING_EQUAL(const char *s1, const char *s2) {
    return strcmp(s1, s2) == 0 ? 1 : 0;
}

void _dcf_PRINT_INT(int32_t i) {
    printf("%d", i);
}

void _dcf_PRINT_STRING(const char *s) {
    printf("%s", s);
}

void _dcf_PRINT_BOOL(int32_t b) {
    printf("%s", b == 0 ? "false" : "true");
}

void _dcf_HALT(const char *msg) {
    printf("Decaf runtime error: %s\n", msg);
    exit(1);
}


int32_t _dcf_rt_INSTANCE_OF(void *vptr, const void *dstVtbl) {
    void **cur = vptr;

    while (cur != NULL) {
        if (cur == dstVtbl) {
            return 1;
        } else {
            cur = (void**)(cur[0]);
        }
    }
    return 0;
}