// port.c
#include "port.h"
#include "config.h"
#include <stdbool.h>
#include <stdio.h>

#if ENABLE_LOCK
#if defined(PLATFORM_LINUX)
bool mutex_init(mutex_t *mutex)
{
    return pthread_mutex_init(mutex, NULL) == 0;
}

void mutex_destroy(mutex_t *mutex)
{
    pthread_mutex_destroy(mutex);
}

void mutex_lock(mutex_t *mutex)
{
    DEBUG_PRINT("Linux platform: Attempt to acquire the mutex lock\n");
    pthread_mutex_lock(mutex);
    DEBUG_PRINT("Linux platform: Mutex lock has been acquired\n");
}

void mutex_unlock(mutex_t *mutex)
{
    pthread_mutex_unlock(mutex);
    DEBUG_PRINT("Linux platform: Mutual exclusion lock has been released\n");
}

#elif defined(PLATFORM_FREERTOS)
bool mutex_init(mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();
    return *mutex != NULL;
}

void mutex_destroy(mutex_t *mutex)
{
    vSemaphoreDelete(*mutex);
}

void mutex_lock(mutex_t *mutex)
{
    DEBUG_PRINT("FreeRTOS platform: Attempt to acquire the mutex lock\n");
    xSemaphoreTake(*mutex, portMAX_DELAY);
    DEBUG_PRINT("FreeRTOS platform: Mutex lock acquired\n");
}

void mutex_unlock(mutex_t *mutex)
{
    xSemaphoreGive(*mutex);
    DEBUG_PRINT("FreeRTOS platform: Mutual exclusion lock has been released\n");
}

#elif defined(PLATFORM_BARE_METAL)
bool mutex_init(mutex_t *mutex)
{
    // 裸机平台无需初始化互斥锁
    return true;
}

void mutex_destroy(mutex_t *mutex)
{
    // 裸机平台无需销毁互斥锁
}

void mutex_lock(mutex_t *mutex)
{
    // 裸机平台无需加锁
    DEBUG_PRINT("Bare machine platform: Simulate obtaining a mutex lock\n");
}

void mutex_unlock(mutex_t *mutex)
{
    // 裸机平台无需解锁
    DEBUG_PRINT("Bare machine platform: Simulating the release of mutex lock\n");
}
#endif
#endif
