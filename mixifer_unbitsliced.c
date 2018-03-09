/* Unbitsliced reference implementation of Mixifer.
 * This code is not optimized in any way, but meant to follow the specification
 * as closely as possible. It even uses secret-dependent table lookups and is
 * therefore NOT SECURE to use. If you really want to benchmark this, be sure
 * that frequency scaling is disabled, that the process is fixed to a single
 * CPU core, etc. The CC0 license applies to this code.
 *
 * Nov 2017.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386))
#include <x86intrin.h>
#define bswap __builtin_bswap64
#define rotr __rorq
#define rdtsc __rdtsc
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
#include <stdlib.h>
#define bswap _byteswap_uint64
#define rotr _rotr64
#define rdtsc __rdtsc
#else
#error This code uses three intrinsics. Either define them yourself, or use a different compiler or target architecture.
uint64_t bswap(uint64_t a) {
    a = ((a & 0x00000000ffffffff) << 32) | ((a & 0xffffffff00000000) >> 32);
    a = ((a & 0x0000ffff0000ffff) << 16) | ((a & 0xffff0000ffff0000) >> 16);
    a = ((a & 0x00ff00ff00ff00ff) << 8)  | ((a & 0xff00ff00ff00ff00) >> 8);
    return a;
}
uint64_t rotr(const uint64_t a, const unsigned int n) {
    return (a >> n) | (a << (64u-n));
}
unsigned long long rdtsc() {
    return 0llu;
}
#endif


#define ROUNDS 16
#define DEBUG 0

#if DEBUG
    #define DEBUGF(s) printf(s)
    #define DEBUGS(s) print(s)
    #define DEBUGH(s) print_hex(s)
#else
    #define DEBUGF(s)
    #define DEBUGS(s)
    #define DEBUGH(s)
#endif

void bitslice(uint64_t * const s) {
    //involution, also unbitslices
    uint64_t copy[4] = {0};
    unsigned int i, j;
    for(i=0;i<4;++i) {
        for(j=0;j<64;++j) {
            copy[i] |= ((s[i] >> (63-j)) & 1) << (((j%8)*8)+(j/8));
        }
    }
    memcpy(s, copy, 32);
}

void print(const uint64_t * const s) {
    unsigned int i,j,k;
    for(i=0;i<4;++i) {
        for(j=0;j<64;j+=8) {
            for(k=0;k<8;++k)
                printf("%lu", (s[i] >> (j+(7-k)) & 1));
            printf((j==24) ? "  " : " ");
        }
        printf("\n");
    }
}

void print_hex(const uint64_t * const s) {
    unsigned int i,j;
    for(i=0;i<4;++i) {
        for(j=0;j<8;++j) {
            printf("%02lx", (s[i] >> j*8) & 0xff);
            if(j==3)
                printf(" ");
        }
        printf("\n");
    }
}

void print_short(const uint64_t * const s) {
    unsigned int i,j;
    for(i=0;i<4;++i) {
        for(j=0;j<64;j+=4) {
            printf("%u", ((s[i] >> j) & 0xf) ? 1 : 0);
            if(j==24)
                printf(" ");
        }
        printf("\n");
    }
}


void mixifer_permute(uint8_t * out, const uint8_t * in) {
    const uint64_t sbox[16] = {0, 0xc, 9, 0xd, 3, 0xa, 0xb, 2, 6, 0xe, 5, 1, 7, 8, 4, 0xf};
    uint64_t rc = 0x10f090502040d0d0;
    uint64_t state[4] = {0};
    uint64_t tmp;
    unsigned int i, j, round;

    //load data in 64-bit big-endian words
    memcpy(state, in, 32);

    DEBUGS(state);
    DEBUGF("load\n---\n");

    bitslice(state);
    for(i=0;i<4;++i)
        state[i] = bswap(state[i]);

    DEBUGS(state);
    DEBUGF("unbitslice\n---\n");

    for(round=0;round<ROUNDS;++round) {
        //Gamma
        for(i=0;i<4;++i) {
            tmp = 0;
            for(j=0;j<64;j+=8) {
                tmp |= sbox[(state[i] >> j) & 0xf] << j;
                tmp |= sbox[(state[i] >> (j+4)) & 0xf] << (j+4);
            }
            state[i] = tmp;
        }

        DEBUGS(state);
        DEBUGF("s-box\n---\n");

        //Theta
        tmp = state[0] ^ state[1] ^ state[2] ^ state[3];
        tmp = rotr(tmp, 4) ^ rotr(tmp, 8) ^ rotr(tmp, 20);
        state[0] ^= tmp;
        state[1] ^= tmp;
        state[2] ^= tmp;
        state[3] ^= tmp;

        DEBUGS(state);
        DEBUGF("cpm\n---\n");

        //Pi
        tmp = state[3];
        state[3] = state[2];
        state[2] = state[1];
        state[1] = state[0];
        state[0] = tmp;

        DEBUGS(state);
        DEBUGF("pi\n---\n");

        //Rho
        const uint64_t masks[3] = {0xff00000000000000, 0xfffffffffffff000, 0xffffff0000000000};
        const unsigned int dists[3] = {56, 12, 40};
        for(i=0;i<3;++i) {
            tmp = state[i] & masks[i];
            for(j=0;j<dists[i];j+=4) {
                tmp |= ((state[i] >> (j+3)) & 0x1) << j;
                tmp |= ((state[i] >> j) & 0x7) << (j+1);
            }
            state[i] = rotr(tmp, dists[i]);
        }

        DEBUGS(state);
        DEBUGF("rho\n---\n");

        //Iota
        state[0] ^= rc;
        rc = (rc >> 8) | ((((rc >> 4) << 1) & 0xf) << 60);

        DEBUGS(state);
        DEBUGF("roundconstant\n---\n");
    }

    DEBUGH(state);

    //store data
    for(i=0;i<4;++i)
        state[i] = bswap(state[i]);
    bitslice(state);
    memcpy(out, state, 32);
}


void mixifer_invpermute(uint8_t * out, const uint8_t * in) {
    const uint64_t invsbox[16] = {0, 0xb, 7, 4, 0xe, 0xa, 8, 0xc, 0xd, 2, 5, 6, 1, 3, 9, 0xf};
    uint64_t rc = 0x10f090502040d0d0;
    uint64_t state[4] = {0};
    uint64_t tmp;
    unsigned int i, j, round;

    //load data in 64-bit big-endian words
    memcpy(state, in, 32);

    DEBUGS(state);
    DEBUGF("load\n---\n");

    bitslice(state);
    for(i=0;i<4;++i)
        state[i] = bswap(state[i]);

    DEBUGS(state);
    DEBUGF("unbitslice\n---\n");

    for(round=0;round<ROUNDS;++round) {
        //Iota
        tmp = rc;
        for(i=0;i<((ROUNDS-round)-1);++i) {
            tmp = (tmp >> 8) | ((((tmp >> 4) << 1) & 0xf) << 60);
        }
        state[0] ^= tmp;

        DEBUGS(state);
        DEBUGF("roundconstant\n---\n");

        //Rho
        const uint64_t masks[3] = {0x00000000000000ff, 0x000fffffffffffff, 0x0000000000ffffff};
        const unsigned int dists[3] = {8, 52, 24};
        for(i=0;i<3;++i) {
            tmp = state[i] & masks[i];
            for(j=dists[i];j<64;j+=4) {
                tmp |= ((state[i] >> (j+1)) & 0x7) << j;
                tmp |= ((state[i] >> j) & 0x1) << (j+3);
            }
            state[i] = rotr(tmp, dists[i]);
        }

        DEBUGS(state);
        DEBUGF("invrho\n---\n");

        //Pi
        tmp = state[0];
        state[0] = state[1];
        state[1] = state[2];
        state[2] = state[3];
        state[3] = tmp;

        DEBUGS(state);
        DEBUGF("invpi\n---\n");

        //Theta
        tmp = state[0] ^ state[1] ^ state[2] ^ state[3];
        tmp = rotr(tmp, 4) ^ rotr(tmp, 8) ^ rotr(tmp, 20);
        state[0] ^= tmp;
        state[1] ^= tmp;
        state[2] ^= tmp;
        state[3] ^= tmp;

        DEBUGS(state);
        DEBUGF("cpm\n---\n");

        //Gamma
        for(i=0;i<4;++i) {
            tmp = 0;
            for(j=0;j<64;j+=8) {
                tmp |= invsbox[(state[i] >> j) & 0xf] << j;
                tmp |= invsbox[(state[i] >> (j+4)) & 0xf] << (j+4);
            }
            state[i] = tmp;
        }

        DEBUGS(state);
        DEBUGF("invs-box\n---\n");
    }

    DEBUGH(state);

    //store data
    for(i=0;i<4;++i)
        state[i] = bswap(state[i]);
    bitslice(state);
    memcpy(out, state, 32);
}


int main(void)
{
    //0000000001020301020401020501020601020701020801020901020a01020b01 unbitsliced becomes
    //000000000000060b000000000049932c00000000042069b200000000920036cb
    uint8_t in[32] = {0,0,0,0,1,2,3,1,2,4,1,2,5,1,2,6,1,2,7,1,2,8,1,2,9,1,2,10,1,2,11,1};
    uint8_t out[32];
    unsigned long long oldcount, cyclecount;
    int i;

    //Print plaintext
    printf("in: ");
    for(i=0;i<32;++i)
        printf("%02x", in[i]);
    printf("\n");


    mixifer_permute(out, in);

    oldcount = rdtsc();
    for(i=0;i<10000;++i)
        mixifer_permute(in, in);
    cyclecount = rdtsc()-oldcount;

    printf("cyc: %f\n", cyclecount/10000.0);


    //Print ciphertext
    printf("out: ");
    for(i=0;i<32;++i)
        printf("%02x", out[i]);
    printf("\n");

    mixifer_invpermute(in, out);

    oldcount = rdtsc();
    for(i=0;i<10000;++i)
        mixifer_permute(out, out);
    cyclecount = rdtsc()-oldcount;

    printf("cyc: %f\n", cyclecount/10000.0);


    //Print plaintext
    printf("in: ");
    for(i=0;i<32;++i)
        printf("%02x", in[i]);
    printf("\n");

    return 0;
}
