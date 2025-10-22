/* Aplicação de comunicação LoRaWAN e sensores */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"

/* Includes dos módulos */
#include "lorawan/lorawan.h"
#include "sensor_ultrassonico/sensor_ultrassonico.h"
#include "deteccao_tamper/deteccao_tamper.h"

/* Definições - deep sleep */
#define FATOR_US_PARA_S   (uint64_t )1000000
#define TEMPO_EM_SLEEP    (uint64_t)1800

/* Definições - motivos de wake-up (para envio LoRaWAN) */
#define MOTIVO_WAKEUP_TAMPER             0x01
#define MOTIVO_WAKEUP_TIMER              0x02
#define MOTIVO_WAKEUP_DESCONHECIDO       0x03


/* Tag de debug */
static const char* TAG_LOGS_LORAWAN_SENSORES = "LIXO_LORAWAN";

/* Protótipos */
static void le_sensor_e_envia_lorawan(void *arg);
static void configura_wake_up_e_entra_deep_sleep(void);
static esp_sleep_wakeup_cause_t obtem_motivo_wake_up(void);

/* Função: configura fontes de wake-up para o ESP32 e entra em deep sleep
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
static void configura_wake_up_e_entra_deep_sleep(void)
{
    /* Variáveis para o deep sleep */
    uint64_t tempo_em_sleep_us = 0;

    ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "entrando em modo deep sleep por %lld segundos\n", TEMPO_EM_SLEEP);
    tempo_em_sleep_us = FATOR_US_PARA_S * TEMPO_EM_SLEEP;

    /* Configura fonte de wake-up como timer e GPIO de tamper indo para nivel alto e entra em deep-sleep */
    esp_sleep_enable_ext0_wakeup(GPIO_TAMPER, 1);
    esp_sleep_enable_timer_wakeup(tempo_em_sleep_us);    
    esp_deep_sleep_start();
}

/* Função: obtem motivo do wake-up do ESP32
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
static esp_sleep_wakeup_cause_t obtem_motivo_wake_up(void)
{
    esp_sleep_wakeup_cause_t motivo_wakeup;

    /* Obtem motivo do ESP32 acordar / ligar */
    motivo_wakeup = esp_sleep_get_wakeup_cause();
    switch(motivo_wakeup)
    {
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Motivo do wake-up: tamper acionado");
            motivo_wakeup = MOTIVO_WAKEUP_TAMPER;
            break;   

        case ESP_SLEEP_WAKEUP_TIMER: 
            ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Motivo do wake-up: timer");
            motivo_wakeup = MOTIVO_WAKEUP_TIMER;
            break;

        default: 
            ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Motivo do wake-up: desconhecido");
            motivo_wakeup = MOTIVO_WAKEUP_DESCONHECIDO;
            break;
    }

    return motivo_wakeup;
}

static void le_sensor_e_envia_lorawan(void *arg)
{             
    esp_sleep_wakeup_cause_t motivo_wakeup;         
    TConfig_LoRaWAN config_lorawan;          /* Variável de configs  do modulo LoRaWAN */
    float distancia = 0.0;                   /* Variável relativa a distancia medida */    
    TConfig_sensores config_sensores;        /* Variável ralativa a config aos sensores */
    char payload_lorawan[7] = {0};           /* Variável para compor payload */       
    int64_t timestamp_aguarda;               /* Variável par temporização não blocante */

    /* Obtem motivo do wake-up do ESP32 */    
    motivo_wakeup = obtem_motivo_wake_up();  

    /* Configura tamper */     
    configura_tamper();

    /* Se o motivo do wake-up foi tamper, aguarda o tamper ser desfeito para prosseguir */
    if (motivo_wakeup == MOTIVO_WAKEUP_TAMPER)
    {       
        vTaskDelay(pdMS_TO_TICKS(1000));

        while (1)
        {
            ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Aguardando tamper ser desfeito..."); 

            /* Le tamper com debounce */
            if (le_tamper() == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(TEMPO_DEBOUNCE_TAMPER));
                if (le_tamper() == 0)
                {
                    break;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Tamper desfeito."); 
    }

    /*  
     *   Configuração do módulo LoRaWAN
     */
    inicializa_uart_lorawan();    

    memset(config_lorawan.APPSKEY, 0x00, sizeof(config_lorawan.APPSKEY));
    memset(config_lorawan.NWSKEY, 0x00, sizeof(config_lorawan.NWSKEY));
    memset(config_lorawan.DEVADDR, 0x00, sizeof(config_lorawan.DEVADDR));
    memset(config_lorawan.CHMASK, 0x00, sizeof(config_lorawan.CHMASK));

    /* Substitua as credenciais abaixo pelas suas, credenciais estas fornecidas pelo 
     * seu distribuidor LoRaWAN
     */
    snprintf(config_lorawan.APPSKEY, sizeof(config_lorawan.APPSKEY), "C2:5D:FF:5F:54:20:A3:DE:75:CE:A2:35:29:23:6E:1C"); 
    snprintf(config_lorawan.NWSKEY, sizeof(config_lorawan.NWSKEY), "BB:82:66:2E:3F:5C:A5:A1:FB:02:22:8C:89:75:6D:5D"); 
    snprintf(config_lorawan.DEVADDR, sizeof(config_lorawan.DEVADDR), "26:0D:FF:D6"); 
    snprintf(config_lorawan.CHMASK, sizeof(config_lorawan.CHMASK), "0100:0000:0000:0000:0000:0000"); 
    
    config_lorawan.confirmacao_de_envio = LORAWAN_ENVIO_SEM_CONFIRMACAO;
    config_lorawan.join_mode = LORAWAN_JOIN_MODE_ABP;
    config_lorawan.adr = LORAWAN_ADR_DESABILITADO;
    config_lorawan.dr = LORAWAN_DR_NIVEL_5;
    config_lorawan.classe = LORAWAN_CLASSE_A;
    
    configurar_lorawan(&config_lorawan);

    /* Simula a leitura de sensores */
    timestamp_aguarda = esp_timer_get_time()/1000;

    while( ( (esp_timer_get_time()/1000) - timestamp_aguarda) <= 5000 )
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /*  
     *   Configura sensores
     */
    config_sensores.gpio_echo = 33;
    config_sensores.gpio_trigger = 25;
    config_sensores.gpio_liga_desliga = 21;    
    //inicializa_sensor(&config_sensores);
    
    //ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Sensor configurado");
    //ESP_LOGI(TAG_LOGS_LORAWAN_SENSORES, "Lendo sensor...");
    //le_sensor(&config_sensores, &distancia); 
    
    /* Monta payload e o envia por LoRaWAN */
    memset(payload_lorawan, 0x00, sizeof(payload_lorawan));
    snprintf(payload_lorawan, sizeof(payload_lorawan), "010203");
    envia_payload_lorawan(payload_lorawan);      
    
    /* Aguarda para desligar sensores e ter certeza que payload foi enviado */
    timestamp_aguarda = esp_timer_get_time()/1000;

    while( ( (esp_timer_get_time()/1000) - timestamp_aguarda) <= 5000 )
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /* Configura fontes de wake-up para o ESP32 e entra em deep sleep */
    configura_wake_up_e_entra_deep_sleep();
}

void app_main(void)
{
    xTaskCreate(le_sensor_e_envia_lorawan, "SENSOR_LORAWAN", CONFIG_SENSORES_LORAWAN_TASK_STACK_SIZE, NULL, 10, NULL);
}
