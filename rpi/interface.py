import time
import serial
import numpy as np
from scipy.io import wavfile

def decode(data):
    return [i for i in data]

def norm(data):
    dat = np.asarray(data, dtype=np.float32)
    dat = (dat - 127.5) / 127.5
    return dat

class Interface():
    def __init__(self, port, sr=16000, sec=5, br=115200, ansatz=33):
        self.ser = serial.Serial(port, baudrate=br)
        self.ser.reset_input_buffer()

        self.SR = sr
        self.SEC = sec
        self.ansatz = ansatz
    
    def getdata(self, prompt="recording...", fname=None, printf=lambda *x:print(*x)):
        self.ser.write((prompt + '\n').encode('ascii'))
        time.sleep(0.001)
        self.ser.write((str(self.ansatz)).encode('ascii'))
        
        printf("receiving data")
        sensors = self.ser.readline()

        start = time.time()
        vals = self.ser.read(self.SR * self.SEC + 2)
        end = time.time()

        sensors = sensors.decode('ascii').split(',')
        sensors = tuple(map(lambda x:float(x), sensors))

        delay_ret, data, check = vals[0], vals[1:-1], vals[-1]
        assert delay_ret == self.ansatz
        assert check == 0

        data = norm(decode(data))

        time_micros = (end - start) * 1e+6
        delay_micros = int(((1e+6 * self.SEC) - time_micros) / (self.SR * self.SEC))
        self.ansatz = max(0, self.ansatz + delay_micros)  # Update stuff

        printf(data.max(), data.min(), data.dtype)
        printf(f"{self.SR * self.SEC} frames: {time_micros} micros")
        printf(f"Suggested delay /frame: {delay_micros} micros")

        if fname is not None:
            wavfile.write(fname, self.SR, data)
        
        return sensors, data
    
    def save(self, data, fname):
        wavfile.write(fname, self.SR, data)

    def close(self):
        self.ser.close()
