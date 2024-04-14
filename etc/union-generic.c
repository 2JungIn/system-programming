#include <stdio.h>

typedef union data_t
{
    void *ptr;
    char *str;
    char c;
    int i;
    long l;
    float f;
    double d;
} data_t;

void swap(data_t *restrict a, data_t *restrict b)
{
    data_t temp = *a;
    *a = *b;
    *b = temp;
}

int main(void)
{
    /* int test */
    data_t int_value = { .i = 1234 };
    printf("int_value: %d\n", int_value.i);

    /* char test */
    data_t char_value = { .c='A' };
    printf("char_value: %c\n", char_value.c);

    /* double test */
    data_t double_value = { .d=1.234 };
    printf("double_value: %lf\n", double_value.d);

    /* string test */
    data_t string_value = { .str = "Hello, World!" };
    printf("string_value: %s\n", string_value.str);

    /* pointer test */
    int num = 10;
    data_t pointer_value = { .ptr = &num };
    printf("pointer_value: %d (%p)\n", *(int *)pointer_value.ptr, pointer_value.ptr);
    *(int *)pointer_value.ptr = -10;
    printf("pointer_value: %d (%p)\n", *(int *)pointer_value.ptr, pointer_value.ptr);

    data_t a = { .i = 10 };
    data_t b = { .f = 0.1f };
    printf("a: %d b: %f\n", a.i, b.f);
    swap(&a, &b);
    printf("a: %f b: %d\n", a.f, b.i);

    return 0;
}