#include <stdlib.h>
#include <time.h>
#include <assert.h>

int range_random(const int from, const int to)
{
    if (from > to)
        return -1;
    /**
     * rand() % (b - a + 1) -> 0 ~ (b-a) ... (1)
     * (1) + a -> a ~ b
    **/
    return (rand() % (to - from + 1)) + from;
}

int check(const int from, const int to)
{
    int random_value = range_random(from, to);

    if (from <= to)
        return from <= random_value && random_value <= to;

    return random_value == -1;
}

int main(void)
{
    /* 시드 초기화 */
    srand(time(NULL));

    assert(check(0, 100));      // 0 <= x <= 100
    assert(check(100, 100));    // 100 <= x <= 100
    assert(check(50, 100));     // 50 <= x <= 100
    assert(check(100, 50));     // -1
    
    exit(EXIT_SUCCESS);
}