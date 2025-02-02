/*
 * Exemplo de solução para o projeto:
 *  - Matriz 5x5 de LEDs WS2812 (buffer para seleção dos LEDs acesos)
 *  - LED RGB (apenas o canal vermelho) piscando a 5 Hz
 *  - Botão A (GPIO 5): incrementa o dígito exibido (0 a 9)
 *  - Botão B (GPIO 6): decrementa o dígito exibido (0 a 9)
 *
 * Por: Heitor Rodrigues Lemos Dias
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "ws2812.pio.h"  

// ================================
// DEFINIÇÕES DE CONSTANTES E PINOS
// ================================
#define IS_RGBW             false
#define NUM_PIXELS          25
#define WS2812_PIN          7

#define LED_RGB_VERMELHO_PIN  12   // LED vermelho do LED RGB
#define BOTAO_A_PIN           5    // Botão A (incrementa)
#define BOTAO_B_PIN           6    // Botão B (decrementa)

#define ATRASO_DEBOUNCE_US    50000      // 50 ms para debounce
#define INTERVALO_PISCA_LED_MS 100      // Intervalo de 100 ms (5 Hz de toggle)

#define COR_WS2812_R      0   // Intensidade do vermelho na matriz
#define COR_WS2812_G      0   // Intensidade do verde na matriz
#define COR_WS2812_B      200 // Intensidade do azul na matriz

// ================================
// VARIÁVEIS GLOBAIS
// ================================
volatile int digito_atual = 0;          // Dígito atualmente exibido (0 a 9)
volatile bool atualizar_exibicao = false;      // Flag para atualizar a exibição

// Variáveis para controle do debounce
volatile absolute_time_t ultimo_debounce_A;
volatile absolute_time_t ultimo_debounce_B;

// Buffer para armazenar quais LEDs estão acesos na matriz 5x5
// Cada posição do buffer corresponde a um LED: true = LED aceso, false = LED apagado.
bool buffer_leds[NUM_PIXELS] = { false };

// ======================================
// PADRÕES DOS DÍGITOS (MATRIZ 5x5) – ESTILO DIGITAL
// Cada padrão é uma matriz 5x5 de valores booleanos.
const bool padroes_digitos[10][5][5] = {
    // Dígito 0
    { {true,  true,  true,  true,  true},
      {true,  false, false, false, true},
      {true,  false, false, false, true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true} },
    // Dígito 1
    { {false, false, true,  false, false},
      {false, true,  true,  false, false},
      {true,  false, true,  false, false},
      {false, false, true,  false, false},
      {false, false, true,  false, false} },
    // Dígito 2
    { {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {true,  true,  true,  true,  true},
      {true,  false, false, false, false},
      {true,  true,  true,  true,  true} },
    // Dígito 3
    { {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {true,  true,  true,  true,  true} },
    // Dígito 4
    { {true,  false, false, false, true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {false, false, false, false, true} },
    // Dígito 5
    { {true,  true,  true,  true,  true},
      {true,  false, false, false, false},
      {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {true,  true,  true,  true,  true} },
    // Dígito 6
    { {true,  true,  true,  true,  true},
      {true,  false, false, false, false},
      {true,  true,  true,  true,  true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true} },
    // Dígito 7
    { {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {false, false, false, true,  false},
      {false, false, true,  false, false},
      {false, false, true,  false, false} },
    // Dígito 8
    { {true,  true,  true,  true,  true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true} },
    // Dígito 9
    { {true,  true,  true,  true,  true},
      {true,  false, false, false, true},
      {true,  true,  true,  true,  true},
      {false, false, false, false, true},
      {true,  true,  true,  true,  true} }
};

// ================================
// FUNÇÕES AUXILIARES PARA OS WS2812
// ================================

/**
 * @brief Envia um pixel (cor em formato GRB) para o WS2812 via PIO.
 *
 * A função desloca o valor (<< 8) conforme o esperado pelo programa PIO.
 */
static inline void enviar_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

/**
 * @brief Converte componentes R, G e B em um valor de 32 bits (formato GRB).
 */
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

/**
 * @brief Envia a cor para cada LED da matriz de acordo com o buffer.
 *
 * Para cada posição do buffer:
 *   - Se true: envia a cor definida (acende o LED);
 *   - Se false: envia 0 (desliga o LED).
 *
 * Após enviar os 25 pixels, aguarda ~60 µs para "travar" os dados.
 */
void definir_leds(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t cor = urgb_u32(r, g, b);
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (buffer_leds[i])
            enviar_pixel(cor);
        else
            enviar_pixel(0);
    }
    sleep_us(60);  // Aguarda para fixar os dados no WS2812
}

/**
 * @brief Atualiza o buffer de LEDs (buffer_leds) com o padrão do dígito.
 *
 * @param digito Dígito (0 a 9) a ser exibido.
 */
void atualizar_buffer_com_digito(int digito) {
    for (int linha = 0; linha < 5; linha++) {
        // Inverte o índice da linha para que o padrão não fique de cabeça para baixo
        int linha_fisica = 4 - linha;
        for (int coluna = 0; coluna < 5; coluna++) {
            int indice;
            // Se a linha física for ímpar, invertemos a ordem das colunas (arranjo serpentina)
            if (linha_fisica % 2 == 1) {
                indice = linha_fisica * 5 + (4 - coluna);
            } else {
                indice = linha_fisica * 5 + coluna;
            }
            // Mapeia o padrão do dígito para o buffer
            buffer_leds[indice] = padroes_digitos[digito][linha][coluna];
        }
    }
}



// ================================
// ROTINA DE INTERRUPÇÃO (Callback GPIO)
// ================================
/**
 * @brief Callback chamado quando ocorre a interrupção em um dos botões.
 *
 * Verifica qual botão foi pressionado e, se o tempo de debounce for respeitado,
 * atualiza o dígito (incrementa para o botão A e decrementa para o botão B).
 */
void callback_gpio(uint gpio, uint32_t eventos) {
    absolute_time_t agora = get_absolute_time();

    if (gpio == BOTAO_A_PIN) {
        if (absolute_time_diff_us(ultimo_debounce_A, agora) > ATRASO_DEBOUNCE_US) {
            digito_atual = digito_atual + 1;
            if (digito_atual > 9)
                digito_atual = 0;
            atualizar_exibicao = true;
            ultimo_debounce_A = agora;
        }
    } else if (gpio == BOTAO_B_PIN) {
        if (absolute_time_diff_us(ultimo_debounce_B, agora) > ATRASO_DEBOUNCE_US) {
            digito_atual = digito_atual - 1;
            if (digito_atual < 0)
                digito_atual = 9;
            atualizar_exibicao = true;
            ultimo_debounce_B = agora;
        }
    }
}

// ================================
// CALLBACK PARA O TIMER DO LED (5 Hz)
// ================================
/**
 * @brief Callback do timer repetitivo que alterna o estado do LED RGB vermelho.
 *
 * Este callback é chamado a cada 100 ms, fazendo com que o LED pisque a 5 Hz.
 */
bool callback_piscar_led(struct repeating_timer *t) {
    static bool estado_led = false;
    estado_led = !estado_led;
    gpio_put(LED_RGB_VERMELHO_PIN, estado_led);
    return true;
}

// ================================
// FUNÇÃO MAIN
// ================================
int main() {
    stdio_init_all();

    // ----------------------------
    // Configuração do LED RGB (vermelho)
    // ----------------------------
    gpio_init(LED_RGB_VERMELHO_PIN);
    gpio_set_dir(LED_RGB_VERMELHO_PIN, GPIO_OUT);
    gpio_put(LED_RGB_VERMELHO_PIN, 0);

    // ----------------------------
    // Configuração dos Botões (com pull-up interna)
    // ----------------------------
    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);
    
    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);

    // Configura as interrupções: detecção na borda de descida (botão pressionado)
    gpio_set_irq_enabled_with_callback(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true, callback_gpio);
    gpio_set_irq_enabled(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // ----------------------------
    // Inicialização dos WS2812 via PIO
    // ----------------------------
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Exibe inicialmente o dígito 0 na matriz WS2812
    atualizar_buffer_com_digito(digito_atual);
    definir_leds(COR_WS2812_R, COR_WS2812_G, COR_WS2812_B);

    // ----------------------------
    // Delay Não Bloqueante
    // ----------------------------
    absolute_time_t proximo_toggle = get_absolute_time();
    bool estado_led = false;

    // ----------------------------
    // Loop principal
    // ----------------------------
    while (true) {
        // Verifica se já passou o intervalo definido (100 ms = 100.000 µs)
        absolute_time_t agora = get_absolute_time();
        if (absolute_time_diff_us(proximo_toggle, agora) >= (INTERVALO_PISCA_LED_MS * 1000)) {
            // Alterna o estado do LED
            estado_led = !estado_led;
            gpio_put(LED_RGB_VERMELHO_PIN, estado_led);
            // Atualiza o instante para o próximo toggle
            proximo_toggle = delayed_by_ms(proximo_toggle, INTERVALO_PISCA_LED_MS);
        }

        // Verifica se houve atualização de exibição do dígito
        if (atualizar_exibicao) {
            atualizar_buffer_com_digito(digito_atual);
            definir_leds(COR_WS2812_R, COR_WS2812_G, COR_WS2812_B);
            atualizar_exibicao = false;
        }
        tight_loop_contents();  // Minimiza o consumo de CPU no loop principal
    }

    return 0;
}

