
import numpy as np
import matplotlib.pyplot as plt
import random as rd

# saf

##################### drl #####################
# (50, 100, 200) - 1718 : 389
dis_drl_1 = np.array([13.35, 13.41, 13.44, 13.46, 13.49])
lat_drl_1 = np.array([12.44, 12.55, 12.54, 12.49, 12.50])
spa_drl_1 = np.array([12.65, 12.57, 12.58, 12.59, 12.57])

# (50, 100, 400) - 1718 : 804
dis_drl_2 = np.array([41.97, 40.61, 41.61, 39.96, 41.40])
lat_drl_2 = np.array([22.07, 22.06, 21.99, 22.08, 22.2])
spa_drl_2 = np.array([20.80, 21.23, 19.92, 20.69, 21.27])

# (50, 100, 600) - 1718 : 1183
dis_drl_3 = np.array([45.84, 45.79, 45.77, 45.96, 45.75])
lat_drl_3 = np.array([23.49, 24.01, 23.78, 23.91, 23.45])
spa_drl_3 = np.array([21.02, 20.92, 20.87, 21.39, 20.89])

# (50, 100, 800) - 1718 : 1594
dis_drl_4 = np.array([46.09, 45.25, 46.02, 46.82, 47.22])
lat_drl_4 = np.array([27.33, 27.49, 26.56, 27.07, 27.38])
spa_drl_4 = np.array([29.16, 31.64, 25.12, 29.03, 25.79])

##################### avg replica #####################
# (50, 100, 200)
dis_rep_1 = np.array([1, 1, 1, 1, 1])
lat_rep_1 = np.array([1, 1, 1, 1, 1])
spa_rep_1 = np.array([1, 1, 1, 1, 1])

# (50, 100, 400)
dis_rep_2 = np.array([3.45, 3.37, 3.43, 3.28, 3.44])
lat_rep_2 = np.array([1.77, 1.77, 1.77, 1.75, 1.77])
spa_rep_2 = np.array([1.66, 1.69, 1.58, 1.65, 1.69])

# (50, 100, 600)
dis_rep_3 = np.array([5.03, 5.07, 5.00, 5.05, 5.01])
lat_rep_3 = np.array([2.24, 2.32, 2.29, 2.31, 2.26])
spa_rep_3 = np.array([1.93, 1.92, 1.91, 1.98, 1.91])

# (50, 100, 800)
dis_rep_4 = np.array([6.44, 6.21, 6.44, 6.6, 6.6])
lat_rep_4 = np.array([3.28, 3.33, 3.12, 3.26, 3.33])
spa_rep_4 = np.array([3.65, 4.1, 2.92, 3.6, 3.03])

##################### replica overhead #####################
# (50, 100, 400)
dis_ro_2 = np.array([90.29, 89.44, 89.81, 90.04, 89.12])
lat_ro_2 = np.array([85.61, 85.47, 84.58, 85.66, 86.83])
spa_ro_2 = np.array([84.6, 85.34, 83.6, 84.43, 85.3])

# (50, 100, 600)
dis_ro_3 = np.array([91.22, 90.59, 91.66, 91.15, 91.29])
lat_ro_3 = np.array([87.09, 86.22, 86.33, 86.22, 85.35])
spa_ro_3 = np.array([84.05, 84.05, 84.21, 84.74, 84.52])

# (50, 100, 800)
dis_ro_4 = np.array([90.59, 90.45, 91.09, 90.19, 90.92])
lat_ro_4 = np.array([88.34, 87.72, 89.31, 87.32, 86.80])
spa_ro_4 = np.array([87.02, 87.05, 86.64, 87.95, 87.11])

##################### failed replica overhead #####################
# (50, 100, 400)
dis_fr_4 = np.array([15.99, 14.90, 13.79, 14.78, 21.7])

###############################################################

dis_drl_mean = [np.mean(dis_drl_1), np.mean(dis_drl_2), np.mean(dis_drl_3), np.mean(dis_drl_4)]
lat_drl_mean = [np.mean(lat_drl_1), np.mean(lat_drl_2), np.mean(lat_drl_3), np.mean(lat_drl_4)]
spa_drl_mean = [np.mean(spa_drl_1), np.mean(spa_drl_2), np.mean(spa_drl_3), np.mean(spa_drl_4)]

dis_drl_std = [np.std(dis_drl_1), np.std(dis_drl_2), np.std(dis_drl_3), np.std(dis_drl_4)]
lat_drl_std = [np.std(lat_drl_1), np.std(lat_drl_2), np.std(lat_drl_3), np.std(lat_drl_4)]
spa_drl_std = [np.std(spa_drl_1), np.std(spa_drl_2), np.std(spa_drl_3), np.std(spa_drl_4)]

dis_rep_mean = [np.mean(dis_rep_1), np.mean(dis_rep_2), np.mean(dis_rep_3), np.mean(dis_rep_4)]
lat_rep_mean = [np.mean(lat_rep_1), np.mean(lat_rep_2), np.mean(lat_rep_3), np.mean(lat_rep_4)]
spa_rep_mean = [np.mean(spa_rep_1), np.mean(spa_rep_2), np.mean(spa_rep_3), np.mean(spa_rep_4)]

dis_rep_std = [np.std(dis_rep_1), np.std(dis_rep_2), np.std(dis_rep_3), np.std(dis_rep_4)]
lat_rep_std = [np.std(lat_rep_1), np.std(lat_rep_2), np.std(lat_rep_3), np.std(lat_rep_4)]
spa_rep_std = [np.std(spa_rep_1), np.std(spa_rep_2), np.std(spa_rep_3), np.std(spa_rep_4)]

dis_ro_mean = [np.mean(dis_ro_2), np.mean(dis_ro_3), np.mean(dis_ro_4)]
lat_ro_mean = [np.mean(lat_ro_2), np.mean(lat_ro_3), np.mean(lat_ro_4)]
spa_ro_mean = [np.mean(spa_ro_2), np.mean(spa_ro_3), np.mean(spa_ro_4)]

dis_ro_std = [np.std(dis_ro_2), np.std(dis_ro_3), np.std(dis_ro_4)]
lat_ro_std = [np.std(lat_ro_2), np.std(lat_ro_3), np.std(lat_ro_4)]
spa_ro_std = [np.std(spa_ro_2), np.std(spa_ro_3), np.std(spa_ro_4)]


ind = np.arange(len(dis_drl_mean))

width = 0.3

fig, ax = plt.subplots()
rects1 = ax.bar(ind - width, dis_drl_mean, width, yerr=dis_drl_std, label='Distance', capsize=5, hatch='\\\\')
rects2 = ax.bar(ind, lat_drl_mean, width, yerr=lat_drl_std, label='Latency', capsize=5, hatch='//')
rects3 = ax.bar(ind + width, spa_drl_mean, width, yerr=spa_drl_std, label='Spatial', capsize=5, hatch='-')
ax.set_xticks(ind)
ax.set_xticklabels(['50,100,200', '50,100,400', '50,100,600', '50,100,800'])
ax.set_xlabel('#Host,#Producer,#Consumer')
ax.set_ylabel('Average data retrieval latency (ms)')
ax.legend()
fig.tight_layout()
plt.show()

fig, ax = plt.subplots()
rects1 = ax.bar(ind - width, dis_rep_mean, width, yerr=dis_rep_std, label='Distance', capsize=5, hatch='\\\\')
rects2 = ax.bar(ind, lat_rep_mean, width, yerr=lat_rep_std, label='Latency', capsize=5, hatch='//')
rects3 = ax.bar(ind + width, spa_rep_mean, width, yerr=spa_rep_std, label='Spatial', capsize=5, hatch='-')
ax.set_xticks(ind)
ax.set_xticklabels(['50,100,200', '50,100,400', '50,100,600', '50,100,800'])
ax.set_xlabel('#Host,#Producer,#Consumer')
ax.set_ylabel('Average replica count per producer')
ax.legend()
fig.tight_layout()
plt.show()

ind = np.arange(len(dis_ro_mean))
fig, ax = plt.subplots()
rects1 = ax.bar(ind - width, dis_ro_mean, width, yerr=dis_ro_std, label='Distance', capsize=5, hatch='\\\\')
rects2 = ax.bar(ind, lat_ro_mean, width, yerr=lat_ro_std, label='Latency', capsize=5, hatch='//')
rects3 = ax.bar(ind + width, spa_ro_mean, width, yerr=spa_ro_std, label='Spatial', capsize=5, hatch='-')
ax.set_ylim([60, 100])
ax.set_xticks(ind)
ax.set_xticklabels(['50,100,400', '50,100,600', '50,100,800'])
ax.set_xlabel('#Host,#Producer,#Consumer')
ax.set_ylabel('Replica overhead (ms)')
ax.legend()
fig.tight_layout()
plt.show()