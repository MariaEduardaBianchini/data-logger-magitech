# Projeto Arduino: Data Logger Magitech 🚀🌡️📅

Este projeto é um sistema avançado de monitoramento dos padrões ambientais utilizando Arduino Uno R3. Ele integra um módulo RTC DS3231 para controle de data/hora, o sensor DHT11 para medições de temperatura e umidade, um sensor LDR para leitura de luminosidade e utiliza a EEPROM para registrar anomalias. Tudo é exibido em um LCD I2C com um menu interativo e animações divertidas.

---

## Funcionalidades Principais 🔥

- **Leitura de Sensores:**
  - **Temperatura e Umidade:**  
    O sensor DHT11 mede a temperatura (com conversão para Celsius, Fahrenheit ou Kelvin) e a umidade.  
  - **Luminosidade:**  
    O sensor LDR mede o nível de luminosidade ambiente.

- **Controle do Tempo com RTC:**
  - Integração com o módulo RTC DS3231 para obter data e hora atual.
  - Ajuste de fuso horário (UTC-3, configurável).

- **Registro de Anomalias na EEPROM:**
  - Armazena registros (timestamp, temperatura e umidade) sempre que os valores estiverem fora dos limites definidos.
  
- **Interface no LCD I2C:**
  - Exibe dados em tempo real com animações, menu interativo e tela “home”.
  - Permite a navegação para configuração da escala de temperatura e exibição do relógio (RTC).

- **Alertas Visuais e Sonoros:**
  - **LEDs:** Indicam condições fora dos limites (ex.: LED vermelho para umidade fora da faixa de 40%-65%).  
  - **Buzzer:** Emite som para alertar sobre anomalias.

- **Monitor Serial:**
  - Imprime periodicamente os dados lidos e os registros de anomalias para depuração.

---

## Componentes Necessários 🛠️

- **Arduino:** (ex.: Arduino Uno ou compatível)
- **Display LCD I2C:** (16x2) 🖥️
- **Sensor DHT22:** Para medições de temperatura e umidade 🌡️
- **Módulo RTC DS3231:** Para controle de data/hora 📅
- **EEPROM:** Para armazenamento dos registros (interna ou módulo externo) 💾
- **Sensor LDR:** Para medições de luminosidade 💡
- **Botões:** Quatro botões para navegação (UP, DOWN, SELECT, BACK) 🔘
- **LEDs:** Três LEDs (vermelho, amarelo e verde) para alertas 🔴🟡🟢
- **Buzzer:** Para alertas sonoros 🔔
- **Protoboard e Jumpers:** Para montagem do circuito 🔌

---

## Conexões ⚡

- **LCD I2C:**  
  - SDA/SCL: Conecte aos pinos I2C do Arduino (ex.: A4 e A5 no Arduino Uno)

- **Sensor DHT22:**  
  - Pino de Dados: Conecte ao pino digital 9

- **Módulo RTC DS3231:**  
  - Conecte ao barramento I2C (SDA e SCL)

- **Sensor LDR:**  
  - Pino Analógico: Conecte ao A0 (utilize um divisor de tensão se necessário)

- **Botões:**  
  - UP_BUTTON: Pino 3  
  - DOWN_BUTTON: Pino 4  
  - SELECT_BUTTON: Pino 5  
  - BACK_BUTTON: Pino 2

- **LEDs:**  
  - LED_RED: Pino 8  
  - LED_YEL: Pino 7  
  - LED_GRE: Pino 6

- **Buzzer:**  
  - BUZZER_PIN: Pino 13

- **EEPROM:**  
  - Utilize a EEPROM interna ou um módulo externo conforme a necessidade.

---

## Instalação e Configuração ⚙️

1. **Instale as Bibliotecas Necessárias:**
   - [LiquidCrystal_I2C](https://github.com/johnrickman/LiquidCrystal_I2C)
   - [DHT Sensor Library](https://github.com/adafruit/DHT-sensor-library)
   - [RTClib](https://github.com/adafruit/RTClib)
   - A biblioteca **EEPROM** geralmente já está incluída no Arduino IDE.

2. **Montagem do Circuito:**
   - Conecte os componentes conforme descrito na seção de Conexões.

3. **Upload do Código:**
   - Abra o código no Arduino IDE, selecione a placa e a porta correta.
   - Faça o upload para o Arduino.

---

## Estrutura do Código 📂

- **Includes & Defines:**  
  - Inclusão das bibliotecas para LCD, RTC, DHT e EEPROM.
  - Definição de constantes como endereços, dimensões, pinos, triggers para temperatura, umidade (atualizados para 40%-65%) e luminosidade.

- **Objetos & Variáveis Globais:**  
  - Instanciação do LCD, DHT, RTC e variáveis para controle de menu, leituras e alertas.
  - Configuração da EEPROM para registro dos dados de anomalia.

- **Setup:**  
  - Inicializa a comunicação serial, o RTC, EEPROM, sensores e configura os pinos.
  - Executa animações de introdução e exibe o menu principal.

- **Loop Principal:**  
  - Lê os sensores (DHT e LDR) e atualiza os valores.
  - Calcula a média das últimas 10 leituras para temperatura e umidade.
  - Registra anomalias na EEPROM com timestamp via RTC.
  - Atualiza o display e gerencia o menu (incluindo o menu do RTC).
  - Imprime os dados no monitor serial.

- **Funções Auxiliares:**  
  - **Alertas:** Verifica os limites dos sensores para acionar LEDs e buzzer.  
  - **Menu e Animações:** Exibe o menu, submenus e animações (efeitos “mago” e “MAGITECH!”).  
  - **EEPROM & Registro:** Funções para gravar e consultar registros na EEPROM.  
  - **RTC:** Funções para exibir data/hora e ajustar o fuso horário.

---

## Uso 📖

- **Navegação no Menu:**
  - Use os botões UP e DOWN para alternar entre as opções.
  - Pressione SELECT para confirmar uma escolha.
  - Utilize BACK para retornar ao menu anterior.

- **Configuração da Escala de Temperatura:**
  - No submenu, escolha entre Celsius, Fahrenheit ou Kelvin.

- **Tela Home:**
  - Exibe os valores médios de temperatura, umidade e luminosidade (atualizados a cada segundo).

- **Menu do RTC:**
  - Mostra a data e hora atuais obtidas via módulo RTC.
  
- **Registro de Anomalias:**
  - Se os valores lidos estiverem fora dos limites (temperatura fora de 15-25°C, umidade fora de 40-65% e luminosidade fora de 0-30%), um registro com timestamp, temperatura e umidade é gravado na EEPROM.

- **Alertas:**
  - LEDs e buzzer são acionados para indicar quando os parâmetros estão fora dos limites seguros.

- **Monitor Serial:**
  - Para depuração, os dados e registros são impressos no monitor serial (9600 baud).

---

## Observação Final 🔚

Ajuste os limites e configurações conforme o ambiente onde o sistema será utilizado.

---

## Licença 📄

Distribuído sob a [Licença MIT](LICENSE).

---

