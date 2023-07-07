import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('result/histogram.csv')

fig, ax = plt.subplots(nrows=1, ncols=3)
ax[0].hist(df['Distance'], histtype='bar', ec='black')
ax[0].set_xlabel('Distance overhead(ms)')
ax[0].set_ylabel('Number of replicas')
ax[1].hist(df['Latency'], histtype='bar', ec='black')
ax[1].set_xlabel('Latency overhead(ms)')
ax[2].hist(df['Spatial'], histtype='bar', ec='black')
ax[2].set_xlabel('Spatial overhead(ms)')

plt.show()