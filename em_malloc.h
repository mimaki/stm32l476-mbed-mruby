#ifndef _EM_MALLOC_H
#define _EM_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void * em_mallocf(mrb_state*, void*, size_t, void*);

#ifdef __cplusplus
}
#endif

#endif /* _EM_MALLOC_H */
