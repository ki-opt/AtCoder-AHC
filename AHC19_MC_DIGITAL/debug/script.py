import subprocess
from subprocess import PIPE
import re
import pandas as pd

SEED =  [i for i in range(0,1)]#10)]

for seed in SEED:
   if seed < 10: filename = f'000{seed}'
   elif seed < 100: filename = f'00{seed}'
   elif seed < 1000: filename = f'0{seed}'
   else: input('error')
   res = subprocess.run(f'powershell -Command ./out.exe {filename}.txt', stderr=PIPE)
   res = subprocess.run(f'powershell -Command tester/compiled/./vis.exe tester/inst/{filename}.txt tester/out/{filename}.txt')
   #print(res)