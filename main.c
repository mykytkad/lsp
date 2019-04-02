#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mtx;
    unsigned N;
} Sam;

////////////////////////////////////////

#define SAM_INIT(n)                    \
{                                      \
    .cond = PTHREAD_COND_INITIALIZER,  \
    .mtx  = PTHREAD_MUTEX_INITIALIZER, \
    .N    = n                          \
};

////////////////////////////////////////

void sam_init(Sam * ps, int N) {
pthread_mutex_init(&ps->mtx,NULL);
pthread_cond_init(&ps->cond,NULL);
ps->N = N;
}

////////////////////////////////////////

void sam_destroy(Sam * ps) {
    pthread_cond_destroy(&ps->cond);
    pthread_mutex_destroy(&ps->mtx);

}

////////////////////////////////////////

void sam_acquire(Sam * ps) {
    pthread_mutex_lock(&ps->mtx);
    while(ps->N == 0) {
        pthread_cond_wait(&ps->cond,&ps->mtx);
    }

    --(ps->N);
    pthread_mutex_unlock(&ps->mtx);
}

////////////////////////////////////////

void sam_release(Sam * ps) {
    pthread_mutex_lock(&ps->mtx);
    ++(ps->N);
    pthread_mutex_unlock(&ps->mtx);
    pthread_cond_signal(&ps->cond);
}

static const int elements = 100;

typedef struct {
    int begin_pos;
    int last_pos;
    int queue[100];
} Queue;

void init_q( Queue * q)
{
    q->begin_pos = 0;
    q->last_pos = 1;

    return;
}

typedef struct
{
     Sam full;
     Sam empty;
     Sam mutex;

}Strange;

void init_s(Strange *s)
{
    s->empty.cond = PTHREAD_COND_INITIALIZER;
    s->empty.mtx = PTHREAD_MUTEX_INITIALIZER;
    s->empty.N = 1;
    s->full.cond = PTHREAD_COND_INITIALIZER;
    s->full.mtx = PTHREAD_MUTEX_INITIALIZER;
    s->full.N = 1;
    s->mutex.cond = PTHREAD_COND_INITIALIZER;
    s->mutex.mtx = PTHREAD_MUTEX_INITIALIZER;
    s->mutex.N = 1;
    return;

}
///////////////////////////
static Queue que;

void insert(Strange * str, int val )
{
    pthread_cond_wait(&str->full.cond, &str->full.mtx);
    sam_acquire(&str->mutex);
    que.queue[que.last_pos - 1] = val;
    que.last_pos ++;
    if (que.last_pos == elements){que.last_pos = 0;}
    if (que.last_pos == que.begin_pos){sam_acquire(&str->full);}
    sam_release(&str->mutex);
    pthread_cond_signal(&str->empty.cond);
}



///////////////////////////////////////

static int sum = 0;

///////////////////////////////////////



void read(Strange * str  )
{

    pthread_cond_wait(&str->empty.cond, &str->empty.mtx);
    sam_acquire(&str->mutex);
    int val = 0;
    val = que.queue[que.begin_pos];
    sum+=val;
    que.begin_pos++;
    if(que.begin_pos == elements)
    {
        que.begin_pos = 0;
        if(que.last_pos == 1){
            sam_acquire(&str->empty);
        }
    }
    if(que.begin_pos + 1 == que.last_pos){ sam_acquire(&str->empty); }
    sam_release(&str->mutex);
    pthread_cond_signal(&str->full.cond);
}

///////////////////////////////////////



void *thread_func_p(Strange  * str)
{

    for(int i = 0; i<4; i++)
    {
        insert( str, i);


    }

    return NULL;
}
void *thread_func_r(Strange * str)
{
    for(int i=0; i<4; i++)
    {
        read(str);
        //fprintf(stderr,"%d\n", sum);

    }
}
int main()
{

    static Strange str;
    init_q(&que);
    init_s(&str);
    pthread_t threads[4];

    for(int i = 0; i<2; i++){

        pthread_create(&threads[i], NULL, thread_func_p, &str);
        pthread_create(&threads[i+2], NULL, thread_func_r, &str);

    }

    for(int i =0; i<4; i++){
        pthread_join(threads[i], NULL);
    }


    return 0;
}