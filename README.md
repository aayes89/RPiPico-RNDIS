# RPiPico-RNDIS
Convierte tu RPiPico en RNDIS sin necesidad de librerías externas como <b>pico-sdk</b> o <b>TinyUsb</b>.

# Objetivo
- Crear un adaptador RNDIS compatible con varios sistemas operativos utilizando exclusivamente Arduino IDE. (bare-metal)
- Establecer una librería independiente para otro tipo de proyectos similares.
  
# Requisitos
- Arduino IDE.
- Raspberry Pi Pico.
- Cable USB micro B compatible.

# Modo BOOTSEL en Raspberry Pi Pico (RPiPico)
- Conectar cable USB micro B en la RPiPico.
- Mantener presionado el botón BOOTSEL mientras conectas el cable al PC.
- Soltar botón cuando conectes el cable al PC y se haya escuchado un sonido de dispositivo de almacenamiento conectado.
- Si aparece una nueva unidad con nombre RPI-RP2, entonces el modo BOOTSEL está activado y listo para programar la RPiPico.

# Como usar
- Instalar Arduino IDE, de preferencia la 2.3.5, pero cualquier versión debe funcionar.
- Importar o Abrir el fichero rndis.ino.
- Añadir el repositorio: <b>https://raw.githubusercontent.com/DeqingSun/ch55xduino/ch55xduino/package_ch55xduino_mcs51_index.json</b> para Raspberry Pi Pico.
- Instalar la placa <b>Arduino Mbed OS RP2040 Boards by Arduino</b>
- Seleccionar en el Menu de herramientas la placa <b>Arduino Mbed OS RP2040</b> -> <b>Raspberry Pi Pico</b>
- Seleccionar la opción <b>Exportar binario compilado</b> en el menu <b>Sketch</b>
- Copiar el fichero <b>.uf2</b> resultante a la memoria RPI-RP2.
- Si la memoria desaparece, FELICIDADES! tu RPiPico ha sido programado con el nuevo firmware.
- Si no vez la memoria de almacenamiento RPI-RP2, repetir los pasos en la sección: <b>Modo BOOTSEL en Raspberry Pi Pico</b>

# Por hacer
- Firmar el controlador para que pueda instalarse correctamente en Windows sin necesidad de deshabilitar la opción en el sistema para permitirlos.
- Crear una interfaz para configurar el adaptador (utilizar la implementación nativa del sistema o una propia).
- Optimizar el código INO para cumplir al 100% con el estándar RNDIS de Windows.

  

