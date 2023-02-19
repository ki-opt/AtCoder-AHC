import subprocess
from subprocess import PIPE
import re

SEED = [i for i in range(10)]

#cost =
for seed in SEED:
   filename = f'000{seed}'
   res = subprocess.run(f'powershell -Command cat tester/inst/{filename}.txt | tester/./tester.exe ./out.exe > tester/res/{filename}.txt',\
      stderr=PIPE)
   total_cost = res.stderr.decode('utf-8')
   total_cost = int(re.sub(r'\D','',total_cost))
   print(total_cost)