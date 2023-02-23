import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

SEED = [i for i in range(0,1)]#10)]

all_data = np.empty(0)
for seed in SEED:
   if seed < 10: filename = f'000{seed}'
   elif seed < 100: filename = f'00{seed}'
   elif seed < 1000: filename = f'0{seed}'
   else: input('error')

   inst = pd.read_csv(f'../{filename}.txt',delimiter=' ',skiprows=1,header=None)
   all_data = np.concatenate([all_data,np.ravel(inst.loc[:199,:].values)],0)
   
plt.hist(all_data,bins=50)
plt.show()