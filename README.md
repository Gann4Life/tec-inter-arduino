# Ahorcado con Arduino

Juego del ahorcado para la materia Tecnologías Interactivas. Las palabras están precargadas en el código y la partida se controla con tres botones, un sensor ultrasónico, dos LEDs y una pantalla LCD 16x2 por I2C.

Las reglas completas están en [DESIGN.md](DESIGN.md).

## Componentes

- Arduino Uno o Nano
- LCD 16x2 con módulo I2C (PCF8574)
- 3 pulsadores
- 1 LED rojo y 1 LED verde
- 2 resistencias de 220 Ω
- Sensor ultrasónico HC-SR04

## Conexiones

| Componente | Pin Arduino |
|---|---|
| LCD I2C — VCC / GND | 5V / GND |
| LCD I2C — SDA / SCL | A4 / A5 |
| Botón 1 (letra anterior) | D2 → GND |
| Botón 2 (letra siguiente) | D3 → GND |
| Botón 3 (confirmar letra) | D4 → GND |
| LED rojo (con resistencia de 220 Ω) | D6 → GND |
| LED verde (con resistencia de 220 Ω) | D7 → GND |
| HC-SR04 — TRIG / ECHO | D9 / D10 |
| HC-SR04 — VCC / GND | 5V / GND |

Los botones usan `INPUT_PULLUP`, por eso se conectan directamente entre el pin y GND, sin resistencia externa.

## Instalación

1. Instala la librería **LiquidCrystal_I2C** de Frank de Brabander desde el Administrador de Librerías del IDE de Arduino.
2. Abre `lcd_screen_test.ino` y súbelo a la placa.
3. Si la pantalla queda en blanco, revisa el potenciómetro de contraste del módulo I2C.
4. Si sigue en blanco, abre `i2c_scanner/i2c_scanner.ino`, súbelo y mira la dirección que aparece en el monitor serial (9600 baudios). Ajusta esa dirección en la línea `LiquidCrystal_I2C lcd(0x27, 16, 2);`.

## Cómo se juega

1. Acerca la mano al sensor ultrasónico (menos de 15 cm) para empezar.
2. El **botón 1** retrocede a la letra anterior y el **botón 2** avanza a la siguiente; el recorrido es circular, de Z vuelve a A y viceversa.
3. El **botón 3** confirma la letra elegida.
   - Si la letra está en la palabra, el LED verde se enciende brevemente.
   - Si no está, el LED rojo parpadea 3 veces y se pierde una vida.
   - Una letra ya probada se ignora y no gasta vida.
4. La primera fila del LCD muestra la palabra con espacios vacíos (`_`) y la segunda la letra seleccionada junto a las vidas restantes.
5. Se gana al descubrir todas las letras. Se pierde al quedarse sin vidas: el LED rojo queda encendido 5 segundos y se revela la palabra.
6. Después de ganar o perder, acerca la mano al sensor para jugar otra vez.

## Ajustes

Los valores se pueden cambiar al inicio de `lcd_screen_test.ino`:

| Constante | Valor | Qué controla |
|---|---|---|
| `VIDAS_INICIALES` | 6 | Intentos por partida |
| `DISTANCIA_MANO_CM` | 15 | Distancia a la que el sensor inicia la partida |
| `PULSO_VERDE_MS` | 300 | Duración del encendido del LED verde |
| `PARPADEOS_ROJO` | 3 | Parpadeos del LED rojo al fallar |
| `LED_ROJO_PERDIDA_MS` | 5000 | Tiempo que el LED rojo queda encendido al perder |
| `REBOTE_MS` | 200 | Antirrebote de los botones |

Las palabras están en el arreglo `PALABRAS`. Deben escribirse en mayúsculas, sin Ñ ni tildes, y con un máximo de 8 letras para que entren en las 16 columnas del LCD.

## Estructura

- `lcd_screen_test.ino` — programa principal del juego.
- `i2c_scanner/i2c_scanner.ino` — utilidad para detectar la dirección I2C del LCD.
- `DESIGN.md` — diseño y reglas del juego.
