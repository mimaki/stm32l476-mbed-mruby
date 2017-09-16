#include "mruby.h"
#include <stdio.h>
#include <string.h>
#include "em_malloc.h"

#define _HEAP_HEAD  0x20000000
#define _HEAP_SIZE  0x00018000  /* 96kB */

#define _BLK_SIZE   64 

#define _STS_BITS   2     /* 00:free, 01:use(cont.), 11:use(term.) */
#define _STS_BLKS   (8 / _STS_BITS)

/*
  b0-1 status #0  (offset 0x0000)
  b2-3 status #1  (offset 0x0020)
  b4-5 status #2  (offset 0x0040)
  b6-7 status #3  (offset 0x0060)
*/

#define _STS_USED   0x01
#define _STS_TERM   0x03
#define _STS_MASK   0x03

#define _MASK_USED  0x55
#define _MASK_TERM  0xaa

#define _MAX_BLKS     (_HEAP_SIZE / _BLK_SIZE)
#define _USE_MAP_SIZE (_MAX_BLKS / _STS_BLKS)

#define _MEM_BLKS(sz) (((sz) + _BLK_SIZE - 1) / _BLK_SIZE)

#define PTR2IDX(p)  ((uint32_t)((uint8_t*)(p) - em_state.blk_head) / _BLK_SIZE)
#define IDX2PTR(i)  (&(em_state.blk_head[(i) * _BLK_SIZE]))

static const uint8_t _use_map_mask[] = {_STS_MASK, _STS_MASK<<2, _STS_MASK<<4, _STS_MASK<<6};
static const uint8_t _use_map_used[] = {_STS_USED, _STS_USED<<2, _STS_USED<<4, _STS_USED<<6};
static const uint8_t _use_map_term[] = {_STS_TERM, _STS_TERM<<2, _STS_TERM<<4, _STS_TERM<<6};

typedef struct {
  uint8_t *blk_head;  /* Pointer to memory block area */
  uint32_t head;      /* Block index 0..._MAX_BLKS */
  uint32_t usecnt;
  uint8_t use_map[_USE_MAP_SIZE];
} _em_state;

static _em_state em_state = {
  (uint8_t*)_HEAP_HEAD,
  0,
  0,
  {0}
};

// フリーの連続領域が見つかっている前提でフリーブロックを使用中とする
// HEAPエリア範囲チェックは行わない
static void
em_set_used(uint32_t head, uint32_t blks)
{
  uint32_t mapidx = head / _STS_BLKS;
  uint32_t mapblk = head % _STS_BLKS;
  uint32_t idx;

  for (idx=0; idx<blks-1; idx++) {
    /* メモリブロック使用中設定 */
    em_state.use_map[mapidx] |= _use_map_used[mapblk++];
    em_state.usecnt++;
    if (mapblk >= _STS_BLKS) {
      mapblk = 0;
      mapidx++;
    }
  }
  /* 最終メモリブロックは終端設定する */
  em_state.use_map[mapidx] |= _use_map_term[mapblk++];

  /* 先頭空きブロックの更新 */
  if (em_state.head >= head) {
    for (idx=head+blks; idx<=_MAX_BLKS; idx++) {
      /* 先頭空きブロックを検索する */
      mapidx = idx / _STS_BLKS;
      mapblk = idx % _STS_BLKS;
      if ((em_state.use_map[mapidx] & _use_map_mask[mapblk]) == 0) {
        break;
      }
    }
    em_state.head = idx;
  }
}

static uint32_t
em_find_free_blocks(uint32_t head, uint32_t blks)
{
  uint32_t mapidx;
  uint32_t mapblk;
  uint32_t idx;
  uint32_t cnt;

  for (idx=head; idx<_MAX_BLKS; idx++) {
    mapidx = idx / _STS_BLKS;
    mapblk = idx % _STS_BLKS;

// printf("em_find_free_blocks : head=%08lx, idx=0x%08lx : ", head, idx);
    
    if ((em_state.use_map[mapidx] & _use_map_mask[mapblk]) == 0) {
// printf("free\n");
      if (cnt == 0) {
        head = idx;
      }
      cnt++;
      if (cnt >= blks) {
        return head;
      }
    }
    else {
// printf("used\n");
      cnt = 0;
    }
  }

  /* free blocks not found */
  return _MAX_BLKS;
}

void*
em_search_free(uint32_t blks)
{
  uint32_t head = em_state.head;
// printf("em_search_free(%d) : head=0x%08lx\n", blks, head);

  if (head >= _MAX_BLKS) {
puts("em_search_free: Out of memory!");
    return NULL;  /* No free block */
  }

  /* 複数ブロック */
  if (blks > 1) {
    head = em_find_free_blocks(head, blks);
  }

  /* check free area */
  if (head >= _USE_MAP_SIZE) {
    return NULL;
  }

  return IDX2PTR(head);
}

void*
em_malloc(uint32_t blks)
{
  /* find free blocks */
  uint8_t *mem = em_search_free(blks);
  
  /* change use_map status to used */
  if (mem) {
    em_set_used(PTR2IDX(mem), blks);
  }

  return mem;
}

static void
em_set_free(uint32_t blkidx)
{
  uint32_t mapidx = blkidx / _STS_BLKS;
  uint32_t mapblk = blkidx % _STS_BLKS;
  uint8_t sts;
  uint8_t msk;

  while (mapidx < _USE_MAP_SIZE) {
    sts = em_state.use_map[mapidx]; /* 毎回読む必要はないが... */
    msk = _use_map_mask[mapblk];

    /* ブロック解放 */
    em_state.use_map[mapidx] &= ~msk;
    em_state.usecnt--;

    if ((sts & msk) == msk) {
      break;  /* Found terminate */
    }
    if ((sts & msk) == 0) {
      // Logical error
puts("em_set_free: Logical Error!");
    }

    mapblk++;
    if (mapblk >= _STS_BLKS) {
      mapidx++;
      mapblk = 0;
    }
  }
}

void
em_free(void *p)
{
  uint32_t blkidx = PTR2IDX(p);

  /* change use_map status to free */
  em_set_free(blkidx);

  /* modify head offset */
  if (em_state.head > blkidx) {
    em_state.head = blkidx;
  }
}

static uint32_t
em_get_used_blocks(void *p)
{
  uint32_t head;
  uint32_t blks = 0;
  uint32_t mapidx;
  uint32_t mapblk;
  uint8_t sts;
  uint8_t msk;

  for (head=PTR2IDX(p); ; head++) {
    mapidx = head / _STS_BLKS;
    mapblk = head % _STS_BLKS;

    msk = _use_map_mask[mapblk];
    sts = em_state.use_map[mapidx] & msk;
    if (sts == 0) {
      // logical error
puts("em_get_used_blocks: Logical Error!");
    }
    blks++;
    if (sts == msk) {
      break;
    }
  }
  return blks;
}

void*
em_realloc(void *p, size_t len)
{
  uint32_t blks = _MEM_BLKS(len);
  uint32_t used_blks;
  uint32_t blkidx;
  uint8_t *exp;

  /* free */
  if (len == 0) {
printf("free(0x%08lx) : ", p);
    em_free(p);
printf("usecnt=%d\n", em_state.usecnt);
    return NULL;
  }

  /* malloc */
  if (p == NULL) {
printf("malloc(%d) : ", len);
    p = em_malloc(blks);
printf("usecnt=%d\n", em_state.usecnt);
    return p;
  }

  /* realloc */
  used_blks = em_get_used_blocks(p);

printf("realloc(0x%08lx, %d) : ", p, len);
// printf("realloc: %d -> %d\n", used_blks, blks);
  /* not change */
  if (used_blks == blks) {
printf("usecnt=%d\n", em_state.usecnt);
    return p;
  }

  /* reduce blocks */
  if (used_blks > blks) {
    blkidx = PTR2IDX(p) + blks;
    /* free blocks */
    em_set_free(blkidx);
    blkidx--;
    /* modify termination */
    em_state.use_map[blkidx / _STS_BLKS] |= _use_map_term[blkidx % _STS_BLKS];
printf("usecnt=%d\n", em_state.usecnt);
    return p;
  }

  /* expand blocks */
  // TODO: expand current block
  exp = em_malloc(blks);
  if (exp) {
    memcpy(exp, p, _BLK_SIZE * blks);
  }
  else {
    // no memory
    puts("em_realloc: Out of memory!");
  }
  em_free(p);
printf("usecnt=%d\n", em_state.usecnt);
  return exp;
}

/*
 * malloc for mrb_open_allocf
*/
void*
em_mallocf(mrb_state *mrb, void *p, size_t len, void *ud)
{
  return em_realloc(p, len);
}
