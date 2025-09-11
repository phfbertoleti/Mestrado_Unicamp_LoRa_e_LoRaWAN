# Mestrado na FEEC (Unicamp) - frente LoRaWAN

Aqui encontra-se os dados, experimentos e materiais produzidos referentes a parte de LoRaWAN do meu mestrado.

## Conteúdo 1: end-device LoRaWAN

O end-device LoRaWAN contido aqui utiliza como hardware:

* Um ESP32 WROOM
* Um módulo LoRaWAN Radioenge (https://www.radioenge.com.br/produto/modulo-lorawan/ )
* Uma fonte 5V/2A (pode ser substituído por powerbank)

O end-device LoRaWAN, montado no projeto de PCB contido neste repositório, pode ser visto na figura abaixo:

Foto do end-device montado: ![Foto do end-device montado)](https://raw.githubusercontent.com/phfbertoleti/Mestrado_Unicamp_LoRa_e_LoRaWAN/main/Frente_LoRaWAN/end_device/Fotos/end_device_montado.jpg)

A programação do ESP-IDF foi feita usando o VSCode, extensão ESP-IDF e ESP-IDF versão 5.4.2.

Neste momento, o end-device limita-se a enviar, a cada 30 minutos, os bytes fixos 0x01, 0x02 e 0x03 a cada 30 minutos.
Há planos de substituir os dados fixos (0x01, 0x02 e 0x03) pelo envio da leitura de uma entrada de tamper e também a leitura de uma distância medida por um sensor ultrasônico HC-SR04. Dessa forma, desde o momento atual, é preciso colocar no mesmo nível da pasta "end_device" a versão mais recente da lib https://github.com/UncleRus/esp-idf-lib .

Obervação: este end-device está preparado para operar na frequência de 916,8MHz e SF=7.

## Conteúdo 2: gateway LoRaWAN monocanal

O gateway LoRaWAN monocanal contido aqui deriva-se do seguinte projeto https://github.com/things4u/ESP-1ch-Gateway . Esta derivação contém algumas customizações para que o gateway melhor se adapte ao uo neste mestrado.
Este gateway é mostrado na figura abaixo:

Foto do gateway LoRaWAN monocanal: ![Gateway LoRaWAN monocanal dentro do case feito em impressora 3D)](https://raw.githubusercontent.com/phfbertoleti/Mestrado_Unicamp_LoRa_e_LoRaWAN/main/Frente_LoRaWAN/gateway_monocanal/Fotos/foto_gateway_monocanal_no_case.jpg)

Este gateway utiliza como hardware:

* Uma placa Heltec WiFi LoRa V2, de frequência 868-915MHz
* Uma fonte 5V/2A

Para compilar e utilizar o gateway, siga as instruções a seguir:

1. Na Arduino IDE, na tela de gerenciamento de placas, instale a versão 2.0.17 do esp32
2. Copie todas as bibliotecas / pastas em "libraries" para a pasta de bibliotecas Arduino do seu computador (exemplo: Documents\Arduino\libraries). Dessa forma, você possuirá todas as biblitoecas necessárias nas versões esperadas.
3. No arquivo configNode.h, altere a rede Wi-Fi (SSID) e senha da rede Wi-Fi na lista wpa. Se você quiser, pode adicionar quantas redes desejar nessa lista, de forma a conter redundâncias em caso de falha da rede Wi-Fi.
Após a alteração, o código do gateway pode ser compilado e, depois, gravado na placa Heltec WiFi LoRa V2. 
4. Quando o gateway inicializar, ele escreverá nas informações do Serial Monitor um ID (Gateway ID). Este ID corresponde ao endereço MAC do gateway, e deve ser usado na The Things Network (TTN) para cadastro do gateway no campo Gateway EUI.

Obervações: 
* Este gateway está preparado para operar na frequência de 916,8MHz e SF=7.
* Para abrigar este gateway LoRaWAN monocanal, foi utilizado o seguinte case, feito na impressora 3D: https://www.thingiverse.com/thing:6522462


## Vídeos explicativos

Para melhor exempificr como estabelecer o gateway LoRaWAN monocanal e o end-device LoRaWAN, existem os seguintes vídeos:

* *Gateway LoRaWAN monocanal:* explicação de parametrização de software, compilação e gravação Heltec WiFi LoRa V2: https://drive.google.com/file/d/1u9MdMoPO0pTy7fwZNUEaMupEhpbTwCIz/view?usp=sharing

* *End-device:* explicação do hardware e breve explicação da integração com The Things Network: https://drive.google.com/file/d/1UwrbAW2tfpAEE6ByUf8U8BOljeANyCgI/view?usp=sharing