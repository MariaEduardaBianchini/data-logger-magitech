# Projeto Arduino: Data Logger MAGITECH 🚀🌡️📅

Este projeto é um sistema avançado de monitoramento ambiental utilizando Arduino. Ele integra um módulo RTC DS3231 para controle de data/hora, o sensor DHT22 para medições de temperatura e umidade, um sensor LDR para leitura de luminosidade e uma EEPROM para registrar anomalias. Tudo é exibido em um LCD I2C com um menu interativo e animações divertidas.

---

## Funcionalidades Principais 🔥

- **Leitura de Sensores:**
  - **Temperatura e Umidade:**  
    O sensor DHT22 mede a temperatura (com conversão para Celsius, Fahrenheit ou Kelvin) e a umidade.  
    **Atenção:** As condições de alerta para umidade foram atualizadas – agora, se a umidade média ficar abaixo de 40% ou acima de 65%, o sistema acionará um alerta.
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
- **LEDs:** Trê
