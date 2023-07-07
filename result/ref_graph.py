import numpy as np
import matplotlib.pyplot as plt
import random as rd

low = 30
high = 50

s1 = np.array([rd.uniform(low, high) for _ in range(10)])
s2 = np.array([rd.uniform(low, high) for _ in range(10)])
s3 = np.array([rd.uniform(low, high) for _ in range(10)])

s1_mean = np.mean(s1)
s2_mean = np.mean(s2)
s3_mean = np.mean(s3)

s1_std = np.std(s1)
s2_std = np.std(s2)
s3_std = np.std(s3)

ind = np.arange(len(s1))

width = 0.3

fig, ax = plt.subplots()
rects1 = ax.bar(ind - width, s1_mean, width, yerr=s1_std, label='s1')
rects2 = ax.bar(ind, s2_mean, width, yerr=s2_std, label='s2')
rects3 = ax.bar(ind + width, s3_mean, width, yerr=s3_std, label='s3')

ax.legend()
fig.tight_layout()
plt.show()