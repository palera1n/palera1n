#ifndef RUN_H
#define RUN_H

#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

enum exploit_stage {
    STAGE_PREPARE = 0,
    STAGE_RESET = 1,
    STAGE_SETUP = 2,
    STAGE_SPRAY = 3,
    STAGE_PATCH = 4,
    STAGE_PONGO = 5,
    STAGE_JAILBREAK = 6,
    STAGE_DONE = 7,
};

typedef struct {
    atomic_int result;
    atomic_bool stop;

    atomic_bool exploit_done;
    atomic_bool pongo_done;

    atomic_int stage;
} shared_t;

bool exploit(shared_t *state);

#ifdef __cplusplus
}
#endif

#endif // RUN_H
