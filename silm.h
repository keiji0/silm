#ifndef SILM_H
#define SILM_H

extern void si_setup(void *memory, long size);
extern void si_interpreter();
extern void si_load(const char *path);

#endif
