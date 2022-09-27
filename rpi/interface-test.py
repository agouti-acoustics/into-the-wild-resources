from interface import Interface

itf = Interface('/dev/ttyACM0')

for i in range(5):
    itf.getdata(f'recording{i}', fname=f'testwavs/test{i}.wav')
