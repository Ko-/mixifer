/* Reference implementation of Mixifer.
 * This code is not optimized in any way, but meant to follow the specification
 * as closely as possible. If you really want to benchmark this, be sure that
 * frequency scaling is disabled, that the process is fixed to a single CPU
 * core, etc.
 *
 * Nov 2017.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386))
#include <x86intrin.h>
#define rotr _rotr
#define rdtsc __rdtsc
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#include <intrin.h>
#define rotr _rotr
#define rdtsc __rdtsc
#else
#error This code uses two intrinsics. Either define them yourself, or use a different compiler or target architecture.
uint32_t rotr(const uint32_t a, const unsigned int n) {
    return (a >> n) | (a << (32u-n));
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

void bitslice(uint32_t * const s) {
    //involution, also unbitslices
    uint32_t copy[8] = {0};
    unsigned int i, j;
    for(i=0;i<7;i+=2) {
        for(j=0;j<32;++j) {
            copy[i+((j%8)>=4)] |= ((s[i] >> (31-j)) & 1) << (((j%4)*8)+4+(j/8));
            copy[i+((j%8)>=4)] |= ((s[i+1] >> (31-j)) & 1) << (((j%4)*8)+(j/8));
        }
    }
    memcpy(s, copy, 32);
}

void print(const uint32_t * const s) {
    unsigned int i,j,k;
    for(i=0;i<8;++i) {
        for(j=0;j<32;j+=8) {
            for(k=0;k<8;++k)
                printf("%u", (s[i] >> (j+(7-k)) & 1));
            printf((j==24) ? "  " : " ");
        }
        if(i%2)
            printf("\n");
    }
}

void print_hex(const uint32_t * const s) {
    unsigned int i,j;
    for(i=0;i<8;++i) {
        for(j=0;j<4;++j)
            printf("%02x", (s[i] >> j*8) & 0xff);
        printf((i%2) ? "\n" : " ");
    }
}

void print_short(const uint32_t * const s) {
    unsigned int i,j;
    for(i=0;i<8;++i) {
        for(j=0;j<32;j+=4)
            printf("%u", ((s[i] >> j) & 0xf) ? 1 : 0);
        printf((i%2) ? "\n" : " ");
    }
}

void mixifer_permute(uint8_t * out, const uint8_t * in) {
    const uint32_t rc = 0xf3485763;
    uint32_t state[8] = {0};
    uint32_t tmp, tmp2, tmp3;
    unsigned int i, round;

    //load data in 32-bit words
    memcpy(state, in, 32);

    DEBUGS(state);
    DEBUGF("load\n---\n");

    for(round=0;round<ROUNDS;++round) {
        //Gamma
        //fun_9c5c
        //0 c 9 d 3 a b 2 6 e 5 1 7 8 4 f
        //b0 = a1 + a2 + a0a2 + a1a2 + a1a2a3
        //b1 = a2 + a3 + a1a3 + a2a3 + a2a3a0
        //b2 = a3 + a0 + a2a0 + a3a0 + a3a0a1
        //b3 = a0 + a1 + a3a1 + a0a1 + a0a1a2
       for(i=0;i<8;++i) {
           tmp = rotr(state[i],8) | ~rotr(state[i],24);
           tmp ^= state[i];
           tmp &= rotr(state[i], 16);
           tmp ^= rotr(state[i], 24);
           state[i] = tmp;
        }

        DEBUGS(state);
        DEBUGF("s-box\n---\n");

        //Theta
        tmp = state[0] ^ state[2] ^ state[4] ^ state[6];
        tmp2 = state[1] ^ state[3] ^ state[5] ^ state[7];

        tmp3 = ((tmp2 & 0xfcfcfcfc) >> 2) | ((tmp2 & 0x03030303) << 6);
        tmp3 ^= tmp ^ tmp2;
        tmp3 = ((tmp3 & 0xfefefefe) >> 1) | ((tmp3 & 0x01010101) << 7);

        state[0] ^= tmp3;
        state[2] ^= tmp3;
        state[4] ^= tmp3;
        state[6] ^= tmp3;

        tmp3 = ((tmp & 0xfefefefe) >> 1) | ((tmp & 0x01010101) << 7);
        tmp3 ^= tmp2;
        tmp3 = ((tmp3 & 0xfefefefe) >> 1) | ((tmp3 & 0x01010101) << 7);
        tmp3 ^= tmp;

        state[1] ^= tmp3;
        state[3] ^= tmp3;
        state[5] ^= tmp3;
        state[7] ^= tmp3;

        DEBUGS(state);
        DEBUGF("cpm\n---\n");

        //Pi
        tmp = state[6];
        state[6] = state[4];
        state[4] = state[2];
        state[2] = state[0];
        state[0] = tmp;
        tmp = state[7];
        state[7] = state[5];
        state[5] = state[3];
        state[3] = state[1];
        state[1] = tmp;

        DEBUGS(state);
        DEBUGF("pi\n---\n");

        //Rho
        state[0] = rotr(state[0], 7);
        state[1] = rotr(state[1], 7);
        tmp = state[2];
        state[2] = rotr(state[3], 2);
        state[3] = rotr(tmp, 1);
        state[4] = rotr(state[4], 5);
        state[5] = rotr(state[5], 5);

        DEBUGS(state);
        DEBUGF("rho\n---\n");

        //Iota
        state[0] ^= rc >> round;

        DEBUGS(state);
        DEBUGF("roundconstant\n---\n");
    }

    DEBUGH(state);

    //store data
    memcpy(out, state, 32);
}

void mixifer_invpermute(uint8_t * out, const uint8_t * in) {
    const uint32_t rc = 0xf3485763;
    uint32_t state[8] = {0};
    uint32_t tmp, tmp2, tmp3;
    unsigned int i, round;

    //load data in 32-bit words
    memcpy(state, in, 32);

    DEBUGS(state);
    DEBUGF("load\n---\n");

    for(round=0;round<ROUNDS;++round) {
        //Iota
        state[0] ^= rc >> ((ROUNDS - 1) - round);

        DEBUGS(state);
        DEBUGF("roundconstant\n---\n");

        //Rho
        state[0] = rotr(state[0], 25);
        state[1] = rotr(state[1], 25);
        tmp = state[3];
        state[3] = rotr(state[2], 30);
        state[2] = rotr(tmp, 31);
        state[4] = rotr(state[4], 27);
        state[5] = rotr(state[5], 27);

        DEBUGS(state);
        DEBUGF("invrho\n---\n");

        //Pi
        tmp = state[0];
        state[0] = state[2];
        state[2] = state[4];
        state[4] = state[6];
        state[6] = tmp;
        tmp = state[1];
        state[1] = state[3];
        state[3] = state[5];
        state[5] = state[7];
        state[7] = tmp;

        DEBUGS(state);
        DEBUGF("invpi\n---\n");

        //Theta
        tmp = state[0] ^ state[2] ^ state[4] ^ state[6];
        tmp2 = state[1] ^ state[3] ^ state[5] ^ state[7];

        tmp3 = ((tmp2 & 0xfcfcfcfc) >> 2) | ((tmp2 & 0x03030303) << 6);
        tmp3 ^= tmp ^ tmp2;
        tmp3 = ((tmp3 & 0xfefefefe) >> 1) | ((tmp3 & 0x01010101) << 7);

        state[0] ^= tmp3;
        state[2] ^= tmp3;
        state[4] ^= tmp3;
        state[6] ^= tmp3;

        tmp3 = ((tmp & 0xfefefefe) >> 1) | ((tmp & 0x01010101) << 7);
        tmp3 ^= tmp2;
        tmp3 = ((tmp3 & 0xfefefefe) >> 1) | ((tmp3 & 0x01010101) << 7);
        tmp3 ^= tmp;

        state[1] ^= tmp3;
        state[3] ^= tmp3;
        state[5] ^= tmp3;
        state[7] ^= tmp3;

        DEBUGS(state);
        DEBUGF("cpm\n---\n");

        //Gamma
        //a0 = b0 + b1 + b3 + b0b2 + b1b2 + b1b3 + b1b2b3
        //a1 = b1 + b2 + b0 + b1b3 + b2b3 + b2b0 + b2b3b0
        //a2 = b2 + b3 + b1 + b2b0 + b3b0 + b3b1 + b3b0b1
        //a3 = b3 + b0 + b2 + b3b1 + b0b1 + b0b2 + b0b1b2
        for(i=0;i<8;++i) {
            tmp = ~rotr(state[i],8) & rotr(state[i],24);
            tmp ^= state[i];
            tmp &= ~rotr(state[i], 16);
            tmp ^= rotr(state[i], 8);
            state[i] = tmp;
        }

        DEBUGS(state);
        DEBUGF("invs-box\n---\n");
    }

    DEBUGH(state);

    //store data
    memcpy(out, state, 32);
}

int main(void)
{
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
