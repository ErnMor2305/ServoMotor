#define RPWM 5
#define LPWM 6
#define R_EN 7
#define L_EN 8

const int velocidadMaxima = 255;
const int pasoVelocidad = 5;
const int retardoRampa = 20;

String comando = "";
String estadoActual = "alto";  // Estado inicial
int velocidadActual = 0;

void setup() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  Serial.begin(115200);
  Serial.println("Sistema listo. Escriba: 'derecha', 'izquierda' o 'alto'");
}

void loop() {
  if (Serial.available() > 0) {
    comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "derecha" && estadoActual != "derecha") {
      desacelerar();
      estadoActual = "derecha";
      acelerar("derecha");
      Serial.println("Girando a la derecha");

    } else if (comando == "izquierda" && estadoActual != "izquierda") {
      desacelerar();
      estadoActual = "izquierda";
      acelerar("izquierda");
      Serial.println("Girando a la izquierda");

    } else if (comando == "alto" && estadoActual != "alto") {
      desacelerar();
      estadoActual = "alto";
      Serial.println("Motor detenido");

    } else {
      Serial.println("Comando no reconocido o sin cambio");
    }
  }
}

void desacelerar() {
  for (int v = velocidadActual; v >= 0; v -= pasoVelocidad) {
    aplicarPWM(v, estadoActual);
    delay(retardoRampa);
  }
  aplicarPWM(0, estadoActual);
  velocidadActual = 0;
}

void acelerar(String sentido) {
  for (int v = 0; v <= velocidadMaxima; v += pasoVelocidad) {
    aplicarPWM(v, sentido);
    delay(retardoRampa);
  }
  velocidadActual = velocidadMaxima;
}

void aplicarPWM(int valorPWM, String sentido) {
  if (sentido == "derecha") {
    analogWrite(RPWM, valorPWM);
    analogWrite(LPWM, 0);
  } else if (sentido == "izquierda") {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, valorPWM);
  } else {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, 0);
  }
}
