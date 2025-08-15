#include <Wire.h>

#define AS5600_ADDR 0x36
#define RAW_ANGLE_REG 0x0C

// ==== CONFIGURACIÓN ====
// Ajusta tu relación de engranajes aquí:
double GEAR_RATIO = 24.0;   // Ej.: 101.0 o 217.0
const float HYST = 20.0;      // Histéresis en grados para detección de wrap

// ==== ESTADO ====
float lastAngleMotor = -1.0;     // Último ángulo (0..360)
long vueltasMotor = 0;           // Contador de vueltas del motor (+/-)
double ceroSalidaDeg = 0.0;      // Offset de "cero" en el eje de salida (deg)

uint16_t readRawAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(RAW_ANGLE_REG);
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 2);

  if (Wire.available() >= 2) {
    uint16_t hi = Wire.read();
    uint16_t lo = Wire.read();
    return (hi << 8) | lo;  // 0..4095
  }
  return 0;
}

float readAngleMotorDeg() {
  uint16_t raw = readRawAngle();         // 0..4095
  return (raw * 360.0f) / 4096.0f;       // 0..360 (no inclusivo)
}

void maybeUpdateWrap(float ang) {
  if (lastAngleMotor < 0.0f) { // primera lectura
    lastAngleMotor = ang;
    return;
  }
  float diff = ang - lastAngleMotor;

  // Normaliza diff a (-180, 180]
  if (diff > 180.0f)  diff -= 360.0f;
  if (diff <= -180.0f) diff += 360.0f;

  // Detección de cruce por 0 con histéresis
  // Si veníamos subiendo y pasamos 360->0, diff negativo grande
  if (diff < -HYST) {
    vueltasMotor += 1;  // Completó una vuelta en sentido + (CW según montaje)
  }
  // Si veníamos bajando y pasamos 0->360, diff positivo grande
  else if (diff > HYST) {
    vueltasMotor -= 1;  // Vuelta en sentido - (CCW)
  }

  lastAngleMotor = ang;
}

void printStatus(float angMotor, double angOutAbs, double angOutTotal) {
  Serial.print("Motor: ");
  Serial.print(angMotor, 2);
  Serial.print(" deg | VueltasMotor: ");
  Serial.print(vueltasMotor);
  Serial.print(" | Out(abs): ");
  Serial.print(angOutAbs, 2);
  Serial.print(" deg | Out(total): ");
  Serial.print(angOutTotal, 2);
  Serial.print(" deg | R=");
  Serial.println(GEAR_RATIO, 3);
}

void handleSerial() {
  if (!Serial.available()) return;
  String s = Serial.readStringUntil('\n');
  s.trim();
  s.toLowerCase();

  if (s == "zero") {
    // Fija la posición actual de salida a 0
    float angMotor = readAngleMotorDeg();
    maybeUpdateWrap(angMotor);
    double thetaMotorTotal = 360.0 * (double)vueltasMotor + angMotor; // deg
    double thetaOutTotal   = thetaMotorTotal / GEAR_RATIO;            // deg
    // Queremos que out_abs = 0 en esta posición => cero = angOutTotal mod 360
    ceroSalidaDeg = fmod(thetaOutTotal, 360.0);
    if (ceroSalidaDeg < 0) ceroSalidaDeg += 360.0;
    Serial.println("Cero de salida fijado a 0 deg en la posición actual.");
  } else if (s == "r?") {
    Serial.print("Relacion actual R = ");
    Serial.println(GEAR_RATIO, 6);
  } else if (s.startsWith("r=")) {
    String val = s.substring(2);
    double nuevoR = val.toDouble();
    if (nuevoR > 0.1 && nuevoR < 10000.0) {
      GEAR_RATIO = nuevoR;
      Serial.print("Nueva relacion R = ");
      Serial.println(GEAR_RATIO, 6);
    } else {
      Serial.println("Valor de R invalido.");
    }
  } else {
    Serial.println("Comandos: zero | r? | r=XXX");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("AS5600 + Reductor: prueba de ángulo con multi-vuelta y cero.");
  Serial.println("Comandos: zero | r? | r=XXX   (ej: r=101)");
}

void loop() {
  handleSerial();

  float angMotor = readAngleMotorDeg();     // 0..360
  maybeUpdateWrap(angMotor);

  // Posición total del motor y mapeo a salida
  double thetaMotorTotal = 360.0 * (double)vueltasMotor + angMotor; // deg
  double thetaOutTotal   = thetaMotorTotal / GEAR_RATIO;            // deg (puede ser >360)
  double angOutAbs       = fmod(thetaOutTotal - ceroSalidaDeg, 360.0);
  if (angOutAbs < 0) angOutAbs += 360.0;

  printStatus(angMotor, angOutAbs, thetaOutTotal);
  delay(100);
}
