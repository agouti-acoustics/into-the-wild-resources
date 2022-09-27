from ai import AI
from interface import Interface

from datetime import datetime
import time
import json
import os


def count(term, labs):
    return sum([1 if term in i.lower() else 0 for i in labs])

def postproc(cat, labs, checks={'vehicle': 0.6, 'gun': 0.5, 'music': 0.4, 'speech': 0.4, 'noise': 0.5}):
    for k in checks:
        if count(k, labs) >= int(len(labs) * checks[k]):
            return k
    return cat

def mv(file, dest):
    os.system(f'cp --no-preserve=mode "{file}" "{dest}"')
    os.system(f'sudo rm "{file}"')

PORT = '/dev/ttyACM0'
STORAGE = "/media/pi/ADATA UFD/Agouti"
INTERVAL = 60
DANGER = ['sawing', 'intrusive', 'fire', 'vehicle', 'gun']

model = AI()
itf = Interface(PORT)
ct = "initialising"
last = time.time()

with open('startup.txt', 'w') as f:
  f.write(str(datetime.now()))

while True:

    (light, temp, hum), wav = itf.getdata(prompt=ct)
    print(light, temp, hum)

    orig_ct, cl, lb = model(wav)
    ct = postproc(orig_ct, lb)
    print(ct, cl, lb)

    if cl in DANGER or ct in DANGER or (time.time() - last) > INTERVAL:
        print('Saving data')

        signature = str(datetime.now())
        signature = signature.replace(' ', '_')
        signature = signature.replace(':', '.')

        bundle = {
            'light': light, 'temperature': temp, 'humidity': hum,
            'category': ct, 'original': orig_ct, 
            'class': cl, 'yamnet': lb
        }

        itf.save(wav, f'{signature}.wav')
        with open(f'{signature}.json', 'w') as f:
            json.dump(bundle, f, indent=4, sort_keys=True)

        mv(f'{signature}.wav', STORAGE)
        mv(f'{signature}.json', STORAGE)

        if (time.time() - last) > INTERVAL:
            last = time.time()

    time.sleep(0.1)

itf.getdata(prompt=ct)
itf.close()
