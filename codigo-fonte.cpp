/************************************************************
 *                   INCLUDES & DEFINES                     *
 ************************************************************/
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <RTClib.h>
#include <EEPROM.h>

RTC_DS3231 rtc; //OBJETO DO TIPO RTC_DS3231
 
// COnfigurações dos dias da semana
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3
#define LOG_OPTION 1     // Opção para ativar a leitura do log

// ===============================
// ALTERADO: Ajustamos recordSize de 8 para 10
// ===============================
const int maxRecords     = 100;
const int recordSize     = 10;  // 4 bytes timestamp + 2 temp + 2 umid + 2 luminosidade
int       startAddress   = 0;
int       endAddress     = maxRecords * recordSize;
int       currentAddress = 0;

int lastLoggedMinute = -1;

// Triggers de temperatura, umidade e luminosidade
float trigger_t_min = 15.0; 
float trigger_t_max = 25.0; 
float trigger_u_min = 30.0; 
float trigger_u_max = 50.0; 
float trigger_l_min = 0.0; 
float trigger_l_max = 30.0; 

// Endereço e dimensões do LCD I2C
#define I2C_ADDR     0x27
#define LCD_COLUMNS  16
#define LCD_LINES    2

// DHT Sensor
#define DHTPIN       3       // Pino do DHT
#define DHTTYPE      DHT11   // Tipo de sensor DHT

// Botões
#define UP_BUTTON     11
#define DOWN_BUTTON   10
#define SELECT_BUTTON 9
#define BACK_BUTTON   8

// LEDs
#define LED_RED  2
#define LED_YEL  4
#define LED_GRE  5

// Buzzer
#define BUZZER_PIN     13
bool buzzerOn          = false; // Indica se o buzzer está ligado
bool buzzerTempReason  = false; // Indica se o buzzer foi ligado especificamente por causa da temperatura
bool buzzerHumdReason  = false; // Indica se o buzzer foi ligado especificamente por causa da umidade
bool buzzerLightReason = false; // Indica se o buzzer foi ligado especificamente por causa da luminosidade

// LDR
#define LDR_PIN A0

/************************************************************
 *               OBJETOS & VARIÁVEIS GLOBAIS                *
 ************************************************************/
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

// DHT
DHT dht(DHTPIN, DHTTYPE);

// Menu
int   menu              = 1;
bool  rtcMenuActive     = false;
bool  subMenuTempActive = false; 
bool  homePageActive    = false;
int   subMenuIndex      = 1;    // Índice no submenu de temperatura

// Escala de temperatura (1=Celsius, 2=Fahrenheit, 3=Kelvin)
int  temperatureScale = 1;
float lastAvgTemp; // Armazena a última média de temperatura
float lastAvgHumd; // Armazena a última média de umidade

// Leituras de sensores
float temp       = 0.0;
float humid      = 0.0;
int   lightLevel = 0;

// Arrays de leituras (média)
float tempReadings[10]; 
float humdReadings[10];
int   currentIndex = 0;

unsigned long lastHomeUpdate = 0;
long totalLeituras = 0;

DateTime adjustedTime;
DateTime now;


/************************************************************
 *                 FUNÇÃO PARA DESLIGAR ALERTAS             *
 ************************************************************/
void turnOffAllAlerts() {
  // Desliga LEDs
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GRE, LOW);
  digitalWrite(LED_YEL, LOW);

  // Desliga Buzzer
  noTone(BUZZER_PIN);

  // Zera flags
  buzzerOn          = false;
  buzzerTempReason  = false;
  buzzerHumdReason  = false;
  buzzerLightReason = false;
}


/************************************************************
 *                          SETUP                           *
 ************************************************************/
void setup() {
  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  EEPROM.begin();

  if(! rtc.begin()) {
    Serial.println("DS3231 não encontrado");
    while(1);
  }
  if(rtc.lostPower()){
    Serial.println("DS3231 OK!");
    rtc.adjust(DateTime(2025, 3, 20, 19, 30, 45));
  }
  delay(100);
  
  // Inicialização dos pinos de botões
  pinMode(UP_BUTTON,     INPUT_PULLUP);
  pinMode(DOWN_BUTTON,   INPUT_PULLUP);
  pinMode(SELECT_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON,   INPUT_PULLUP);

  // Buzzer e LEDs
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_YEL,    OUTPUT);
  pinMode(LED_GRE,    OUTPUT);

  // LDR
  pinMode(LDR_PIN, INPUT);

  // Inicializa o DHT
  dht.begin();

  // LCD
  lcd.init();
  lcd.backlight();

  // Animações de introdução
  lcd.clear();
  lcd.setCursor(0, 0);
  wizard1();
  wizard2();
  magic();
  lcd.clear();
  welcome();
  delay(1000);
  
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Loading");
  delay(1500);

  // Exibe o menu principal ao iniciar
  exibir_menu();
}


/************************************************************
 *                          LOOP                            *
 ************************************************************/
void loop() {

  // Controla a frequência da impressão serial
  static unsigned long lastSerialTime = 0;

  // ==== LEITURA DE SENSORES ====
  int valorLDR   = analogRead(LDR_PIN);
  lightLevel     = map(valorLDR, 40, 950, 0, 100);

  float tempRaw  = dht.readTemperature(); // Celsius
  float humRaw   = dht.readHumidity();

  if (!isnan(tempRaw)) {
    // Converte a temperatura conforme a escala
    switch (temperatureScale) {
      case 1: // Celsius
        temp = tempRaw;
        break;
      case 2: // Fahrenheit
        temp = (tempRaw * 1.8) + 32;
        break;
      case 3: // Kelvin
        temp = tempRaw + 273.15;
        break;
    }
  }

  if (!isnan(humRaw)) {
    humid = humRaw; // umidade em %
  }

  tempReadings[currentIndex] = temp;
  humdReadings[currentIndex] = humid;
  currentIndex = (currentIndex + 1) % 10;

  if (currentIndex == 9) {
    tenthRead();
  }

  // Exibição serial a cada ~1s
  if (millis() - lastSerialTime >= 1000UL) {
    lastSerialTime = millis();
    totalLeituras++;
    serialLog(temp, humid, valorLDR, totalLeituras);
  }

  // Verifica e registra anomalias na EEPROM
  recordEEPROM();

  if (homePageActive) {
    // Verifica luminosidade só enquanto em HOME
    checkLightAlert();

    // A cada 1s, atualiza a tela
    if (millis() - lastHomeUpdate >= 1000UL) {
      lastHomeUpdate = millis();
      showHomeValues();
    }
  }
  else {
    // Se não está na HOME, desliga todos os alertas
    turnOffAllAlerts();
  }

  // ==== SUBMENU DE TEMPERATURA ====
  if (subMenuTempActive) {
    if (!digitalRead(BACK_BUTTON)) {
      subMenuTempActive = false;
      exibir_menu();
      delay(100);
      while (!digitalRead(BACK_BUTTON));
    }

    if (!digitalRead(DOWN_BUTTON)) {
      subMenuIndex++;
      if (subMenuIndex > 3) subMenuIndex = 3;
      exibir_submenu_temp();
      delay(100);
      while (!digitalRead(DOWN_BUTTON));
    }

    if (!digitalRead(UP_BUTTON)) {
      subMenuIndex--;
      if (subMenuIndex < 1) subMenuIndex = 1;
      exibir_submenu_temp();
      delay(100);
      while (!digitalRead(UP_BUTTON));
    }

    if (!digitalRead(SELECT_BUTTON)) {
      executeActionTemp();
      subMenuTempActive = false;
      exibir_menu();
      delay(100);
      while (!digitalRead(SELECT_BUTTON));
    }
    return;
  }

  // ==== TELA HOME ====
  if (homePageActive) {
    if (!digitalRead(BACK_BUTTON)) {
      homePageActive = false;
      exibir_menu();
      delay(100);
      while (!digitalRead(BACK_BUTTON));
    }
    return;
  }

  // ==== MENU PRINCIPAL ====
  if (!digitalRead(DOWN_BUTTON)) {
    menu++;
    exibir_menu();
    delay(100);
    while (!digitalRead(DOWN_BUTTON));
  }
  if (!digitalRead(UP_BUTTON)) {
    menu--;
    exibir_menu();
    delay(100);
    while (!digitalRead(UP_BUTTON));
  }
  if (!digitalRead(SELECT_BUTTON)) {
    executeAction();
    if (!subMenuTempActive && !homePageActive) {
      exibir_menu();
    }
    delay(100);
    while (!digitalRead(SELECT_BUTTON));
  }

  // Se estiver no menu do RTC
  if (rtcMenuActive) {
    displayRTC();
    if (!digitalRead(BACK_BUTTON)) {
      rtcMenuActive = false;
      exibir_menu();
      delay(100);
      while (!digitalRead(BACK_BUTTON));
    }
    return;
  }

  if (!digitalRead(DOWN_BUTTON)) {
    menu++;
    exibir_menu();
    delay(100);
    while (!digitalRead(DOWN_BUTTON));
  }
  if (!digitalRead(UP_BUTTON)) {
    menu--;
    exibir_menu();
    delay(100);
    while (!digitalRead(UP_BUTTON));
  }
  if (!digitalRead(SELECT_BUTTON)) {
    executeAction();
    delay(100);
    while (!digitalRead(SELECT_BUTTON));
  }
}


/************************************************************
 *                       FUNÇÕES MENU                       *
 ************************************************************/
void exibir_menu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">ESCALA TEMP.");
      lcd.setCursor(0, 1);
      lcd.print(" HOME");
      break;
    case 2:
      lcd.clear();
      lcd.print(" ESCALA TEMP.");
      lcd.setCursor(0, 1);
      lcd.print(">HOME");
      break;
    case 3:
      lcd.clear();
      lcd.print(" HOME");
      lcd.setCursor(0, 1);
      lcd.print(">RTC");
      break;
    case 4:
      menu = 3;
      break;
  }
}

void exibir_submenu_temp() {
  lcd.clear();
  switch (subMenuIndex) {
    case 0:
      subMenuIndex = 1;
    case 1:
      lcd.clear();
      lcd.print(">CELSIUS");
      lcd.setCursor(0, 1);
      lcd.print(" FAHRENHEIT");
      break;
    case 2:
      lcd.clear();
      lcd.print(" CELSIUS");
      lcd.setCursor(0, 1);
      lcd.print(">FAHRENHEIT");
      break;
    case 3:
      lcd.clear();
      lcd.print(" FAHRENHEIT");
      lcd.setCursor(0, 1);
      lcd.print(">KELVIN");
      break;
    case 4:
      subMenuIndex = 3;
      break;
  }
}

void executeActionTemp() {
  switch (subMenuIndex) {
    case 1: temperatureScale = 1; break; // Celsius
    case 2: temperatureScale = 2; break; // Fahrenheit
    case 3: temperatureScale = 3; break; // Kelvin
  }
  lcd.clear();
  lcd.print("Escala defin.");
  delay(1000);
}

void executeAction() {
  switch (menu) {
    case 1:
      subMenuTempActive = true;
      subMenuIndex      = 1;
      exibir_submenu_temp();
      break;
    case 2:
      showHomePage();
      break;
    case 3:
      rtcMenuActive = true;
      lcd.clear();
      while (rtcMenuActive) {
        displayRTC();
        delay(1000);

        if (!digitalRead(BACK_BUTTON)) {
          rtcMenuActive = false;
          exibir_menu();
          delay(100);
          while (!digitalRead(BACK_BUTTON));
        }
      }
      break;
  }
}

void showHomeValues() {
  // Determina o sufixo de temperatura
  char tempSuffix = 'C';
  if (temperatureScale == 2)      tempSuffix = 'F';
  else if (temperatureScale == 3) tempSuffix = 'K';

  String tempStr  = String(lastAvgTemp, 0) + tempSuffix;
  String lightStr = String(lightLevel) + "%";
  String humStr   = String(lastAvgHumd, 0) + "%";

  // Limpa toda a linha 1 antes de imprimir novos valores
  lcd.setCursor(0, 1);
  lcd.print("                ");

  //  Exibe temp, lum, umidade
  lcd.setCursor(0, 1);
  lcd.print(tempStr);

  lcd.setCursor(6, 1);
  lcd.print(lightStr);

  lcd.setCursor(12, 1);
  lcd.print(humStr);
}

void showHomePage() {
  lcd.clear();
  homePageActive = true;
  homePage();
  lastHomeUpdate = millis();
}


/************************************************************
 *                       FUNÇÕES HOME                       *
 ************************************************************/
void homePage() {
  byte name0x1[]  = { B01110, B01010, B01010, B01010, B11111, B11111, B11111, B01110 };
  byte name0x7[]  = { B00001, B00010, B00100, B01000, B11111, B00010, B00100, B01000 };
  byte name0x13[] = { B00100, B00100, B01110, B01110, B11111, B11111, B11111, B01110 };

  lcd.createChar(0, name0x1);
  lcd.createChar(1, name0x7);
  lcd.createChar(2, name0x13);

  lcd.setCursor(1, 0); 
  lcd.write((uint8_t)0);

  lcd.setCursor(7, 0);
  lcd.write((uint8_t)1);

  lcd.setCursor(13, 0);
  lcd.write((uint8_t)2);

  showHomeValues();
}


/************************************************************
 *                FUNÇÕES DE ANIMAÇÃO/TELAS                 *
 ************************************************************/
void welcome() {
  String line = "SEJA BEM VINDO";
  for (int i = 0; i < (int)line.length(); i++) {
    lcd.setCursor(i + 1, 0);
    lcd.print(line[i]);
    delay(150);

    // Efeito de 'cair'
    lcd.setCursor(i + 1, 0);
    lcd.print(" ");
    lcd.setCursor(i + 1, 1);
    lcd.print(line[i]);

    if (!isWhitespace(line[i])) {
      tone(BUZZER_PIN, 250);
      delay(150);
      noTone(BUZZER_PIN);
    }
  }
}

void wizard1() {
  byte name1x4[] = {
    B00000, B00000, B00000, B00000,
    B00000, B00000, B00000, B00000
  };
  byte name0x0[] = {
    B00000, B00000, B00000, B00000,
    B00000, B00000, B00000, B00001
  };
  byte name0x1[] = {
    B00000, B01000, B10100, B00100,
    B01110, B01110, B11111, B11111
  };
  byte name0x2[] = {
    B00000, B00000, B00000, B00000,
    B00000, B00000, B00000, B10000
  };
  byte name1x0[] = {
    B00000, B00000, B00001, B00010,
    B00010, B00010, B00010, B00010
  };
  byte name1x1[] = {
    B10001, B11011, B10001, B01010,
    B00100, B00000, B00100, B00100
  };
  byte name1x2[] = {
    B00010, B00101, B10010, B01010,
    B01010, B01110, B01010, B01000
  };

  lcd.createChar(0, name1x4);
  lcd.createChar(1, name0x0);
  lcd.createChar(2, name0x1);
  lcd.createChar(3, name0x2);
  lcd.createChar(4, name1x0);
  lcd.createChar(5, name1x1);
  lcd.createChar(6, name1x2);

  lcd.setCursor(4, 1); 
  lcd.write(0);
  lcd.setCursor(0, 0); 
  lcd.write(1);
  lcd.setCursor(1, 0); 
  lcd.write(2);
  lcd.setCursor(2, 0); 
  lcd.write(3);
  lcd.setCursor(0, 1); 
  lcd.write(4);
  lcd.setCursor(1, 1); 
  lcd.write(5);
  lcd.setCursor(2, 1); 
  lcd.write(6);

  delay(400);
  lcd.clear();
}

void wizard2() {
  byte name1x2[] = {
    B00000, B00000, B10000, B01000,
    B01001, B01110, B01010, B01000
  };
  byte name0x0[] = {
    B00000, B00000, B00000, B00000,
    B00000, B00000, B00000, B00001
  };
  byte name0x1[] = {
    B00000, B01000, B10100, B00100,
    B01110, B01110, B11111, B11111
  };
  byte name0x2[] = {
    B00000, B00000, B00000, B00000,
    B00000, B00000, B00000, B10000
  };
  byte name1x0[] = {
    B00000, B00000, B00001, B00010,
    B00010, B00010, B00010, B00010
  };
  byte name1x1[] = {
    B10001, B11011, B10001, B01010,
    B00100, B00000, B00100, B00100
  };
  byte name1x3[] = {
    B00100, B01010, B01100, B10000,
    B00000, B00000, B00000, B00000
  };

  lcd.createChar(0, name1x2);
  lcd.createChar(1, name0x0);
  lcd.createChar(2, name0x1);
  lcd.createChar(3, name0x2);
  lcd.createChar(4, name1x0);
  lcd.createChar(5, name1x1);
  lcd.createChar(6, name1x3);

  lcd.setCursor(2, 1); 
  lcd.write(0);
  lcd.setCursor(0, 0); 
  lcd.write(1);
  lcd.setCursor(1, 0); 
  lcd.write(2);
  lcd.setCursor(2, 0); 
  lcd.write(3);
  lcd.setCursor(0, 1); 
  lcd.write(4);
  lcd.setCursor(1, 1); 
  lcd.write(5);
  lcd.setCursor(3, 1); 
  lcd.write(6);
}

void magic() {
  String word = "MAGITECH!";
  byte ball[] = {
    B00100, B01110, B00100, B00000,
    B00000, B00000, B00000, B00000
  };

  lcd.createChar(7, ball);

  int startPos   = 4;
  int endPos     = 15;
  int frameDelay = 200;

  lcd.setCursor(startPos, 1);
  lcd.write(byte(7));

  for (int pos = startPos + 1; pos <= endPos; pos++) {
    delay(frameDelay);

    // "apaga" a posição anterior
    lcd.setCursor(pos - 1, 1);
    lcd.print(" ");

    // desenha na nova posição
    lcd.setCursor(pos, 1);
    lcd.write(byte(7));

    if (pos >= 6) {
      int letterIndex = pos - 6;
      if (letterIndex < (int)word.length()) {
        lcd.setCursor(pos - 1, 1);
        lcd.print(word[letterIndex]);
      }
    }
  }

  delay(500);
  lcd.setCursor(endPos, 1);
  lcd.print(" ");
  delay(500);
}

void displayRTC() {
  DateTime adjustedTime = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DATA: ");
  lcd.print(adjustedTime.day() < 10 ? "0" : "");
  lcd.print(adjustedTime.day());
  lcd.print("/");
  lcd.print(adjustedTime.month() < 10 ? "0" : ""); 
  lcd.print(adjustedTime.month());
  lcd.print("/");
  lcd.print(adjustedTime.year());

  lcd.setCursor(0, 1);
  lcd.print("HORA: ");
  lcd.print(adjustedTime.hour() < 10 ? "0" : ""); 
  lcd.print(adjustedTime.hour());
  lcd.print(":");
  lcd.print(adjustedTime.minute() < 10 ? "0" : ""); 
  lcd.print(adjustedTime.minute());
  lcd.print(":");
  lcd.print(adjustedTime.second() < 10 ? "0" : ""); 
  lcd.print(adjustedTime.second());
}


/************************************************************
 *                FUNÇÕES DE LEITURA/SENSORES               *
 ************************************************************/
void tenthRead() {
  float sumTemp = 0;
  float sumHumd = 0;

  for (int i = 0; i < 10; i++) {
    sumTemp += tempReadings[i];
    sumHumd += humdReadings[i];
  }

  lastAvgTemp = sumTemp / 10;
  lastAvgHumd = sumHumd / 10;

  if (homePageActive) {
    checkTempAlert();
    checkHumdAlert();
  }
}

// ALERTAS
void checkTempAlert() {
  float minTempThreshold = 15.0;
  float maxTempThreshold = 25.0;

  switch (temperatureScale) {
    case 2: // Fahrenheit
      minTempThreshold = (15.0 * 1.8) + 32;
      maxTempThreshold = (25.0 * 1.8) + 32;
      break;
    case 3: // Kelvin
      minTempThreshold = 15.0 + 273.15;
      maxTempThreshold = 25.0 + 273.15;
      break;
  }

  if ((lastAvgTemp < minTempThreshold) || (lastAvgTemp > maxTempThreshold)) {
    digitalWrite(LED_GRE, HIGH);
    if (!buzzerOn) {
      tone(BUZZER_PIN, 1000);
      buzzerOn         = true;
      buzzerTempReason = true;
    }
  }
  else {
    digitalWrite(LED_GRE, LOW);
    if (buzzerTempReason) {
      buzzerTempReason = false;
      if (!buzzerHumdReason && !buzzerLightReason) {
        noTone(BUZZER_PIN);
        buzzerOn = false;
      }
    }
  }
}

void checkHumdAlert() {
  if ((lastAvgHumd < 40.0) || (lastAvgHumd > 65.0)) {
    digitalWrite(LED_RED, HIGH);
    if (!buzzerOn) {
      tone(BUZZER_PIN, 1000);
      buzzerOn = true;
      buzzerHumdReason = true;
    }
  }
  else {
    digitalWrite(LED_RED, LOW);
    if (buzzerHumdReason) {
      buzzerHumdReason = false;
      if (!buzzerTempReason && !buzzerLightReason) {
        noTone(BUZZER_PIN);
        buzzerOn = false;
      }
    }
  }
}

void checkLightAlert() {
  if ((lightLevel < 0) || (lightLevel > 30)) {
    digitalWrite(LED_YEL, HIGH);
    if (!buzzerOn) {
      tone(BUZZER_PIN, 1000);
      buzzerOn          = true;
      buzzerLightReason = true;
    }
  }
  else {
    digitalWrite(LED_YEL, LOW);
    if (buzzerLightReason) {
      buzzerLightReason = false;
      if (!buzzerTempReason && !buzzerHumdReason) {
        noTone(BUZZER_PIN);
        buzzerOn = false;
      }
    }
  }
}

// Função para avançar o ponteiro na EEPROM
void getNextAddress() {
  currentAddress += recordSize;
  if (currentAddress >= endAddress) {
    currentAddress = 0;
  }
}

// Lê o log da EEPROM
void get_log() {
  Serial.println("Data stored in EEPROM:");
  Serial.println("Timestamp\t\tTemperature\tHumidity\tLuminosidade");

  for (int address = startAddress; address < endAddress; address += recordSize) {
    long timeStamp;
    int tempInt, humiInt, lumiInt;

    EEPROM.get(address,     timeStamp);
    EEPROM.get(address + 4, tempInt);
    EEPROM.get(address + 6, humiInt);
    // ===============================
    // NOVO: Leitura da luminosidade
    // ===============================
    EEPROM.get(address + 8, lumiInt);

    float temperature = tempInt / 100.0;
    float humidity    = humiInt / 100.0;
    int   luminosity  = lumiInt;

    // 0xFFFFFFFF = valor 'invalido' ou "sem dados"
    if (timeStamp != 0xFFFFFFFF) {
      DateTime dt(timeStamp);

      Serial.print(dt.year());
      Serial.print("-");
      Serial.print(dt.month() < 10 ? "0" : ""); 
      Serial.print(dt.month());
      Serial.print("-");
      Serial.print(dt.day() < 10 ? "0" : "");
      Serial.print(dt.day());
      Serial.print(" ");
      Serial.print(dt.hour() < 10 ? "0" : "");
      Serial.print(dt.hour());
      Serial.print(":");
      Serial.print(dt.minute() < 10 ? "0" : "");
      Serial.print(dt.minute());
      Serial.print(":");
      Serial.print(dt.second() < 10 ? "0" : "");
      Serial.print(dt.second());

      Serial.print("\t");
      Serial.print(temperature); Serial.print("C\t\t");
      Serial.print(humidity);    Serial.print("%\t\t");
      Serial.print(luminosity);  Serial.println("%");
    }
  }
}

// Registra anomalias na EEPROM
void recordEEPROM(){
  now = rtc.now();

  // Offset de fuso
  int offsetSeconds = UTC_OFFSET * 3600;
  now = now.unixtime() + offsetSeconds;
  adjustedTime = DateTime(now);

  // Só registra uma vez por minuto
  if (adjustedTime.minute() != lastLoggedMinute) {
    lastLoggedMinute = adjustedTime.minute();

    // Se algum valor está fora do limite
    if (lastAvgTemp < trigger_t_min || lastAvgTemp > trigger_t_max || 
        lastAvgHumd < trigger_u_min || lastAvgHumd > trigger_u_max || 
        lightLevel < trigger_l_min || lightLevel > trigger_l_max) {
          
      int tempInt = (int)(lastAvgTemp * 100);
      int humiInt = (int)(lastAvgHumd * 100);
      // =========================
      // NOVO: guarda luminosidade
      // =========================
      int lumiInt = lightLevel; 

      EEPROM.put(currentAddress,     now.unixtime()); 
      EEPROM.put(currentAddress + 4, tempInt);        
      EEPROM.put(currentAddress + 6, humiInt);        
      EEPROM.put(currentAddress + 8, lumiInt);        

      // Log no Serial
      Serial.println("Registro de Anomalia Gravado:");
      Serial.print("Data/Hora: ");
      Serial.print(adjustedTime.year());  Serial.print("-");
      Serial.print(adjustedTime.month() < 10 ? "0" : ""); Serial.print(adjustedTime.month()); Serial.print("-");
      Serial.print(adjustedTime.day() < 10 ? "0" : "");   Serial.print(adjustedTime.day());   Serial.print(" ");
      Serial.print(adjustedTime.hour() < 10 ? "0" : "");  Serial.print(adjustedTime.hour());  Serial.print(":");
      Serial.print(adjustedTime.minute() < 10 ? "0" : "");Serial.print(adjustedTime.minute());Serial.print(":");
      Serial.print(adjustedTime.second() < 10 ? "0" : "");Serial.println(adjustedTime.second());

      Serial.print("Temperatura: ");  Serial.print(lastAvgTemp);  Serial.println("°C");
      Serial.print("Umidade: ");      Serial.print(lastAvgHumd);  Serial.println("%");
      Serial.print("Luminosidade: "); Serial.print(lightLevel);    Serial.println("%");
      Serial.println("---------------------------------");

      getNextAddress();
    }
  }
}

// Log no monitor serial
void serialLog(float temp, float humid, int valorLDR, long leituraNum) {
  now = rtc.now();

  int offsetSeconds = UTC_OFFSET * 3600;
  now = now.unixtime() + offsetSeconds;
  adjustedTime = DateTime(now);
  
  String scaleChar = "°C";
  if (temperatureScale == 2)      scaleChar = "°F";
  else if (temperatureScale == 3) scaleChar = "°K";

  Serial.println("Leitura: " + String(leituraNum + 1));
  Serial.println("Temp: " + String(temp) + scaleChar);
  Serial.println("Ultima Temp Media: " + String(lastAvgTemp) + scaleChar);
  Serial.println("Umidade: " + String(humid) + " %");
  Serial.println("Ultima Umidade Media: " + String(lastAvgHumd) + " %");
  Serial.println("Luminosidade: " + String(lightLevel) + " %");
  Serial.println("ValorLDR: " + String(valorLDR));
  
  Serial.print(adjustedTime.day());
  Serial.print("/");
  Serial.print(adjustedTime.month());
  Serial.print("/");
  Serial.print(adjustedTime.year());
  Serial.print(" ");
  Serial.print(adjustedTime.hour() < 10 ? "0" : "");
  Serial.print(adjustedTime.hour());
  Serial.print(":");
  Serial.print(adjustedTime.minute() < 10 ? "0" : "");
  Serial.print(adjustedTime.minute());
  Serial.print(":");
  Serial.print(adjustedTime.second() < 10 ? "0" : "");
  Serial.print(adjustedTime.second());
  Serial.println();
  Serial.println("---");
}
