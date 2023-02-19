import subprocess
from subprocess import PIPE
import re
import pandas as pd

SEED = [i for i in range(0,100)]#10)]

cost = pd.DataFrame(index=SEED,columns=['cost','ave','std'])

for seed in SEED:
   if seed < 10: filename = f'000{seed}'
   elif seed < 100: filename = f'00{seed}'
   elif seed < 1000: filename = f'0{seed}'
   else: input('error')
   res = subprocess.run(f'powershell -Command cat tester/inst/{filename}.txt | tester/./tester.exe ./out.exe > tester/res/{filename}.txt',\
      stderr=PIPE)
   total_cost = res.stderr.decode('utf-8')
   total_cost = int(re.sub(r'\D','',total_cost))
   cost.loc[seed,'cost'] = total_cost
   print(f'seed {filename}: ',total_cost)

cost.loc[SEED[0],['ave','std']] = cost['cost'].mean(), cost['cost'].std()
cost.to_csv('cost.csv')