/*
 * Lutz's own PRNG :-)
 *
 * (Versions of "prngd" before 0.9.0 did use the OpenSSL internal PRNG.)
 *
 * Technical description:
 *
 * The entropy data is kept in a "pool" of large size (default size is 4kB
 * = 4096 bytes rounded up to full 20bytes = 4100 bytes) (* 8 bits).
 *
 * Adding seed:
 * ============
 *
 * Whenever entropy is added, an SHA1-hash of a (20+64)byte block at a
 * moving "position is created and a block of 20bytes of the new data is
 * hashed in. The 20byte hash is then written back (to be more precise:
 * XORed with the bytes at position) and position is advanced by 20bytes.
 *
 * Requesting random data:
 * =======================
 *
 * Whenever entropy is requested, the same (20+64)byte block is hashed. The
 * 20byte hash output is given back as "random data". The 20byte block is
 * also XORed into the bytes at position. Then position is advanced by 20bytes
 * to retrieve the next "random bytes" until the request is satisfied.
 *
 * In order to improve the mixing (and security), each time random data is
 * requested, the pool is mixed completly before and after serving the
 * request.
 *
 * Security:
 * =========
 *
 * Protection against unseeded PRNG:
 * - Random data is only returned, when enough entropy was seeded into the pool.
 *
 * Protection against reading PRNG state from memory/core dump:
 * - Whenever entropy is retrieved, the pool is completly mixed, entropy is
 *   retrieved, then the pool is again completly mixed. So retrieval of
 *   the state from memory would have to happen before the second mixing.
 *   Retrieval from core won't work anyway, since the second mixing will
 *   happen, even before rand_bytes() returns with the random bytes just
 *   generated.
 * - All memory locations temporarily used are memset() to 0 when finishing
 *   an operation.
 *
 * Acknowledgements:
 * =================
 *
 * First versions of PRNGD (before 0.9.0) utilized the OpenSSL PRNG by linking
 * libcrypto. Functionality like the "complete mix before retrieve" needed to
 * be done with a wrapper, the pool size was fixed and some other limitations
 * applied. Therefore the PRNG needed to be included into "prngd".
 *
 * Because the OpenSSL license is not as flexible as mine, I decided to create
 * my own PRNG. Its design is based on my experience with the OpenSSL PRNG
 * and additionally heavily inspired by Peter Gutmann's work.
 *   http://www.cs.auckland.ac.nz/~pgut001/
 * and especially the text
 *   http://www.cryptoengines.com/~peter/06_random.pdf
 * (See the last reference, section cryptlib-PRNG, for the "20+64"
 * technique :-)
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "prngd.h"

static int position = 0, pool_size = 0, pool_seeded = 0;
static unsigned char *pool = NULL;
static double entropy = 0, entropy_needed;

static void rand_flip_bits(void)
{
  int i;

  /*
   * Flip all bits in the pool
   */
  for (i = 0; i < pool_size; i++)
    pool[i] ^= 0xff;
}

void rand_add(const void *buf, int num, double add)
{
  int i, j, end;
  unsigned char temp_md[SHA_DIGEST_LENGTH];
  SHA1_CTX c;

  /* be absolutely sure that the pool array has been allocated */
  if (pool == NULL) {
  	error_fatal("In rand.c pool has not been allocated", 0);
	exit(EXIT_FAILURE);
  }

  /*
   * Mix new bytes into the pool. To perform this operation, read
   * SHA_DIGEST_LENGTH bytes starting from (position-SHA_DIGEST_LENGTH)
   * and the next 64bytes. Then "add" the new bytes, finally write it back
   * to "position" and advance "position".
   */
  for (i = 0; i < num; i += SHA_DIGEST_LENGTH) {
    SHA1_init(&c);
    /*
     * Read back entropy from the pool while taking account of wrap around
     * effects. First read 20bytes from "left of the current position",
     * then 64bytes starting at position.
     */
    if (position == 0)
      SHA1_update(&c, pool + pool_size - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    else
      SHA1_update(&c, pool + position - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    
    end = position + 64;
    if (end < pool_size)
      SHA1_update(&c, pool + position, 64);
    else {
      end %= pool_size;
      SHA1_update(&c, pool + position, pool_size - position);
      SHA1_update(&c, pool, end);
    }

    SHA1_update(&c, (unsigned char *)buf + i,
		(num - i > SHA_DIGEST_LENGTH) ? SHA_DIGEST_LENGTH : (num - i));
    SHA1_final(&c);
    SHA1_digest(&c, temp_md);
    for (j = 0; j < SHA_DIGEST_LENGTH; j++)
      pool[position + j] ^= temp_md[j];
    position += SHA_DIGEST_LENGTH;
    position %= pool_size;	/* wrap, if needed */
    if (position == 0)
      rand_flip_bits();		/* when wrapped, flip once */
  }

  (void)memset(temp_md, 0, SHA_DIGEST_LENGTH);
  (void)memset(&c, 0, sizeof(c));

  /*
   * Register the weighted entropy, but of course there cannot be more
   * entropy in the pool than its size.
   */
  entropy += add;
  if (entropy > pool_size)
    entropy = pool_size;

  /*
   * The "entropy" counter is incremented and decremented to reflect the
   * entropy added or "used up" by querying. Once enough entropy was added
   * to satisfy the minimum requirement, PRNGD will continue to serve
   * random numbers using the "pseudo" RNG even if the entropy count goes
   * back down below the minimum.
   */
  if (entropy >= entropy_needed)
    pool_seeded = 1;
}

int rand_bytes(unsigned char *buf, int num)
{
  int i, j, end;
  unsigned char temp_md[SHA_DIGEST_LENGTH];
  SHA1_CTX c;

  assert(pool != NULL);

  /*
   * Sanity: only return "random bytes", when the pool was successfully
   * seeded.
   */
  if (!pool_seeded)
    return 0;

  /*
   * Before "random bytes" are returned, make sure that the pool is mixed.
   * To achieve this goal, we run the "mixer" scheme, which is also used
   * for adding and retrieving "20+64" across the pool. Unlike the OpenSSL
   * PRNG, which only mixes once when first retrieving random bytes, we
   * always completly mix the pool before supplying the bytes.
   * In many cases, applications using OpenSSL will only do one transaction,
   * then exit, so it won't make a big difference. PRNGD will run continously,
   * so it _does_ make a difference here. By mixing the pool completly, the
   * bytes finally retrieved are influenced by all bits in the pool. Since
   * only a limited number of bytes can be retrieved at a time and by
   * default, this number (255) is much smaller than the pool size, the
   * bytes retrieved will only reveal a small amount of state information
   * when compared to the total amount of state bits.
   */
  for (i = 0; i < pool_size; i += SHA_DIGEST_LENGTH) {
    SHA1_init(&c);
    if (position == 0)
      SHA1_update(&c, pool + pool_size - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    else
      SHA1_update(&c, pool + position - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    
    end = position + 64;
    if (end < pool_size)
      SHA1_update(&c, pool + position, 64);
    else {
      end %= pool_size;
      SHA1_update(&c, pool + position, pool_size - position);
      SHA1_update(&c, pool, end);
    }

    SHA1_final(&c);
    SHA1_digest(&c, temp_md);
    for (j = 0; j < SHA_DIGEST_LENGTH; j++)
      pool[position + j] ^= temp_md[j];
    position += SHA_DIGEST_LENGTH;
    position %= pool_size;	/* wrap, if needed */
    if (position == 0)
      rand_flip_bits();		/* when wrapped, flip once */
  }

  /*
   * Ok, now retrieve the entropy requested!
   */
  rand_flip_bits();
  for (i = 0; i < num; i += SHA_DIGEST_LENGTH) {
    SHA1_init(&c);
    /*
     * Read back entropy from the pool while taking account of wrap around
     * effects. First read 20bytes from "left of the current position",
     * then 64bytes starting at position.
     */
    if (position == 0)
      SHA1_update(&c, pool + pool_size - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    else
      SHA1_update(&c, pool + position - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    
    end = position + 64;
    if (end < pool_size)
      SHA1_update(&c, pool + position, 64);
    else {
      end %= pool_size;
      SHA1_update(&c, pool + position, pool_size - position);
      SHA1_update(&c, pool, end);
    }

    SHA1_final(&c);
    SHA1_digest(&c, temp_md);
    for (j = 0; j < SHA_DIGEST_LENGTH; j++)
      pool[position + j] ^= temp_md[j];
    position += SHA_DIGEST_LENGTH;
    position %= pool_size;	/* wrap, if needed */
    for (j = i; (j < num) && (j - i < SHA_DIGEST_LENGTH); j++)
      buf[j] = temp_md[j - i];
  }

  /*
   * Now that the entropy has been retrieved, the pool is mixed again.
   * If somebody could gain access to the memory, he would not find any
   * (useful) traces of the pool state left.
   */
  rand_flip_bits();
  for (i = 0; i < pool_size; i += SHA_DIGEST_LENGTH) {
    SHA1_init(&c);
    if (position == 0)
      SHA1_update(&c, pool + pool_size - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    else
      SHA1_update(&c, pool + position - SHA_DIGEST_LENGTH, SHA_DIGEST_LENGTH);
    
    end = position + 64;
    if (end < pool_size)
      SHA1_update(&c, pool + position, 64);
    else {
      end %= pool_size;
      SHA1_update(&c, pool + position, pool_size - position);
      SHA1_update(&c, pool, end);
    }

    SHA1_final(&c);
    SHA1_digest(&c, temp_md);
    for (j = 0; j < SHA_DIGEST_LENGTH; j++)
      pool[position + j] ^= temp_md[j];
    position += SHA_DIGEST_LENGTH;
    position %= pool_size;	/* wrap, if needed */
    if (position == 0)
      rand_flip_bits();		/* when wrapped, flip once */
  }

  (void)memset(&c, 0, sizeof(c));
  (void)memset(temp_md, 0, SHA_DIGEST_LENGTH);

  /*
   * When entropy was retrieved, reduce the "entropy counter". Do not go
   * below 0, because this would not make sense.
   */
  entropy -= num;
  if (entropy < 0)
    entropy = 0;

  return 1;
}

int rand_pool(void)
{

 /*
  * Return the amount of (weighted) entropy available in bits. If not
  * enough entropy is available yet, return "0" to flag that a call to
  * rand_bytes() would fail.
  */

  if (!pool_seeded)
    return 0;
  else
    return (int)(entropy * BITS_PER_BYTE);
}

void rand_init(int set_pool_size, int set_entropy_needed)
{
  /*
   * Set the pool size. For simplicity (we must be able to walk back and
   * forward), the pool_size shall be a multiple of SHA_DIGEST_LENGTH.
   * In order to work appropriately (we always mix 20+64=84bytes) the
   * minimum _technical_ pool size is set to 100bytes.
   */
  pool_size = set_pool_size;
  if (pool_size < 100)
    pool_size = 100;
  if (pool_size % SHA_DIGEST_LENGTH)
    pool_size = (pool_size / SHA_DIGEST_LENGTH + 1) * SHA_DIGEST_LENGTH;
  pool = my_malloc(pool_size);

  /*
   * Remember the minimum seed needed.
   */
  entropy_needed = set_entropy_needed;
}
