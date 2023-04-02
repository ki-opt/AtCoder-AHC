import subprocess
from subprocess import PIPE
import re
import pandas as pd

SEED =  [i for i in range(0,100)]#10)]

score = pd.DataFrame(index=SEED, columns=['score'])
for seed in SEED:
	if seed < 10: filename = f'000{seed}'
	elif seed < 100: filename = f'00{seed}'
	elif seed < 1000: filename = f'0{seed}'
	else: input('error')
	
	inst = pd.read_csv(f'inst/{seed}.txt')
   #input(res)
	data = pd.read_csv(f'tester/score/{filename}.txt',header=None)
	score.loc[seed,'score'] = data.iloc[0,0]
