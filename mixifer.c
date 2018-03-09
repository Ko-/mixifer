/* Wrapper to test and benchmark the assembly implementation of Mixifer in
 * mixifer.s. The CC0 license applies to this code.
 *
 * Nov 2017.
*/

#include "common/stm32wrapper.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void mixifer_permute(uint8_t *);
extern void mixifer_invpermute(uint8_t *);

int main(void)
{
    clock_setup();
    gpio_setup();
    usart_setup(115200);
    flash_setup();
    cyccnt_setup();

    uint8_t data[32] = {0,0,0,0,1,2,3,1,2,4,1,2,5,1,2,6,1,2,7,1,2,8,1,2,9,1,2,10,1,2,11,1};
    char buffer[68];
    int i;


    //Print plaintext
    sprintf(buffer, "plaintext: ");
    send_USART_str(buffer);
    for(i=0;i<32;++i)
        sprintf(buffer+2*i, "%02x", data[i]);
    send_USART_str(buffer);


    unsigned int oldcount = DWT_CYCCNT;
    mixifer_permute(data);
    unsigned int cyclecount = DWT_CYCCNT-oldcount;

    sprintf(buffer, "cyc: %d", cyclecount);
    send_USART_str(buffer);


    //Print ciphertext
    sprintf(buffer, "ciphertext: ");
    send_USART_str(buffer);
    for(i=0;i<32;++i) {
        sprintf(buffer+2*i, "%02x", data[i]);
    }
    send_USART_str(buffer);


    oldcount = DWT_CYCCNT;
    mixifer_invpermute(data);
    cyclecount = DWT_CYCCNT-oldcount;

    sprintf(buffer, "cyc: %d", cyclecount);
    send_USART_str(buffer);


    //Print plaintext
    sprintf(buffer, "plaintext: ");
    send_USART_str(buffer);
    for(i=0;i<32;++i)
        sprintf(buffer+2*i, "%02x", data[i]);
    send_USART_str(buffer);

    while (1);

    return 0;
}
