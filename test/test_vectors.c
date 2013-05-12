#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ottery-internal.h"
#include "ottery.h"

#include "streams.h"

typedef unsigned char u8;

void
dumphex(const char *label, const u8 *bytes, int n)
{
  int m=0;
  if (label) printf("%s: ", label);
  while (n--) {
    printf("%02x", *bytes++);
    if (0 == (++m % 32) && n)
      printf("\n");
  }
  puts("");
}

void
experiment(const u8 *key, const u8 *nonce, unsigned skip,
           const struct ottery_prf *prf)
{
  __attribute__((aligned(16))) struct stream state;
  u8 stream[512];
  unsigned blocklen = prf->output_len / prf->idx_step;
  stream_setup(&state, prf, key, 32, nonce, 8);

  puts("================================================================");
  printf("cipher: %s\n", prf->name);
  dumphex("   key", key, 32);
  dumphex(" nonce", nonce, 8);
  printf("offset: %d\n", skip);

  stream_generate(&state, stream, 512, skip/(blocklen));
  dumphex(NULL, stream, 512);
}

#define X(key,nonce,skip)                                               \
  do {                                                                  \
    int i;                                                              \
    for (i = 0; prfs[i]; ++i) {                                         \
      experiment((const u8*)(key),(const u8*)(nonce),(skip), prfs[i]);  \
    }                                                                   \
  } while (0)

const struct ottery_prf *prfs_best[] = {
#ifdef OTTERY_HAVE_SIMD_IMPL
  &ottery_prf_chacha8_,
  &ottery_prf_chacha12_,
  &ottery_prf_chacha20_,
#endif
  NULL
};

const struct ottery_prf *prfs_no_simd[] = {
  &ottery_prf_chacha8_merged_,
  &ottery_prf_chacha12_merged_,
  &ottery_prf_chacha20_merged_,
  NULL
};

int
main(int argc, char **argv)
{
  const struct ottery_prf **prfs = prfs_best;
  if (argc > 1 && !strcmp(argv[1], "no-simd"))
    prfs = prfs_no_simd;

  X("helloworld!helloworld!helloworld", "!hellowo", 0);
  X("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
    "\0\0\0\0\0\0\0\0", 0);
  X("helloworld!helloworld!helloworld", "!hellowo", 8192);
  X("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",
    "\0\0\0\0\0\0\0\0", 8192);
  X("Zombie ipsum reversus ab viral i", "nferno, ", 128);
  X("nam rick grimes malum cerebro. D", "e carne ", 512);
  X("lumbering animata corpora quaeri", "tis. Sum", 640);
  X("mus brains sit, morbo vel malefi", "cia? De ", 704);

  return 0;
}
