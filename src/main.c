#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


#define LED_C_NODE DT_ALIAS(led1)
static const struct gpio_dt_spec ledc = GPIO_DT_SPEC_GET(LED_C_NODE, gpios);

// Configuração do botão
#define BUTTON_NODE DT_NODELABEL(user_button_0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

// ISR - Toggle LED
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&ledc);
}

void main(void)
{
    printk("=== Exemplo de Botão com Pull-up ===\n");
    printk("Pressione o botão na placa FRDM-KL25Z\n");
    
    // Verificar se o botão está disponível
    if (!gpio_is_ready_dt(&button)) {
        printk("ERRO: Botão não está disponível!\n");
        return;
    }
    if (!gpio_is_ready_dt(&ledc)) {
        printk("ERRO: LED não está disponível!\n");
        return;
    }
    
    gpio_pin_configure_dt(&ledc, GPIO_OUTPUT_INACTIVE);
    // Configurar botão como entrada COM pull-up forçado
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    
    printk("Botão configurado: Porto %s, Pino %d (COM PULL-UP)\n", 
           button.port->name, button.pin);
    
        // Configurar interrupção na borda de descida
    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    
    while (1) {
        // Ler estado do botão
        int button_state = gpio_pin_get_dt(&button);
        button_state = !button_state;  // Inverter lógica (0=solto, 1=pressionado)
        
        printk("Botão: %s (Estado = %d)\n", 
               button_state ? "PRESSIONADO" : "SOLTO", button_state);
        
        k_sleep(K_MSEC(300));
    }
}