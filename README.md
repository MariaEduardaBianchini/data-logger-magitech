# Data Logger de Luminosidade, Temperatura e Umidade ☀️🌡️💧

Bem-vindo(a) ao repositório do **Data Logger** que monitora luminosidade, temperatura e umidade, exibindo tudo em um **LCD** e registrando dados de anomalias na **EEPROM**.  
Esse projeto conta com recursos de **menu interativo**, **conversão de escalas de temperatura**, **alertas com LEDs** e muito mais!  

---

## ✨ Principais Destaques

1. **Menu Principal Intuitivo**  
   - Navegue usando botões físicos para:  
     - Alterar a **escala de temperatura** (Celsius, Fahrenheit ou Kelvin).  
     - Exibir a **tela HOME** com valores em tempo real.  
     - Mostrar o **RTC** (Relógio de Tempo Real) configurado com fuso horário.  

2. **Conversão de Escalas de Temperatura**  
   - É possível alterar entre as escalas rapidamente.  
   - A conversão é feita no próprio código para que as leituras sejam adaptadas à preferência do usuário.  

3. **Alertas Visuais e Sonoros**  
   - **LEDs** coloridos indicam diferentes condições de anomalia:  
     - LED **VERDE** para temperatura fora do intervalo configurado.  
     - LED **VERMELHO** para umidade fora do intervalo.  
     - LED **AMARELO** para luminosidade fora do intervalo.  
   - **Buzzer** para alerta sonoro quando valores estão críticos.  

4. **Registro de Anomalias na EEPROM**  
   - Sempre que ocorre uma medição fora dos limites, o evento é armazenado na memória EEPROM.  
   - Informações como data/hora (via RTC), temperatura, umidade e luminosidade são registradas.  

5. **Interface LCD**  
   - O projeto utiliza um display LCD (16x2) para exibição dos menus, valores medidos e animações iniciais.  
   - Interface amigável e fácil de visualizar.  

---

## 🛠️ Tecnologias e Bibliotecas Utilizadas

- **Arduino** (IDE ou PlatformIO)
- **Linguagem C/C++** para microcontroladores
- **Bibliotecas**:
  - [LiquidCrystal_I2C](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)  
  - [Wire](https://www.arduino.cc/en/reference/wire)  
  - [DHT](https://github.com/adafruit/DHT-sensor-library)  
  - [RTClib](https://github.com/adafruit/RTClib)  
  - [EEPROM](https://www.arduino.cc/en/Reference/EEPROM)  

---

## ⚙️ Como o Projeto Funciona

1. **Leitura de Sensores**  
   - **DHT11** para temperatura e umidade.  
   - **LDR** (sensor de luz) conectado na entrada analógica A0.  

2. **Exibição de Dados**  
   - Valores médios de temperatura e umidade são atualizados a cada 10 leituras.  
   - O valor de luminosidade é convertido de 0 a 100%.  

3. **Menu de Navegação**  
   - Há quatro botões: **UP**, **DOWN**, **SELECT**, **BACK**.  
   - O menu principal possui 3 opções básicas:  
     1. **Escala de Temperatura**  
     2. **HOME**  
     3. **RTC**  

4. **Alertas**  
   - Se qualquer valor (temperatura, umidade ou luminosidade) ultrapassar os limites definidos, um LED correspondente acende e o buzzer emite som.  

5. **Registro na EEPROM**  
   - A cada minuto, se for detectada anomalia, os valores são gravados na EEPROM com base no **timestamp** (RTC).  
   - O código gerencia o endereço de escrita para não sobrescrever registros anteriores.  

---

## 🚀 Montagem do Circuito

- **RTC DS3231**: Comunicação I2C (pinos SDA, SCL).  
- **LCD 16x2 I2C**: Mesmo barramento I2C (GND, VCC, SDA, SCL).  
- **DHT11**: Pino de sinal no **D3** (ou conforme define o `#define DHTPIN 3`).  
- **LDR**: Conectado ao pino **A0** com um resistor pull-down ou pull-up (divisor de tensão).  
- **Botões**: Cada botão possui `INPUT_PULLUP`.  
- **Buzzer**: Ligado ao pino **13**.  
- **LEDs**:  
  - **LED_RED** no pino **2**  
  - **LED_YEL** no pino **4**  
  - **LED_GRE** no pino **5**  

> **Dica:** Consulte o diagrama de pinos no próprio código para detalhes completos de cada conexão.

---

## 🔥 Como Usar

1. **Clonar o Repositório**  
   ```bash
   git clone https://github.com/seu-usuario/SeuDataLogger.git
