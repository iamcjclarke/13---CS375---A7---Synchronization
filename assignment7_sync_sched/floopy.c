#include <stdio.h>
#include <pthread.h>

typedef struct account_t {
    pthread_mutex_t lock;
    int balance;
    long uuid;
} account_t;

typedef struct transfer_args_t {
    account_t* donor;
    account_t* recipient;
    float amount;
} transfer_args_t;

void transfer(account_t *donor, account_t *recipient, float amount) {
    account_t *first = (donor->uuid < recipient->uuid) ? donor : recipient;
    account_t *second = (donor->uuid < recipient->uuid) ? recipient : donor;

    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    if (donor->balance < amount) {
        printf("Insufficient funds.\n");
    } else {
        donor->balance -= (int)amount;
        recipient->balance += (int)amount;
        printf("Transferred %.0f from account %ld to %ld\n",
               amount, donor->uuid, recipient->uuid);
    }

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

void* transfer_thread(void* arg) {
    transfer_args_t* args = (transfer_args_t*)arg;
    transfer(args->donor, args->recipient, args->amount);
    return NULL;
}

int main() {
    account_t acc1 = {PTHREAD_MUTEX_INITIALIZER, 1000, 1};
    account_t acc2 = {PTHREAD_MUTEX_INITIALIZER, 500, 2};

    pthread_t t1, t2;

    transfer_args_t a1 = {&acc1, &acc2, 200};
    transfer_args_t a2 = {&acc2, &acc1, 100};

    pthread_create(&t1, NULL, transfer_thread, &a1);
    pthread_create(&t2, NULL, transfer_thread, &a2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("Final balance acc1: %d\n", acc1.balance);
    printf("Final balance acc2: %d\n", acc2.balance);

    pthread_mutex_destroy(&acc1.lock);
    pthread_mutex_destroy(&acc2.lock);
    return 0;
}
