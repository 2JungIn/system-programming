/**
 * thread-pool.c
 * 
 * 스레드풀 구현 예제 입니다.
**/

#define _POSIX_C_SOURCE  200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pthread.h>


// #define AUTO_RESIZE

/* array queue */
typedef struct queue
{
    int capacity;

    void **memory;
    int front;
    int rear;

    void (*free_item)(void *);
} queue;

/* task structure */
struct task
{
    int (*function)(void *);    /* 성공하면 0리턴, 실패하면 -1 리턴 */
    void *parameter;            /* task 함수의 매개변수 */
};

/* task queue */
typedef struct task_queue
{
    queue *q;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue;

/* worker thread arg */
typedef struct thread_arg   
{
    int id;
    int state;
    pthread_t tid;
    
    task_queue *tq;
} thread_arg;

/* thread pool structure */
typedef struct thread_pool
{
    task_queue *tq;   /* task queue */

    int thread_cnt;     /* worker thread count */
    thread_arg *t_args; /* worker threads */
} thread_pool;


/* task functions */
int task1(void *param);
int task2(void *param);
int task3(void *param);


/* queue function */
queue *init_queue(void (*free_item)(void *), const int queue_size);
void destroy_queue(queue *q);
void queue_print(const queue *q, void (*print_func)(const void *));

int queue_full(const queue *q);
int queue_empty(const queue *q);
int queue_size(const queue *q);
int queue_resize(queue *q, const int size);


int queue_enqueue(queue *q, void *item);
void *queue_dequeue(queue *q);
void queue_clear(queue *q);


/* task_queue function */
task_queue *task_queue_init(const int queue_size);
int task_queue_destroy(task_queue *tq, void (*free_item)(void *));

int task_queue_full(const task_queue *tq);
int task_queue_resize(task_queue *tq, const int size);

int task_queue_push(task_queue *tq, struct task *item);
struct task *task_queue_pop(task_queue *tq);


/* worker thread function */
void *worker(void *param)
{    
    thread_arg *t_arg = (thread_arg *)param;
    
    int rc;
    while (t_arg->state)
    {
        struct task *t = task_queue_pop(t_arg->tq);

        if (t_arg->state)
        {
            printf("TID(%d): Task start!\n", t_arg->id);
            rc = t->function(t->parameter);
            printf("TID(%d): Task end!\n", t_arg->id);
        }

        if (rc < 0)
            printf("task fail!\n");

        free(t);
    }

    pthread_exit(NULL);
}


/* thread pool */
thread_pool *init_thread_pool(const int thread_cnt)
{
    if (thread_cnt <= 0)  /* error: invalid arg */
        return NULL;

    thread_pool *pool = malloc(sizeof(thread_pool));
    assert(pool);   /* error: malloc() */

    pool->tq = task_queue_init(thread_cnt);
    assert(pool->tq);   /* error: task_queue_init() */

    pool->thread_cnt = thread_cnt;
    pool->t_args = (thread_arg *)malloc(sizeof(thread_arg) * thread_cnt);
    assert(pool->t_args);   /* error: malloc() */

    /* pthread create */
    int rc;
    for (int i = 0; i < thread_cnt; i++)
    {
        thread_arg *t_arg = &pool->t_args[i];

        t_arg->id = i + 1;      /* id */
        t_arg->state = 1;       /* running state */
        t_arg->tq = pool->tq;   /* woreker queue */

        if ((rc = pthread_create(&t_arg->tid, NULL, worker, t_arg)) < 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    return pool;
}

void destroy_thread_pool(thread_pool *pool)
{
    assert(pool);   /* error: invalid arg */

    int rc;
    int thread_cnt = pool->thread_cnt;

    /* running 에서 stop을 변경 */
    for (int i = 0; i < thread_cnt; i++)
        pool->t_args[i].state = 0; /* stop state */

    /* 브로드 캐스팅을 해서 cond_wait 상태인 스레드들을 모두 깨움 */
    if ((rc = pthread_cond_broadcast(&pool->tq->cond)) < 0)
    {
        fprintf(stderr, "pthread_cond_broadcast: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    /* thread join */
    for (int i = 0; i < thread_cnt; i++)
    {
        if ((rc = pthread_join(pool->t_args[i].tid, NULL)) < 0)
        {
            fprintf(stderr, "pthread_join: %s\n", strerror(rc));
            exit(EXIT_FAILURE);
        }
    }

    /* worker queue 파괴 */
    task_queue_destroy(pool->tq, free);

    /* 자원 해제*/
    free(pool->t_args);
    free(pool);
}

int thread_pool_add_task(const thread_pool *pool, int (*function)(void *), void *parameter)
{
    assert(pool);       /* error: invalid arg */
    assert(function);   /* error: invalid arg */
    
    /* make task */
    struct task *new_task = (struct task *)malloc(sizeof(struct task));
    assert(new_task);   /* error: malloc() */

    new_task->function = function;
    new_task->parameter = parameter;

    /* push task */
    if (task_queue_full(pool->tq))
    {
#ifdef AUTO_RESIZE
        if (task_queue_resize(pool->tq, pool->tq->q->capacity * 2) < 0)
        {
            printf("task_queue_resize: fail!\n");
            return -1;
        }
#else
        free(new_task);
        printf("task queue is full!\n");

        return -1;
#endif
    }
    
    return task_queue_push(pool->tq, new_task);
}


int main(void)
{
    printf("1: 10초동안 슬립 합니다.\n");
    printf("2: \"Hello, Thread!\"를 출력합니다.\n");
    printf("3: 1 ~ 10000까지의 합을 계산 후 출력합니다.\n");
    printf("-1: 프로그램을 종료 합니다.\n");

    int rc;
    thread_pool *pool = init_thread_pool(8);

    int item;
    while(1)
    {
        scanf("%d", &item);

        if (item == -1)
            break;
        else if (item == 1)
            thread_pool_add_task(pool, task1, NULL);
        else if (item == 2)
            thread_pool_add_task(pool, task2, "Hello, Thread!");
        else if (item == 3)
            thread_pool_add_task(pool, task3, NULL);
        else 
            printf("잘못 입력 하셨습니다!\n");
    }

    /* debug msg */
    printf("task queue capacity: %d\n", pool->tq->q->capacity);
    printf("task queue size: %d\n", queue_size(pool->tq->q));

    destroy_thread_pool(pool);

    exit(EXIT_SUCCESS);
}

/* task implementation */
int task1(void *param)
{
    sleep(10);

    return 0;
}

int task2(void *param)
{
    char *msg = (char *)param;
    
    printf("%s\n", msg);

    return 0;
}

int task3(void *param)
{
    int sum = 0;
    for (int i = 1; i <= 10000; i++)
        sum += i;

    printf("sum: %d\n", sum);

    return 0;
}



/* queue implementation */
queue *init_queue(void (*free_item)(void *), const int queue_size)
{   
    if (queue_size <= 0)    /* error: invalid arg */
        return NULL;

    queue *q = NULL;
    if ((q = (queue *)malloc(sizeof(queue))) == NULL)    /* error: malloc() */
        return NULL;
    
    if ((q->memory = (void **)malloc(sizeof(void *) * queue_size)) == NULL)   /* error: malloc() */
    {
        free(q);
        return NULL;
    }

    q->capacity = queue_size;
    q->front = q->rear = 0;

    q->free_item = free_item;

    return q;
}

void destroy_queue(queue *q)
{
    if (q == NULL)    /* error: invalid arg */
        return ;
    
    queue_clear(q);
    free(q->memory);
    free(q);
}


int queue_full(const queue *q)
{
    if (q == NULL)  /* error: invalid arg */
        return -1;

    return (q->rear + 1) % q->capacity == q->front;
}

int queue_empty(const queue *q)
{
    if (q == NULL)  /* error: invalid arg */
        return -1;

    return q->front == q->rear;
}

int queue_size(const queue *q)
{
    if (q == NULL)  /* error: invalid arg */
        return -1;

    if (queue_empty(q))
        return 0;

    int size = q->rear - q->front;
    
    if (q->front < q->rear)
        return size;
    
    return size + q->capacity;
}

int queue_resize(queue *q, const int size)
{
    if (q == NULL)   /* error: invalid arg */
        return -1;

    if (size <= 1)   /* error: size는 1보다 커야 한다. */
        return -1;

    int new_capacity = size;
    void **new_memory = NULL;
    if ((new_memory = (void **)malloc(sizeof(void *) * new_capacity)) == NULL)  /* error: malloc() */
        return -1;

    int count = 0;
    if (new_capacity < q->capacity)
    {
        int i = q->front;
        for ( ; count < size; count++)
            new_memory[count] = q->memory[i++];

        void (*free_item)(void *) = q->free_item;
        if (free_item)
        {
            for ( ; i != q->rear; i = (i + 1) % q->capacity)
                free_item(q->memory[i]);
        }

        q->front = 0;
        q->rear = size - 1;
    }
    else
    {
        for (int i = q->front; i != q->rear; i = (i + 1) % q->capacity)
            new_memory[count++] = q->memory[i];
    }

    free(q->memory);
    q->capacity = new_capacity;
    q->memory = new_memory;

    return 0;
}


int queue_enqueue(queue *q, void *item)
{
    if (q == NULL)  /* error: invalid arg */
        return -1;

    if (item == NULL)   /* error: invalid arg */
        return -1;

    if (queue_full(q))
        return -1;
    
    q->memory[q->rear] = item;
    q->rear = (q->rear + 1) % q->capacity;

    return 0;
}

void *queue_dequeue(queue *q)
{
    if (q == NULL)  /* error: invalid arg */
        return NULL;
    
    if (queue_empty(q))
        return NULL;
    
    void *ret_item = NULL;
    
    void (*free_item)(void *) = q->free_item;
    if (free_item)
        free_item(q->memory[q->front]);
    else
        ret_item = q->memory[q->front];

    q->front = (q->front + 1) % q->capacity;

    return ret_item;
}


void queue_clear(queue *q)
{
    if (q == NULL)  /* error: invalid arg */
        return ;

    while (!queue_empty(q))
        queue_dequeue(q);

    q->front = q->rear = 0;
}



/* task queue implementation */
task_queue *task_queue_init(const int queue_size)
{
    task_queue *tq = (task_queue *)malloc(sizeof(task_queue));
    assert(tq);     /* error: malloc() */

    tq->q = init_queue(NULL, queue_size);
    assert(tq->q);  /* error: init_list() */

    int rc;
    if ((rc = pthread_mutex_init(&tq->mutex, NULL)))
    {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }
    if ((rc = pthread_cond_init(&tq->cond, NULL)))
    {
        fprintf(stderr, "pthread_cond_init: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    return tq;
}

int task_queue_destroy(task_queue *tq, void (*free_item)(void *))
{
    assert(tq);         /* error: invalid arg */
    assert(free_item);  /* error: invalid arg */

    int rc;
    if ((rc = pthread_mutex_lock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(rc));
        return -1;
    }
    
    tq->q->free_item = free_item;
    destroy_queue(tq->q);

    if ((rc = pthread_mutex_unlock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(rc));
        return -1;
    }

    if ((rc = pthread_mutex_destroy(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(rc));
        return -1;
    }
    if ((rc = pthread_cond_destroy(&tq->cond)))
    {
        fprintf(stderr, "pthread_cond_destroy: %s\n", strerror(rc));
        return -1;
    }

    free(tq);

    return 0;
}


int task_queue_full(const task_queue *tq)
{
    return queue_full(tq->q);
}

int task_queue_resize(task_queue *tq, const int size)
{
    int rc;
    int ret_val;
    if ((rc = pthread_mutex_lock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(rc));
        return -1;
    }
    
    ret_val = queue_resize(tq->q, size);

    if ((rc = pthread_mutex_unlock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(rc));
        return -1;
    }

    return ret_val;
}


int task_queue_push(task_queue *tq, struct task *item)
{
    assert(tq);     /* error: invalid arg */
    assert(item);   /* error: invalid arg */

    int rc;
    int ret_val;
    if ((rc = pthread_mutex_lock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(rc));
        return -1;
    }
    
    ret_val = queue_enqueue(tq->q, item);

    if ((rc = pthread_mutex_unlock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(rc));
        return -1;
    }

    if (ret_val == 0 && (rc = pthread_cond_signal(&tq->cond)) < 0)
    {
        fprintf(stderr, "pthread_cond_signal: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    return ret_val;
}

struct task *task_queue_pop(task_queue *tq)
{
    assert(tq);         /* error: invalid arg */

    int rc;
    struct task *ret_val;
    if ((rc = pthread_mutex_lock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(rc));
        return NULL;
    }
    
    if (queue_empty(tq->q))
    {
        if ((rc = pthread_cond_wait(&tq->cond, &tq->mutex)))
        {
            fprintf(stderr, "pthread_cond_wait: %s\n", strerror(rc));
            return NULL;
        }
    }

    if (queue_empty(tq->q))
        ret_val = NULL;
    else
        ret_val = queue_dequeue(tq->q);

    if ((rc = pthread_mutex_unlock(&tq->mutex)))
    {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(rc));
        return NULL;
    }

    return ret_val;
}
