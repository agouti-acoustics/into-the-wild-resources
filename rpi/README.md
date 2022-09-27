# Setting Up Agouti on Raspberry Pi

## Initial Setup

1. Clone this repository, then navigate into this directory.

2. Run `sudo python3 -m pip install -r requirements.txt`.

3. Use `sudo python3 -m serial.tools.miniterm` to check for the name of the port where the Wio Terminal is connected to.

4. Plug in your USB stick, locate it in `/media/pi/`, and create a folder on it to store audio and sensor data

5. Add your port and usb to `main.py` (lines 23-24), i.e.:

```python
PORT = '<YOUR_PORT_HERE>' # e.g. /dev/ttyACM0
STORAGE = "<YOUR_STORAGE_HERE>" # e.g. /media/pi/USB/Agouti
```

6. Now you can run `sudo python3 main.py` with your Wio connected, and hopefully magic will happen!

### Running program on startup

To enable your Raspberry Pi to run the program automatically on startup, do the following steps:

1. Run `sudo nano /etc/rc.local`. This will launch the GNU nano text editor. Scroll to the bottom of the file (but before `exit 0`).

2. Add the following line before `exit 0`, replacing `<agouti_rpi_dir>` with the _absolute_ filepath to the directory containing your `main.py` file (e.g. `/home/pi/Desktop/Agouti/into-the-wild-resources/rpi`):

   ```bash
   cd <agouti_rpi_dir> ; sudo python3 main.py &
   ```

   _The `cd` command is really important as it ensures all relative filepaths work in `main.py`. The amperstand '&' at the end is also important: it stops the infinite loop in `main.py` from blocking other processes._

   So, for example, the last few lines of my `rc.local` file ended up looking like this:

   ```bash
   if [ "$_IP" ]; then
     printf "My IP address is %s\n" "$_IP"
   fi

   cd /home/pi/Desktop/Agouti/into-the-wild-resources/rpi ; sudo python3 main.py &

   exit 0
   ```

## Using a custom model

You can replace the existing `metadata.json` and `toplayer.tflite` files with your own versions. These are generated automatically after you train your AI on Google Colab. See the [AI](../ai) directory.
