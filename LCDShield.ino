#include <LiquidCrystal.h>

const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Реле
#define PIN_CCW 3          // Поворот против часовой стрелки
#define PIN_CW 2           // Поворот по часовой стрелки
#define PIN_UP 11          // Актуатор вверх
#define PIN_DOWN 12        // Актуатор вниз
#define STEP 1             // Шаг
#define AZSENSOR A2        // Номер пина для аналогового датчика азимута
#define ELSENSOR A1        // Номер пина для аналогового датчика элевации
#define ANALOG_KEYS_PIN A0 // Шина для аналоговых кнопок A0
#define ANALOG_KEYS_PIN_A5 A5 // Шина для аналоговых кнопок A5
//петля усреднения
const int numReadings = 10;
int readIndexAz = 0;
int readIndexEl = 0;
int azAngle = 0;          // Угол азимута
int azOldSensorValue = 0; // Предыдущее значение с датчика азимута
int azTarget = 300;       // Цель для поворота
boolean azMove = false;   // Флаг включения/отключения вращения по азимуту
String strAzAngle;        // Текущее положение антенны
String strAzTarget;       // Цель для перемещения
int azCorrect = 0;        // Коррекция азимута нуля градусов
int azimuth[numReadings]; // the readings from the analog input
int totalAz = 0;
int averageAz = 0; // усреднение азимута

int elAngle = 0;
int elOldSensorValue = 0;
int elTarget = 0;
boolean elMove = false;
String strElAngle;
String strElTarget;
int elCorrect = 0;
int elevation[numReadings];
int totalEl = 0;
int averageEl = 0; // усреднение элевации
boolean operate = false;

// Кнопки
int adcKeyOld;
int adcKeyIn;
int NUM_KEYS = 5;
int key = -1;
int adcKeyVal[5] = {30, 180, 360, 535, 760}; //Define the value at A0 pin

byte heart[8] = { 0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000 };

byte upArrow[8] = { 0b00000, 0b00000, 0b00100, 0b01010, 0b10001, 0b00000, 0b00000, 0b00000 };
byte dwArrow[8] = { 0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b00000, 0b00000, 0b00000 };

byte queue[8] = { 0b00001,
                  0b00011,
                  0b00001,
                  0b00100,
                  0b00110,
                  0b10100,
                  0b11000,
                  0b10000
                };

void clearLine(int line)
{
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void printDisplay(String message)
{
  Serial.println(message);
  lcd.setCursor(9, 1);
  lcd.print(message);
  delay(1000);
  clearLine(1);
}

void initSensorAvarage()
{
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    azimuth[thisReading] = 0;
    elevation[numReadings] = 0;
  }
}

int azSensor()
{

  // AVERAGING LOOP - subtract the last reading:
  totalAz = totalAz - azimuth[readIndexAz];
  // Читакм сенсор
  azimuth[readIndexAz] = analogRead(AZSENSOR);
  // прибавьте показание к итогу
  totalAz = totalAz + azimuth[readIndexAz];
  // перейти к следующей позиции в массиве
  readIndexAz = readIndexAz + 1;
  // если мы в конце массива ...
  if (readIndexAz >= numReadings)
  {
    // ...еще раз
    readIndexAz = 0;
  }
  // calculate the average:
  averageAz = totalAz / numReadings;
  // Serial.println(averageAz);
  azAngle = (averageAz - 86) * 1.127;
  azAngle = int(azAngle / 2.842); // azimuth value 0-359
  if (azAngle < 0)
  {
    azAngle = 0;
  }
  if (azAngle > 359)
  {
    azAngle = 359; // keep values between limits
  }

  return azAngle;
}

int elSensor()
{
  //ELEVATION AVERAGING LOOP
  totalEl = totalEl - elevation[readIndexEl];
  elevation[readIndexEl] = analogRead(ELSENSOR);

  totalEl = totalEl + elevation[readIndexEl];
  readIndexEl = readIndexEl + 1;
  if (readIndexEl >= numReadings)
  {
    readIndexEl = 0;
  }
  averageEl = totalEl / numReadings;
  elAngle = (averageEl - 86) * 1.127;
  elAngle = int(elAngle / 2.842); // elevation value 0-359
  
  //elAngle = ((averageEl-2)*1.025);
  //elAngle = int(elAngle/11.3);       // elev value 0-90
  if (elAngle < 0)
  {
    elAngle = 0;
  }
  if (elAngle > 90)
  {
    elAngle = 90;
  }

  return elAngle;
}

void getSensors()
{
  azSensor();
  elSensor();
}

void getKeysA0()
{
  adcKeyIn = analogRead(ANALOG_KEYS_PIN);
    Serial.print("KEYA0: ");
  Serial.println(adcKeyIn);
  adcKeyIn = getKey(adcKeyIn);

  if (adcKeyIn == 0)
  {
    delay(300);
    if (azTarget + STEP <= 359)
      azTarget += STEP;
  }

  if (adcKeyIn == 3)
  {
    delay(300);
    if (azTarget - STEP >= 0)
      azTarget -= STEP;
  }

  if (adcKeyIn == 4)
  {
    // delay(300);
    // azMove = true;
    // strAzTarget = AzElString(azTarget);
  }

  if (adcKeyIn == 1)
  {
    delay(400);
    if (elTarget + STEP <= 90)
      elTarget += STEP;
  }

  if (adcKeyIn == 2)
  {
    delay(400);
    if (elTarget - STEP >= 0)
      elTarget -= STEP;
  }
};

void getKeysA5()
{
  adcKeyIn = analogRead(ANALOG_KEYS_PIN_A5);
  Serial.print("KEYA5: ");
  Serial.println(adcKeyIn);
  adcKeyIn = getKey(adcKeyIn);

  if (operate) {
    if (adcKeyIn == 3)
    {
      delay(300);
      azMove = true;
    }
    if (adcKeyIn == 2)
    {
      delay(300);
      elMove = true;
    }
  }


  if (adcKeyIn == 0)
  {
    delay(300);

    if (operate) {
      operate = false;
      if (azMove) {
        azMove = false;
      }
      if (elMove) {
        elMove = false;
      }
      digitalWrite(PIN_CW, LOW);
      digitalWrite(PIN_CCW, LOW);
      digitalWrite(PIN_UP, LOW);
      digitalWrite(PIN_DOWN, LOW);
      lcd.setCursor(13, 1);
      lcd.print(" ");
      lcd.setCursor(14, 0);
      lcd.print(" ");
      lcd.setCursor(15, 0);
      lcd.print(" ");
    } else {
      operate = true;
      lcd.setCursor(13, 1);
      lcd.print(char(1));
    }
  }
};

String AzElString(int someIntVolue)
{
  if (someIntVolue < 0)
  {
    return "" + String(someIntVolue);
  }
  if (someIntVolue < 10)
  {
    return "  " + String(someIntVolue);
  }

  if (someIntVolue < 100)
  {
    return " " + String(someIntVolue);
  }

  if (someIntVolue >= 100)
  {
    return String(someIntVolue);
  }
}

void cw(boolean azMoveFlag)
{
  if (azMoveFlag)
  {
    digitalWrite(PIN_CW, LOW);
    lcd.setCursor(14, 0);
    lcd.print(">");
    digitalWrite(PIN_CCW, HIGH);
  }
  else
  {
    digitalWrite(PIN_CW, LOW);
  }
}

void ccw(boolean azMoveFlag)
{
  if (azMoveFlag)
  {
    digitalWrite(PIN_CCW, LOW);
    lcd.setCursor(14, 0);
    lcd.print("<");
    digitalWrite(PIN_CW, HIGH);
  }
  else
  {
    digitalWrite(PIN_CCW, LOW);
  }
}

void up(boolean elMoveFlag) {
  if (elMoveFlag)
  {
    digitalWrite(PIN_UP, LOW);
    lcd.setCursor(15, 0);
    lcd.print(char(2));
    digitalWrite(PIN_DOWN, HIGH);
  }
  else
  {
    digitalWrite(PIN_UP, LOW);
  }
}
void down(boolean elMoveFlag) {
  if (elMoveFlag)
  {
    digitalWrite(PIN_DOWN, LOW);
    lcd.setCursor(15, 0);
    lcd.print(char(3));
    digitalWrite(PIN_UP, HIGH);
  }
  else
  {
    digitalWrite(PIN_DOWN, LOW);
  }
}

int getKey(unsigned int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
  {
    if (input < adcKeyVal[k])
    {
      return k;
    }
  }
  if (k >= NUM_KEYS)
    k = -1; // No valid key pressed
  return k;
}

void queueIndicate() {
  if (!operate) {
    if (azMove || elMove) {
      lcd.setCursor(13, 0);
      lcd.print(char(4));
    } else {
      lcd.setCursor(13, 0);
      lcd.print(" ");
    }
  }

}

void setup()
{
  Serial.begin(9600);

  pinMode(PIN_CCW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(PIN_UP, OUTPUT);
  pinMode(PIN_DOWN, OUTPUT);
  pinMode(AZSENSOR, INPUT);
  pinMode(ELSENSOR, INPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.createChar(1, heart);
  lcd.createChar(2, upArrow);
  lcd.createChar(3, dwArrow);
  lcd.createChar(4, queue);
  lcd.print(" R8CDF ROTATOR");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print("AZT");
  lcd.setCursor(0, 1);
  lcd.print("AZA");

  lcd.setCursor(7, 0);
  lcd.print("ELT");
  lcd.setCursor(7, 1);
  lcd.print("ELA");
  initSensorAvarage();
  getSensors();
}

void loop()
{
  getSensors();
  getKeysA5();
  if (operate) {
    getKeysA0();
    if (azMove)
    {
      if (azTarget == azAngle)
      {
        lcd.setCursor(14, 0);
        lcd.print(" ");
        digitalWrite(PIN_CW, LOW);
        digitalWrite(PIN_CCW, LOW);
        azMove = false;
      }

      if (azTarget - azAngle >= 1)
      {
        cw(azMove);
      }

      if (azAngle - azTarget >= 1)
      {
        ccw(azMove);
      }
    }

    if (elMove)
    {
      if (elTarget == elAngle)
      {
        lcd.setCursor(15, 0);
        lcd.print(" ");
        digitalWrite(PIN_UP, LOW);
        digitalWrite(PIN_DOWN, LOW);
        elMove = false;
      }
      if (elTarget - elAngle >= 1)
      {
        up(elMove);
      }

      if (elAngle - elTarget >= 1)
      {
        down(elMove);
      }
    }
  }


  // Operate
  lcd.setCursor(14, 1);
  lcd.print(azMove);
  lcd.setCursor(15, 1);
  lcd.print(elMove);
  queueIndicate();
  // Отображение азимута текущей цели антенны
  lcd.setCursor(3, 0);
  lcd.print(strAzTarget);
  // Отображение азимута текущего положения антенны
  lcd.setCursor(3, 1);
  lcd.print(strAzAngle);
  // Отображение данных с датчика азимута
  strAzAngle = AzElString(azAngle);
  // Отображение цели азимута
  strAzTarget = AzElString(azTarget);

  // Отображение елевации цели антенны
  lcd.setCursor(10, 0);
  lcd.print(strElTarget);
  // Отображение элевации
  lcd.setCursor(10, 1);
  lcd.print(strElAngle);
  // Отображение данных с датчика элевации
  strElAngle = AzElString(elAngle);
  // Отображение цели элевации
  strElTarget = AzElString(elTarget);
}
