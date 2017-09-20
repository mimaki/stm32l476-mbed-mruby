#include "mruby.h"
#include <stdio.h>
#include <string.h>
#include "em_malloc.h"

/*
  b0-1 status #0  (offset 0x0000)
  b2-3 status #1  (offset 0x0020)
  b4-5 status #2  (offset 0x0040)
  b6-7 status #3  (offset 0x0060)
*/

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
  em_state.usecnt++;

  /* 先頭空きブロックの更新 */
  if (em_state.head >= head) {
    for (idx=head+blks; idx<_MAX_BLKS; idx++) {
      /* 先頭空きブロックを検索する */
      mapidx = idx / _STS_BLKS;
      mapblk = idx % _STS_BLKS;
      if ((em_state.use_map[mapidx] & _use_map_mask[mapblk]) == 0) {
        break;
      }
    }
    em_state.head = idx;
    if (idx == _MAX_BLKS) {
      puts("em_set_used: Out of memory!");
    }
  }
}

static uint32_t
em_find_free_blocks(uint32_t head, uint32_t blks)
{
  uint32_t mapidx;
  uint32_t mapblk;
  uint32_t idx;
  uint32_t cnt = 0;

#ifdef EMDEBUG
  printf("em_find_free_blocks(%08lx, %d) : ", head, blks);
#endif

  for (idx=head; idx<_MAX_BLKS; idx++) {
    mapidx = idx / _STS_BLKS;
    mapblk = idx % _STS_BLKS;
    
    if ((em_state.use_map[mapidx] & _use_map_mask[mapblk]) == 0) {
#ifdef EMDEBUG
      printf("o", mapidx*4+mapblk, cnt);
#endif
      if (cnt == 0) {
        head = idx;
#ifdef EMDEBUG
        printf("\nem_find_free_blocks(%08lx, %d) : ", head, blks);
#endif
      }
      cnt++;
      if (cnt >= blks) {
#ifdef EMDEBUG
        puts("");
#endif
        return head;
      }
    }
    else {
#ifdef EMDEBUG
      printf("x");
#endif
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
#ifdef EMDEBUG
    puts("em_search_free: Out of memory!");
#endif
    return NULL;  /* No free block */
  }

  /* 複数ブロック */
  if (blks > 1) {
    head = em_find_free_blocks(head, blks);
  }

  /* check free area */
  if (head >= _MAX_BLKS) {
#ifdef EMDEBUG
    puts("em_search_free: Out of memory!");
#endif
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
#ifdef EMDEBUG
  else {
    puts("em_malloc: Out of memory!");
  }
#endif
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
#ifdef EMDEBUG
    printf("free(0x%08lx) : ", p);
#endif
    em_free(p);
#ifdef EMDEBUG
    printf("usecnt=%d\n", em_state.usecnt);
#endif
    return NULL;
  }

  /* malloc */
  if (p == NULL) {
#ifdef EMDEBUG
    printf("malloc(%d) => ", len);
#endif
    p = em_malloc(blks);
#ifdef EMDEBUG
    printf("%08lx : usecnt=%d\n", p, em_state.usecnt);
#endif
    return p;
  }

  /* realloc */
  used_blks = em_get_used_blocks(p);

#ifdef EMDEBUG
  printf("realloc(0x%08lx, %d) => ", p, len);
#endif
  /* not change */
  if (used_blks == blks) {
#ifdef EMDEBUG
    printf("%08lx : usecnt=%d\n", p, em_state.usecnt);
#endif
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
#ifdef EMDEBUG
    printf("%08lx : usecnt=%d\n", p, em_state.usecnt);
#endif
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
#ifdef EMDEBUG
  printf("%08lx : usecnt=%d\n", p, em_state.usecnt);
#endif
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

void
em_show_status(void)
{
  printf("head=%08lx, use=%d/%d\n", em_state.head, em_state.usecnt, _MAX_BLKS);
}

#ifdef EMDEBUG
void
em_dump(void)
{
  uint32_t i, j, cnt=0;
  uint8_t map, m;

  for (i=0; i<16 && cnt<em_state.usecnt; i++) {
    map = em_state.use_map[i];
    for (j=0; j<4; j++) {
      m = map & _use_map_mask[j];
      if (m == 0) {
        printf("o");
      }
      else {
        if (m == _use_map_used[j]) printf(".");
        else if (m == _use_map_term[j]) printf("*");
        else                            printf("?");
        cnt++;
        if (cnt >= em_state.usecnt) {
          break;
        }
      }
    }
  }
  puts("");
}
#endif
