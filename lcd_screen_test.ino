/*
  AHORCADO - Tecnologías Interactivas
  Ver DESIGN.md para las reglas del juego.

  Conexiones (Arduino Uno/Nano):
    LCD I2C    VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5
    Boton 1    D2  -> GND   (letra anterior)
    Boton 2    D3  -> GND   (letra siguiente)
    Boton 3    D4  -> GND   (confirmar letra)
    LED rojo   D6  -> resistencia 220 ohm -> GND
    LED verde  D7  -> resistencia 220 ohm -> GND
    HC-SR04    TRIG -> D9, ECHO -> D10, VCC -> 5V, GND -> GND

  Libreria: LiquidCrystal_I2C de Frank de Brabander.
  Si la pantalla queda en blanco, corre i2c_scanner/ y ajusta la direccion.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // cambia 0x27 por 0x3F si hace falta

// ---------------- Pines ----------------
const uint8_t PIN_BOTON_ANTERIOR = 2;
const uint8_t PIN_BOTON_SIGUIENTE = 3;
const uint8_t PIN_BOTON_OK = 4;
const uint8_t PIN_LED_ROJO = 6;
const uint8_t PIN_LED_VERDE = 7;
const uint8_t PIN_TRIG = 9;
const uint8_t PIN_ECHO = 10;

// ---------------- Ajustes ----------------
const uint8_t VIDAS_INICIALES = 6;  // palabras de maximo 8 letras: 15 columnas
const unsigned long REBOTE_MS = 200;
const unsigned long PULSO_VERDE_MS = 300;
const unsigned long PARPADEO_MS = 100;  // medio parpadeo
const uint8_t PARPADEOS_ROJO = 3;
const unsigned long LED_ROJO_PERDIDA_MS = 5000;
const unsigned int DISTANCIA_MANO_CM = 15;
const unsigned long ESPERA_REINICIO_MS = 1500;

// ---------------- Palabras ----------------
const char* const PALABRAS[] = {
  "ARDUINO", "SENSOR", "BOTON", "PANTALLA", "JUEGO",
  "CABLE", "MOTOR", "TIERRA", "FUENTE", "LED"
};
const uint8_t TOTAL_PALABRAS = sizeof(PALABRAS) / sizeof(PALABRAS[0]);

// ---------------- Estado ----------------
enum Estado { ESPERANDO, JUGANDO, GANADO, PERDIDO };
Estado estado = ESPERANDO;

const char* palabra = "";
uint8_t largoPalabra = 0;
uint8_t ultimaPalabra = 255;
bool letrasUsadas[26];
uint8_t letraActual = 0;
uint8_t vidas = VIDAS_INICIALES;

bool botonAnteriorEstado = HIGH;
bool botonSiguienteEstado = HIGH;
bool botonOkEstado = HIGH;
unsigned long ultimoBotonAnterior = 0;
unsigned long ultimoBotonSiguiente = 0;
unsigned long ultimoBotonOk = 0;

unsigned long verdeHasta = 0;
unsigned long rojoHasta = 0;
unsigned long rojoUltimoCambio = 0;
bool rojoEncendido = false;
bool rojoFijo = false;
unsigned long finPartidaEn = 0;
unsigned long ultimaManoEn = 0;

// ---------------- Botones ----------------
bool leerBoton(uint8_t pin, bool &anterior, unsigned long &ultimo) {
  bool actual = digitalRead(pin);
  bool pulsado = false;

  if (actual == LOW && anterior == HIGH && millis() - ultimo > REBOTE_MS) {
    ultimo = millis();
    pulsado = true;
  }
  anterior = actual;
  return pulsado;
}

// ---------------- Sensor ultrasonico ----------------
unsigned int medirDistanciaCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duracion = pulseIn(PIN_ECHO, HIGH, 15000);  // ~2,5 m como maximo
  if (duracion == 0) return 999;
  return duracion / 58;
}

bool manoCerca() {
  unsigned int distancia = medirDistanciaCm();
  return distancia > 0 && distancia < DISTANCIA_MANO_CM;
}

// ---------------- Pantalla ----------------
void limpiarFila(uint8_t fila) {
  lcd.setCursor(0, fila);
  lcd.print(F("                "));
}

void mostrarPalabra() {
  limpiarFila(0);
  lcd.setCursor(0, 0);
  for (uint8_t i = 0; i < largoPalabra; i++) {
    char c = palabra[i];
    lcd.print(letrasUsadas[c - 'A'] ? c : '_');
    if (i + 1 < largoPalabra) lcd.print(' ');
  }
}

void mostrarPalabraRevelada() {
  limpiarFila(0);
  lcd.setCursor(0, 0);
  for (uint8_t i = 0; i < largoPalabra; i++) {
    lcd.print(palabra[i]);
    if (i + 1 < largoPalabra) lcd.print(' ');
  }
}

void mostrarSeleccion() {
  limpiarFila(1);
  lcd.setCursor(0, 1);
  lcd.print((char)('A' + letraActual));
  lcd.print(F("  Vidas: "));
  lcd.print(vidas);
}

// ---------------- Juego ----------------
bool palabraCompleta() {
  for (uint8_t i = 0; i < largoPalabra; i++) {
    if (!letrasUsadas[palabra[i] - 'A']) return false;
  }
  return true;
}

void iniciarPartida() {
  uint8_t indice = random(TOTAL_PALABRAS);
  if (TOTAL_PALABRAS > 1 && indice == ultimaPalabra) {
    indice = (indice + 1) % TOTAL_PALABRAS;
  }
  ultimaPalabra = indice;

  palabra = PALABRAS[indice];
  largoPalabra = strlen(palabra);
  for (uint8_t i = 0; i < 26; i++) letrasUsadas[i] = false;

  letraActual = 0;
  vidas = VIDAS_INICIALES;
  verdeHasta = 0;
  rojoHasta = 0;
  rojoFijo = false;
  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  ultimaManoEn = millis();

  estado = JUGANDO;
  mostrarPalabra();
  mostrarSeleccion();
}

void ganar() {
  estado = GANADO;
  finPartidaEn = millis();
  limpiarFila(1);
  lcd.setCursor(0, 1);
  lcd.print(F("GANASTE!"));
}

void perder() {
  estado = PERDIDO;
  finPartidaEn = millis();
  rojoFijo = true;
  digitalWrite(PIN_LED_ROJO, HIGH);

  mostrarPalabraRevelada();
  limpiarFila(1);
  lcd.setCursor(0, 1);
  lcd.print(F("PERDISTE"));
}

void confirmarLetra() {
  uint8_t indice = letraActual;
  if (letrasUsadas[indice]) return;  // letra ya probada: no gasta vida

  letrasUsadas[indice] = true;
  char c = 'A' + indice;

  if (strchr(palabra, c) != NULL) {
    digitalWrite(PIN_LED_VERDE, HIGH);
    verdeHasta = millis() + PULSO_VERDE_MS;
    mostrarPalabra();
    if (palabraCompleta()) ganar();
  } else {
    vidas--;
    if (vidas == 0) {
      perder();
    } else {
      rojoHasta = millis() + (PARPADEOS_ROJO * 2UL * PARPADEO_MS);
      rojoUltimoCambio = millis();
      rojoEncendido = true;
      digitalWrite(PIN_LED_ROJO, HIGH);
      mostrarSeleccion();
    }
  }
}

// ---------------- LEDs ----------------
void actualizarLeds() {
  unsigned long ahora = millis();

  if (rojoHasta != 0 && (long)(ahora - rojoHasta) >= 0) {
    rojoHasta = 0;
    digitalWrite(PIN_LED_ROJO, LOW);
  } else if (!rojoFijo && rojoHasta != 0 &&
             ahora - rojoUltimoCambio >= PARPADEO_MS) {
    rojoUltimoCambio = ahora;
    rojoEncendido = !rojoEncendido;
    digitalWrite(PIN_LED_ROJO, rojoEncendido ? HIGH : LOW);
  }

  if (verdeHasta != 0 && (long)(ahora - verdeHasta) >= 0) {
    verdeHasta = 0;
    digitalWrite(PIN_LED_VERDE, LOW);
  }
}

// ---------------- Inicio ----------------
void setup() {
  pinMode(PIN_BOTON_ANTERIOR, INPUT_PULLUP);
  pinMode(PIN_BOTON_SIGUIENTE, INPUT_PULLUP);
  pinMode(PIN_BOTON_OK, INPUT_PULLUP);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_LED_ROJO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("AHORCADO"));
  lcd.setCursor(0, 1);
  lcd.print(F("Acerca la mano"));

  randomSeed(analogRead(A0));
}

void loop() {
  unsigned long ahora = millis();
  actualizarLeds();

  switch (estado) {
    case JUGANDO:
      if (leerBoton(PIN_BOTON_ANTERIOR, botonAnteriorEstado, ultimoBotonAnterior)) {
        letraActual = (letraActual + 25) % 26;  // retrocede, con vuelta de A a Z
        mostrarSeleccion();
      }
      if (leerBoton(PIN_BOTON_SIGUIENTE, botonSiguienteEstado, ultimoBotonSiguiente)) {
        letraActual = (letraActual + 1) % 26;  // avanza, con vuelta de Z a A
        mostrarSeleccion();
      }
      if (leerBoton(PIN_BOTON_OK, botonOkEstado, ultimoBotonOk)) {
        confirmarLetra();
      }
      break;

    case ESPERANDO:
      if (manoCerca() && ahora - ultimaManoEn > 1000) {
        iniciarPartida();
      }
      break;

    case GANADO:
      if (manoCerca() && ahora - finPartidaEn > ESPERA_REINICIO_MS) {
        iniciarPartida();
      }
      break;

    case PERDIDO:
      if (rojoFijo && ahora - finPartidaEn >= LED_ROJO_PERDIDA_MS) {
        rojoFijo = false;
        digitalWrite(PIN_LED_ROJO, LOW);
        limpiarFila(1);
        lcd.setCursor(0, 1);
        lcd.print(F("Acerca la mano"));
      }
      if (!rojoFijo && manoCerca()) {
        iniciarPartida();
      }
      break;
  }
}
