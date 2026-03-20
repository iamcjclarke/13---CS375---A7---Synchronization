#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

int numBatterInBowl = 0;
int numEggInBowl = 0;
bool readyToEat = false;
int cakesMade = 0;
const int MAX_CAKES = 3;

pthread_mutex_t lock;
pthread_cond_t needIngredients, readyToBake, startEating;

void addBatter() {
    numBatterInBowl += 1;
    printf("Added batter. Batter=%d Eggs=%d\n", numBatterInBowl, numEggInBowl);
}

void addEgg() {
    numEggInBowl += 1;
    printf("Added egg. Batter=%d Eggs=%d\n", numBatterInBowl, numEggInBowl);
}

void heatBowl() {
    readyToEat = true;
    numBatterInBowl = 0;
    numEggInBowl = 0;
    printf("Cake baked!\n");
}

void eatCake() {
    readyToEat = false;
    cakesMade++;
    printf("Cake eaten! Total cakes=%d\n", cakesMade);
}

void* batterAdder(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (cakesMade >= MAX_CAKES) {
            pthread_cond_broadcast(&readyToBake);
            pthread_cond_broadcast(&startEating);
            pthread_cond_broadcast(&needIngredients);
            pthread_mutex_unlock(&lock);
            break;
        }

        while ((numBatterInBowl >= 1 || readyToEat) && cakesMade < MAX_CAKES) {
            pthread_cond_wait(&needIngredients, &lock);
        }

        if (cakesMade < MAX_CAKES && !readyToEat && numBatterInBowl < 1) {
            addBatter();
            pthread_cond_signal(&readyToBake);
        }

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* eggBreaker(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (cakesMade >= MAX_CAKES) {
            pthread_cond_broadcast(&readyToBake);
            pthread_cond_broadcast(&startEating);
            pthread_cond_broadcast(&needIngredients);
            pthread_mutex_unlock(&lock);
            break;
        }

        while ((numEggInBowl >= 2 || readyToEat) && cakesMade < MAX_CAKES) {
            pthread_cond_wait(&needIngredients, &lock);
        }

        if (cakesMade < MAX_CAKES && !readyToEat && numEggInBowl < 2) {
            addEgg();
            pthread_cond_signal(&readyToBake);
        }

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* bowlHeater(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (cakesMade >= MAX_CAKES) {
            pthread_cond_broadcast(&startEating);
            pthread_mutex_unlock(&lock);
            break;
        }

        while ((numBatterInBowl < 1 || numEggInBowl < 2 || readyToEat) && cakesMade < MAX_CAKES) {
            pthread_cond_wait(&readyToBake, &lock);
        }

        if (cakesMade < MAX_CAKES && numBatterInBowl >= 1 && numEggInBowl >= 2 && !readyToEat) {
            heatBowl();
            pthread_cond_signal(&startEating);
        }

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* cakeEater(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        if (cakesMade >= MAX_CAKES) {
            pthread_cond_broadcast(&needIngredients);
            pthread_mutex_unlock(&lock);
            break;
        }

        while (!readyToEat && cakesMade < MAX_CAKES) {
            pthread_cond_wait(&startEating, &lock);
        }

        if (cakesMade < MAX_CAKES && readyToEat) {
            eatCake();
            pthread_cond_broadcast(&needIngredients);
        }

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    pthread_t batter, egg1, egg2, heater, eater;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&needIngredients, NULL);
    pthread_cond_init(&readyToBake, NULL);
    pthread_cond_init(&startEating, NULL);

    pthread_create(&batter, NULL, batterAdder, NULL);
    pthread_create(&egg1, NULL, eggBreaker, NULL);
    pthread_create(&egg2, NULL, eggBreaker, NULL);
    pthread_create(&heater, NULL, bowlHeater, NULL);
    pthread_create(&eater, NULL, cakeEater, NULL);

    pthread_join(batter, NULL);
    pthread_join(egg1, NULL);
    pthread_join(egg2, NULL);
    pthread_join(heater, NULL);
    pthread_join(eater, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&needIngredients);
    pthread_cond_destroy(&readyToBake);
    pthread_cond_destroy(&startEating);

    return 0;
}
