#ifndef _EM_MALLOC_H
#define _EM_MALLOC_H

// #define EMDEBUG  /* for debug */

/* Heap address and area size */
#define _HEAP_HEAD  0x20000000
#define _HEAP_SIZE  0x00018000  /* 96kB */

/* Memory block size */
#define _BLK_SIZE   8

#define _STS_BITS   2     /* 00:free, 01:use(cont.), 11:use(term.) */
#define _STS_BLKS   (8 / _STS_BITS)

#define _STS_USED   0x01
#define _STS_TERM   0x03
#define _STS_MASK   0x03

#define _MAX_BLKS     (_HEAP_SIZE / _BLK_SIZE)
#define _USE_MAP_SIZE (_MAX_BLKS / _STS_BLKS)

#define _MEM_BLKS(sz) (((sz) + _BLK_SIZE - 1) / _BLK_SIZE)

#define PTR2IDX(p)  ((uint32_t)((uint8_t*)(p) - em_state.blk_head) / _BLK_SIZE)
#define IDX2PTR(i)  (&(em_state.blk_head[(i) * _BLK_SIZE]))

#define _MASK_USED  0x55
#define _MASK_TERM  0xaa


#ifdef __cplusplus
extern "C" {
#endif

void * em_mallocf(mrb_state*, void*, size_t, void*);
void em_show_status(void);
#ifdef EMDEBUG
void em_dump(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _EM_MALLOC_H */
