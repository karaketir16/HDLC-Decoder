/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "pico/multicore.h"
#include "hardware/uart.h"

#include <vector>

/// \tag::hello_uart[]

#define UART_ID uart0
#define BAUD_RATE 1000000

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define HDLC_CLK 14
#define HDLC_DATA 15

static volatile uint8_t consecutive_one_counter = 0;

void gpio_callback(uint gpio, uint32_t events);

volatile enum State {
    e_IDLE,
    e_DATA
} state;

void parse_bit(bool bit){
    volatile static uint8_t bit_counter = 0;
    volatile static uint8_t val = 0;

    if(consecutive_one_counter >= 6){//
        consecutive_one_counter = 6;
        state = e_IDLE;
        if(bit_counter != 6){//sync itself to flag
            val = 0x7E;
            bit_counter = 7;
            return;
        }
    }

    val |= (bit << (bit_counter++));

    if(bit_counter == 8){

        if(val != 0x7E){
            state = e_DATA;
        }

        if(state == e_DATA){
            uart_putc_raw(UART_ID, val);
        }
        
        bit_counter = 0;
        val = 0;
    }
}

void gpio_callback(uint gpio, uint32_t events) {

    if(events & GPIO_IRQ_EDGE_RISE){
        bool val = gpio_get(HDLC_DATA);

        if(consecutive_one_counter == 5 && val == false){//inserted zero
            consecutive_one_counter = 0;
            return;//discard
        }

        if(val){
            consecutive_one_counter++;
        } else {
            consecutive_one_counter=0;
        }

        parse_bit(val);
    }
}

void core1_entry(){

}

int main() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    uart_putc_raw(UART_ID, 'A');


    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(500);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
    sleep_ms(500);

    
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select

    gpio_init(HDLC_DATA);
    gpio_set_dir(HDLC_DATA, GPIO_IN);
    gpio_set_irq_enabled_with_callback(HDLC_CLK, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    
    
    while (1){
        tight_loop_contents();
    }

    return 0;
}
