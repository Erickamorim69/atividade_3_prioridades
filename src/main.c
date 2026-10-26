#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// Registro do módulo de Logging para este arquivo
LOG_MODULE_REGISTER(app_button, LOG_LEVEL_DBG);

#define LED_C_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec ledc = GPIO_DT_SPEC_GET(LED_C_NODE, gpios);

// Configuração do botão
#define BUTTON_NODE DT_NODELABEL(user_button_0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // Medição temporal intermediária
    uint32_t uptime_ms = k_uptime_get_32();
    uint32_t cycles = k_cycle_get_32();

    gpio_pin_toggle_dt(&ledc);

    // Logging dentro da ISR (armazenamento diferido e ultra-rápido em buffer)
    LOG_INF("Botao acionado Uptime: %u ms | Ciclos SysTick: %u", uptime_ms, cycles);
}

void main(void)
{
    LOG_INF("Pressione o botao na placa FRDM-KL25Z");
    
    // Verificar se o botão está disponível
    if (!gpio_is_ready_dt(&button)) {
        LOG_ERR("ERRO: Botao nao esta disponivel!");
        return;
    }
    if (!gpio_is_ready_dt(&ledc)) {
        LOG_ERR("ERRO: LED nao esta disponivel!");
        return;
    }
    
    gpio_pin_configure_dt(&ledc, GPIO_OUTPUT_INACTIVE);
    // Configurar botão como entrada COM pull-up forçado
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    
    LOG_INF("Botao configurado: Porto %s, Pino %d (COM PULL-UP)", 
            button.port->name, button.pin);
    
    // Configurar interrupção na borda de descida
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    while (1) {
        // Ler estado do botão
        int button_state = gpio_pin_get_dt(&button);
        button_state = !button_state;  // Inverter lógica (0=solto, 1=pressionado)
        
        LOG_DBG("Botao: %s (Estado = %d) | Uptime: %u ms | Ciclos: %u", 
                button_state ? "PRESSIONADO" : "SOLTO", button_state,
                k_uptime_get_32(), k_cycle_get_32());
        
        k_sleep(K_MSEC(300));
    }
}