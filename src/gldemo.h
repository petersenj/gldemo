#ifndef GLDEMO_H
#define GLDEMO_H

typedef struct {
    int swapInterval;

    int sceneArgc;
    char** sceneArgv;
} gldemo_options;

extern void gldemo(gldemo_options* opts);

#endif /* GLDEMO_H */
