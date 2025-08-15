#define RPWM 5
#define LPWM 6
#define R_EN 7
#define L_EN 8

const int velocidadMaximaPermitida = 255;  // Límite absoluto por seguridad
const int pasoVelocidad = 5;
const int retardoRampa = 20;

String comando = "";
String estadoActual = "alto";  // "derecha", "izquierda" o "alto"
int velocidadActual = 0;
int velocidadObjetivo = 255;   // Velocidad por defecto

void setup() {
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  Serial.begin(115200);
  Serial.println("Sistema listo. Comandos: 'derecha', 'izquierda', 'alto', 'velocidad <0-255>'");
}

void loop() {
  if (Serial.available() > 0) {
    comando = Serial.readStringUntil('\n');
    comando.trim();

    // Control de dirección
    if (comando == "derecha" || comando == "izquierda") {
      if (comando != estadoActual) {
        desacelerar();
        estadoActual = comando;
        acelerar(estadoActual);
        Serial.print("Girando a la ");
        Serial.println(estadoActual);
      } else {
        Serial.println("Ya está en ese estado.");
      }
    }

    // Detener el motor
    else if (comando == "alto") {
      if (estadoActual != "alto") {
        desacelerar();
        estadoActual = "alto";
        Serial.println("Motor detenido");
      } else {
        Serial.println("Ya está detenido.");
      }
    }

    // Cambiar velocidad con comando: velocidad <valor>
    else if (comando.startsWith("velocidad")) {
      int nuevaVel = comando.substring(9).toInt();
      nuevaVel = constrain(nuevaVel, 0, velocidadMaximaPermitida);

      if (nuevaVel != velocidadObjetivo) {
        ajustarVelocidad(nuevaVel);
        Serial.print("Velocidad cambiada a ");
        Serial.println(nuevaVel);
      } else {
        Serial.println("La velocidad ya está en ese valor.");
      }
    }

    else {
      Serial.println("Comando no reconocido.");
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
  for (int v = 0; v <= velocidadObjetivo; v += pasoVelocidad) {
    aplicarPWM(v, sentido);
    delay(retardoRampa);
  }
  velocidadActual = velocidadObjetivo;
}

void ajustarVelocidad(int nuevaVelocidad) {
  if (nuevaVelocidad == velocidadActual) return;

  int paso = (nuevaVelocidad > velocidadActual) ? pasoVelocidad : -pasoVelocidad;

  for (int v = velocidadActual; v != nuevaVelocidad; v += paso) {
    aplicarPWM(v, estadoActual);
    delay(retardoRampa);
  }

  aplicarPWM(nuevaVelocidad, estadoActual);
  velocidadActual = nuevaVelocidad;
  velocidadObjetivo = nuevaVelocidad;
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
