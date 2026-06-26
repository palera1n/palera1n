#ifndef RUN_H
#define RUN_H

#include <stdbool.h>

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
    STAGE_DONE = 6,
};

bool exploit(void);

#ifdef __cplusplus
}
#endif

#endif // RUN_H
