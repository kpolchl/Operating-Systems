#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAX_PATIENTS_IN_HOSPITAL 3
#define CONSULTATION_MEDICINE_REQUIRED 3
#define MEDICINE_PER_DELIVERY 3
#define MAX_MEDICINE_CAPACITY 6

int num_patients;
int num_pharmacists;

int patients_in_hospital = 0;
int patient_ids_in_consultation_room[MAX_PATIENTS_IN_HOSPITAL];
int medicine_count = MAX_MEDICINE_CAPACITY;
int pending_pharmacist_deliveries = 0;

int patients_finished_consultation = 0;
int pharmacists_finished_work = 0;

pthread_mutex_t hospital_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doctor_wake_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t patient_consulted_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t pharmacy_space_available_condition = PTHREAD_COND_INITIALIZER;

char *get_current_time()
{
    static char buffer[64];
    struct timeval tv;
    struct tm *tm_info;

    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);

    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03ld",
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, tv.tv_usec / 1000);
    return buffer;
}

void random_sleep_exact(int seconds)
{
    if (seconds > 0)
    {
        sleep(seconds);
    }
}

void random_sleep_range(int min_seconds, int max_seconds)
{
    if (min_seconds < 0)
        min_seconds = 0;
    if (max_seconds < min_seconds)
        max_seconds = min_seconds;
    if (min_seconds == 0 && max_seconds == 0)
        return;

    int sleep_time = min_seconds;
    if (max_seconds > min_seconds)
    {
        sleep_time += rand() % (max_seconds - min_seconds + 1);
    }
    if (sleep_time > 0)
        sleep(sleep_time);
}

void *patient_thread(void *arg)
{
    int patient_id = *(int *)arg;
    free(arg);
    bool consulted = false;

    while (!consulted)
    {
        int travel_time = 2 + (rand() % (5 - 2 + 1));
        printf("[%s] - Pacjent(%d): Ide do szpitala, bede za %d s\n",
               get_current_time(), patient_id, travel_time);
        random_sleep_exact(travel_time);

        pthread_mutex_lock(&hospital_mutex);

        if (patients_in_hospital >= MAX_PATIENTS_IN_HOSPITAL)
        {
            pthread_mutex_unlock(&hospital_mutex);
            int short_walk_time = 1 + (rand() % (3 - 1 + 1));
            printf("[%s] - Pacjent(%d): za dużo pacjentów, wracam później za %d s\n",
                   get_current_time(), patient_id, short_walk_time);
            random_sleep_exact(short_walk_time);
            continue;
        }

        patient_ids_in_consultation_room[patients_in_hospital] = patient_id;
        patients_in_hospital++;
        printf("[%s] - Pacjent(%d): czeka %d pacjentów na lekarza\n",
               get_current_time(), patient_id, patients_in_hospital);

        if (patients_in_hospital == MAX_PATIENTS_IN_HOSPITAL)
        {
            printf("[%s] - Pacjent(%d): budzę lekarza\n",
                   get_current_time(), patient_id);
            pthread_cond_signal(&doctor_wake_condition);
        }

        int initial_patients_in_hospital_count = patients_in_hospital;
        while (patients_in_hospital >= initial_patients_in_hospital_count && patients_in_hospital > 0 && !consulted)
        {
            pthread_cond_wait(&patient_consulted_condition, &hospital_mutex);

            if (patients_in_hospital < initial_patients_in_hospital_count || patients_in_hospital == 0)
            {
                consulted = true;
            }
        }

        if (consulted)
        {
            printf("[%s] - Pacjent(%d): kończę wizytę\n", get_current_time(), patient_id);
        }

        pthread_mutex_unlock(&hospital_mutex);
    }

    pthread_mutex_lock(&hospital_mutex);
    patients_finished_consultation++;

    pthread_cond_signal(&doctor_wake_condition);
    pthread_mutex_unlock(&hospital_mutex);
    return NULL;
}

void *pharmacist_thread(void *arg)
{
    int pharmacist_id = *(int *)arg;
    free(arg);

    int travel_time = 5 + (rand() % (15 - 5 + 1)); // 5-15s
    printf("[%s] - Farmaceuta(%d): ide do szpitala, bede za %d s\n",
           get_current_time(), pharmacist_id, travel_time);
    random_sleep_exact(travel_time);

    pthread_mutex_lock(&hospital_mutex);

    while (medicine_count >= MAX_MEDICINE_CAPACITY)
    {
        printf("[%s] - Farmaceuta(%d): czekam na oproznienie apteczki\n",
               get_current_time(), pharmacist_id);
        pthread_cond_wait(&pharmacy_space_available_condition, &hospital_mutex);
    }

    if (medicine_count < CONSULTATION_MEDICINE_REQUIRED)
    {
        printf("[%s] - Farmaceuta(%d): budzę lekarza\n",
               get_current_time(), pharmacist_id);
        pthread_cond_signal(&doctor_wake_condition);
    }

    printf("[%s] - Farmaceuta(%d): dostarczam leki\n",
           get_current_time(), pharmacist_id);
    pending_pharmacist_deliveries++;
    pthread_cond_signal(&doctor_wake_condition);

    printf("[%s] - Farmaceuta(%d): zakończyłem dostawę\n",
           get_current_time(), pharmacist_id);

    pharmacists_finished_work++;
    pthread_mutex_unlock(&hospital_mutex);
    return NULL;
}

void *doctor_thread(void *arg)
{
    printf("[%s] - Lekarz: rozpoczynam dyżur. Apteczka: %d/%d.\n", get_current_time(), medicine_count, MAX_MEDICINE_CAPACITY);

    while (1)
    {
        pthread_mutex_lock(&hospital_mutex);

        if (patients_finished_consultation >= num_patients && patients_in_hospital == 0)
        {
            pthread_mutex_unlock(&hospital_mutex);
            break;
        }

        bool condition_consult_possible = (patients_in_hospital == MAX_PATIENTS_IN_HOSPITAL &&
                                           medicine_count >= CONSULTATION_MEDICINE_REQUIRED);

        bool condition_delivery_wake_up = (pending_pharmacist_deliveries > 0 &&
                                           medicine_count < CONSULTATION_MEDICINE_REQUIRED &&
                                           medicine_count < MAX_MEDICINE_CAPACITY);

        if (!condition_consult_possible && !condition_delivery_wake_up)
        {
            printf("[%s] - Lekarz: zasypiam\n", get_current_time());
            pthread_cond_wait(&doctor_wake_condition, &hospital_mutex);

            pthread_mutex_unlock(&hospital_mutex);
            continue;
        }

        printf("[%s] - Lekarz: budzę się\n", get_current_time());

        condition_consult_possible = (patients_in_hospital == MAX_PATIENTS_IN_HOSPITAL &&
                                      medicine_count >= CONSULTATION_MEDICINE_REQUIRED);

        bool condition_accept_delivery_action = (pending_pharmacist_deliveries > 0 &&
                                                 medicine_count < MAX_MEDICINE_CAPACITY);

        if (condition_consult_possible)
        {
            printf("[%s] - Lekarz: konsultuję pacjentów %d, %d, %d\n", get_current_time(),
                   patient_ids_in_consultation_room[0],
                   patient_ids_in_consultation_room[1],
                   patient_ids_in_consultation_room[2]);

            medicine_count -= CONSULTATION_MEDICINE_REQUIRED;

            if (medicine_count < MAX_MEDICINE_CAPACITY)
            {
                pthread_cond_broadcast(&pharmacy_space_available_condition);
            }

            int num_actually_consulted = patients_in_hospital;
            patients_in_hospital = 0;

            pthread_mutex_unlock(&hospital_mutex);
            random_sleep_range(2, 4);
            pthread_mutex_lock(&hospital_mutex);

            pthread_cond_broadcast(&patient_consulted_condition);
        }
        else if (condition_accept_delivery_action)
        {
            printf("[%s] - Lekarz: przyjmuję dostawę leków\n", get_current_time());

            int space_available = MAX_MEDICINE_CAPACITY - medicine_count;
            int medicine_to_add = (space_available >= MEDICINE_PER_DELIVERY) ? MEDICINE_PER_DELIVERY : space_available;

            if (medicine_to_add > 0)
            {
                medicine_count += medicine_to_add;
                pending_pharmacist_deliveries--;

                pthread_cond_broadcast(&pharmacy_space_available_condition);
            }

            pthread_mutex_unlock(&hospital_mutex);
            random_sleep_range(1, 3);
            pthread_mutex_lock(&hospital_mutex);
        }

        pthread_mutex_unlock(&hospital_mutex);
    }

    printf("[%s] - Lekarz: kończę pracę\n", get_current_time());
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <number_of_patients> <number_of_pharmacists>\n", argv[0]);
        return 1;
    }

    num_patients = atoi(argv[1]);
    num_pharmacists = atoi(argv[2]);

    if (num_patients <= 0 || num_pharmacists <= 0)
    {
        printf("Number of patients and pharmacists must be positive.\n");
        return 1;
    }

    srand(time(NULL));

    printf("Szpital rozpoczyna pracę z %d pacjentami i %d farmaceutami\n",
           num_patients, num_pharmacists);
    printf("Apteczka rozpoczyna z %d jednostkami leku\n", medicine_count);

    pthread_t doctor_tid;
    pthread_t *patient_tids = malloc(num_patients * sizeof(pthread_t));
    pthread_t *pharmacist_tids = malloc(num_pharmacists * sizeof(pthread_t));

    if (!patient_tids || !pharmacist_tids)
    {
        perror("Failed to allocate memory for thread IDs");
        free(patient_tids);
        free(pharmacist_tids);
        return 1;
    }

    pthread_create(&doctor_tid, NULL, doctor_thread, NULL);

    for (int i = 0; i < num_patients; i++)
    {
        int *patient_id = malloc(sizeof(int));
        if (!patient_id)
        {
            perror("Failed to allocate memory for patient ID");
            exit(EXIT_FAILURE);
        }
        *patient_id = i + 1;
        pthread_create(&patient_tids[i], NULL, patient_thread, patient_id);
    }

    for (int i = 0; i < num_pharmacists; i++)
    {
        int *pharmacist_id = malloc(sizeof(int));
        if (!pharmacist_id)
        {
            perror("Failed to allocate memory for pharmacist ID");
            exit(EXIT_FAILURE);
        }
        *pharmacist_id = i + 1;
        pthread_create(&pharmacist_tids[i], NULL, pharmacist_thread, pharmacist_id);
    }

    pthread_join(doctor_tid, NULL);

    for (int i = 0; i < num_patients; i++)
    {
        pthread_join(patient_tids[i], NULL);
    }

    for (int i = 0; i < num_pharmacists; i++)
    {
        pthread_join(pharmacist_tids[i], NULL);
    }

    printf("Szpital kończy pracę\n");

    free(patient_tids);
    free(pharmacist_tids);

    pthread_mutex_destroy(&hospital_mutex);
    pthread_cond_destroy(&doctor_wake_condition);
    pthread_cond_destroy(&patient_consulted_condition);
    pthread_cond_destroy(&pharmacy_space_available_condition);

    return 0;
}