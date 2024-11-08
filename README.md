this project is about using an esp32 microcontroller to interface with WS2812 LED matrix display and allowing the user to control it using bluetooth low energy BLE in their terminal app
it uses neomatrix and bluetooth low energy libraries
first install the boards manager for the esp32 to be able to use it from the arduino IDE and also add preferences link in the settings for the esp32
then install essenatial ADAfruit neomatrix libraries

connect the esp32 to the ws2812 display using 5v and ground wires and the 3rd data-in wire to any pin
it's not necessary but connecting the data pin to a resistor will help protect the matrix display from any electric spikes
