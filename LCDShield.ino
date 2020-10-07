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
#define ANALOG_KEYS_PIN A0 // Шина для аналоговых кнопок

int azAngle = 0;           // Угол азимута
int azOldSensorValue = 0;  // Предыдущее значение с датчика азимута
int azTarget = 300;        // Цель для поворота
boolean azMove = false;    // Флаг включения/отключения вращения по азимуту

//Переменная для отображения азимута  в режиме MANUAL
String strAzAngle;         // Текущее положение антенны
String strAzTarget;        // Цель для перемещения

int adcKeyOld;
int adcKeyIn;
int NUM_KEYS = 5;
int key = -1;
int adcKeyVal[5] = {30, 150, 360, 535, 760 }; //Define the value at A0 pin

int azCorrect = 0;
int elCorrect = 0;

//averaging loop
const int numReadings = 25;
int azimuth[numReadings];      // the readings from the analog input
int totalAz = 0;
int averageAz = 0;                // the average
int readIndex = 0;              // the index of the current reading
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

int azSensor() {
  // AVERAGING LOOP - subtract the last reading:
  totalAz = totalAz - azimuth[readIndex];
  // Читакм сенсор
  azimuth[readIndex] = analogRead(AZSENSOR);
  // прибавьте показание к итогу
  totalAz = totalAz + azimuth[readIndex];
  // перейти к следующей позиции в массиве
  readIndex = readIndex + 1;
  // если мы в конце массива ...
  if (readIndex >= numReadings) {
    // ...еще раз
    readIndex = 0;
  }
  // calculate the average:
  averageAz = totalAz / numReadings;
  //  Serial.println(averageAz);
  azAngle = (averageAz - 86) * 1.127;
  azAngle = int(azAngle / 2.842);    // azimuth value 0-359
  if (azAngle < 0) {
    azAngle = 0;
  }
  if (azAngle > 359) {
    azAngle = 359; // keep values between limits
  }

  return azAngle;
}

int elSensor(){

}

void getSensors()
{
  azSensor();
}

void getKeys() {
  adcKeyIn = analogRead(ANALOG_KEYS_PIN);

  adcKeyIn = get_key(adcKeyIn);
  if (adcKeyIn == 0) {
    delay(500);
    if (azTarget + STEP <= 359)
      azTarget += STEP;
  }

  if (adcKeyIn == 3) {
    delay(300);
    if (azTarget - STEP >= 0)
      azTarget -= STEP;
  }

  if (adcKeyIn == 4) {
    delay(300);
    azMove = true;
    strAzTarget = AzElString(azTarget, false);
  }
};
String AzElString(int someIntVolue, bool el)
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

void cw(int currentAz)
{
  if (currentAz < 359) {
    digitalWrite(PIN_CW, LOW);
    lcd.setCursor(15, 0);
    lcd.print(">");
    digitalWrite(PIN_CCW, HIGH);
  } else {
    digitalWrite(PIN_CW, LOW);
  }

}

void ccw(int currentAz)
{
  if (currentAz > 0) {
    digitalWrite(PIN_CCW, LOW);
    lcd.setCursor(15, 0);
    lcd.print("<");
    digitalWrite(PIN_CW, HIGH);
  } else {
    digitalWrite(PIN_CCW, LOW);
  }
}

int get_key(unsigned int input)
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
    k = -1;     // No valid key pressed
  return k;
}

void setup()
{
  Serial.begin(9600);
  pinMode(PIN_CCW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(AZSENSOR, INPUT);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
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
  getSensors();
}

void loop()
{

  getSensors();
  getKeys();

  if (azMove)
  {
    if (azTarget == azAngle)
    {
      lcd.setCursor(15, 0);
      lcd.print(" ");
      digitalWrite(PIN_CW, LOW);
      digitalWrite(PIN_CCW, LOW);
      azMove = false;
    }

    if (azTarget - azAngle >= 1)
    {
      cw(azAngle);
    }

    if (azAngle - azTarget >= 1)
    {
      ccw(azAngle);
    }
  }

  //getSensors();
  // Отображение азимута текущей цели антенны
  lcd.setCursor(3, 0);
  lcd.print(strAzTarget);
  // Отображение азимута текущего положения антенны
  lcd.setCursor(3, 1);
  lcd.print(strAzAngle);
  // Отображение данных с датчика
  strAzAngle = AzElString(azAngle, false);
  // Отображение цели
  strAzTarget = AzElString(azTarget, false);
}
