#include <stdio.h>
#include <pthread.h>

typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
    int priority;
} account_t;

typedef struct transfer_args_t {
    account_t* donor;
    account_t* recipient;
    float amount;
    int priority;
} transfer_args_t;

void transfer(account_t *donor, account_t *recipient, float amount, int thread_priority) {
    account_t *first = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t *second = (donor->uuid < recipient->uuid) ? recipient : donor;

    pthread_mutex_lock(&first->lock);
    if (first->priority < thread_priority) first->priority = thread_priority;

    pthread_mutex_lock(&second->lock);
    if (second->priority < thread_priority) second->priority = thread_priority;

    if (donor->balance < amount) {
        printf("[priority %d] Insufficient funds.\n", thread_priority);
    } else {
        donor->balance -= (int)amount;
        recipient->balance += (int)amount;
        printf("[priority %d] Transferred %.0f from account %ld to %ld\n",
               thread_priority, amount, donor->uuid, recipient->uuid);
    }

    second->priority = 0;
    first->priority = 0;

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

void* transfer_thread(void* arg) {
    transfer_args_t* params = (transfer_args_t*)arg;
    transfer(params->donor, params->recipient, params->amount, params->priority);
    return NULL;
}

int main() {
    account_t acc1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1, 0};
    account_t acc2 = {PTHREAD_MUTEX_INITIALIZER, 500, 2, 0};

    pthread_t t1, t2;

    transfer_args_t p1 = {&acc1, &acc2, 200, 2};
    transfer_args_t p2 = {&acc2, &acc1, 100, 1};

    pthread_create(&t1, NULL, transfer_thread, &p1);
    pthread_create(&t2, NULL, transfer_thread, &p2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final balance acc1: %d\n", acc1.balance);
    printf("Final balance acc2: %d\n", acc2.balance);

    pthread_mutex_destroy(&acc1.lock);
    pthread_mutex_destroy(&acc2.lock);
    return 0;
}
