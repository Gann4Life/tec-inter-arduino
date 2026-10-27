# Proyecto Ahorcado

Proyecto de arduino para tecnologías interactivas, objetivo lúdico usando arduino.

Las palabras están precargadas en el código.

* Boton 1: Se utiliza para seleccionar la anterior letra.
* Boton 2: Se utiliza para seleccionar la siguiente letra.
* Boton 3: Se utiliza para confirmar la letra seleccionada.
* Sensor ultrasonico: se utiliza para empezar la partida cuando alguien acerca la mano.
* Pantalla LCD 
  Muestra el row de la palabra a adivinar con slots vacios pero visibles
  la segunda fila muestra la letra actualmente seleccionada.
  La vida (cantidad de intentos disponibles) tambien debe aparecer en pantalla.
* Led Rojo: 
  Parpadea 3 veces rapidamente cuando se selecciona una letra incorrecta.
  Se mantiene encendido 5 segundos cuando el jugador pierde. El sensor ultrasonico inicia una nueva partida.

* Led Verde: Se ilumina una sola vez cuando la letra seleccionada es correcta.

Nota (2026-10-26): Con la adición del tercer botón se está evaluando agregar un selector de dificultad antes de empezar la partida.
