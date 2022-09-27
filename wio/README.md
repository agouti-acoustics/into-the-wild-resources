# Setting up Agouti on Wio Terminal

1. Read this [Getting Started](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/) guide from Seeed for the basics of uploading code to Wio.

2. Download all the files in the [`arduino-core`](https://github.com/Sensirion/arduino-core) and [`arduino-i2c-sht4x`](https://github.com/Sensirion/arduino-i2c-sht4x) repositories from Sensirion **as a ZIP file**.

3. In Arduino IDE, use `sketch -> Include Library -> Add .ZIP Library` to add the two above libraries. This allows the Wio to interface with and read data from the SHT40 sensor.

Now you should be able to upload our code for running on the Wio Terminal.
