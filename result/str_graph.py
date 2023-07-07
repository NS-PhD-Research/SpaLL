
import numpy as np
import matplotlib.pyplot as plt
import random as rd

# str
##################### e2e #####################
# (50, 100, 200) - 1706 : 412
dis_e2e_1 = np.array([28.09])
lat_e2e_1 = np.array([19.26])
spa_e2e_1 = np.array([19.11])

# (50, 100, 400) - 1706 : 805
dis_e2e_2 = np.array([28.00, 27.89, 28.11, 28.12, 27.87])
lat_e2e_2 = np.array([19.54, 19.75, 19.57, 19.63, 19.55])
spa_e2e_2 = np.array([19.86, 19.79, 19.50, 19.80, 19.56])

# (50, 100, 600) - 1706 : 1193
dis_e2e_3 = np.array([27.96, 28.35, 28.03, 27.98, 28.16])
lat_e2e_3 = np.array([19.99, 19.74, 19.94, 19.82, 19.78])
spa_e2e_3 = np.array([19.87, 19.51, 20.04, 19.81, 19.99])

# (50, 100, 800) - 1706 : 1608
dis_e2e_4 = np.array([28.41, 28.17, 28.26, 28.30, 29.11])
lat_e2e_4 = np.array([19.74, 19.94, 19.94, 19.81, 19.80])
spa_e2e_4 = np.array([20.50, 19.71, 19.95, 19.83, 19.95])

##################### avg replica #####################
# (50, 100, 200)
dis_rep_1 = np.array([1, 1, 1, 1, 1])
lat_rep_1 = np.array([1, 1, 1, 1, 1])
spa_rep_1 = np.array([1, 1, 1, 1, 1])

# (50, 100, 400)
dis_rep_2 = np.array([3.09, 3.04, 3.1, 3.03, 3.05])
lat_rep_2 = np.array([1.92, 1.93, 1.91, 1.94, 1.94])
spa_rep_2 = np.array([1.92, 1.94, 1.94, 1.90, 2.00])

# (50, 100, 600)
dis_rep_3 = np.array([4.58, 4.62, 4.56, 4.58, 4.60])
lat_rep_3 = np.array([2.44, 2.46, 2.44, 2.45, 2.47])
spa_rep_3 = np.array([2.34, 2.26, 2.33, 2.30, 2.35])

# (50, 100, 800)
dis_rep_4 = np.array([5.99, 6.07, 6.09, 6.00, 6.10])
lat_rep_4 = np.array([3.63, 3.51, 3.51, 3.60, 3.55])
spa_rep_4 = np.array([4.21, 4.40, 4.09, 4.10, 4.30])

##################### replica overhead #####################
# (50, 100, 400)
dis_ro_2 = np.array([58.70, 58.01, 58.50, 58.53, 57.76])
lat_ro_2 = np.array([58.74, 58.026, 58.80, 58.66, 56.21])
spa_ro_2 = np.array([58.26, 57.81, 58.29, 57.91, 58.88])

# (50, 100, 600)
dis_ro_3 = np.array([60.25, 60.14, 58.78, 59.01, 59.05])
lat_ro_3 = np.array([57.93, 58.05, 59.48, 58.43, 58.11])
spa_ro_3 = np.array([58.31, 58.65, 58.98, 58.32, 58.98])

# (50, 100, 800)
dis_ro_4 = np.array([59.03, 59.95, 58.83, 58.20, 59.13])
lat_ro_4 = np.array([59.58, 59.22, 59.45, 59.30, 59.45])
spa_ro_4 = np.array([59.66, 60.22, 59.99, 59.88, 60.10])

##################### failed replica overhead #####################
# (50, 100, 800)
dis_fr_4 = np.array([26, 21, 23])

###############################################################

dis_e2e_mean = [np.mean(dis_e2e_1), np.mean(dis_e2e_2), np.mean(dis_e2e_3), np.mean(dis_e2e_4)]
lat_e2e_mean = [np.mean(lat_e2e_1), np.mean(lat_e2e_2), np.mean(lat_e2e_3), np.mean(lat_e2e_4)]
spa_e2e_mean = [np.mean(spa_e2e_1), np.mean(spa_e2e_2), np.mean(spa_e2e_3), np.mean(spa_e2e_4)]

dis_e2e_std = [np.std(dis_e2e_1), np.std(dis_e2e_2), np.std(dis_e2e_3), np.std(dis_e2e_4)]
lat_e2e_std = [np.std(lat_e2e_1), np.std(lat_e2e_2), np.std(lat_e2e_3), np.std(lat_e2e_4)]
spa_e2e_std = [np.std(spa_e2e_1), np.std(spa_e2e_2), np.std(spa_e2e_3), np.std(spa_e2e_4)]

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

ind = np.arange(len(dis_e2e_mean))
plt.plot(ind, dis_e2e_mean)
plt.errorbar(ind, dis_e2e_mean, yerr=dis_e2e_std, capsize=5)

plt.plot(ind, lat_e2e_mean)
plt.errorbar(ind, lat_e2e_mean, yerr=lat_e2e_std, capsize=5)

plt.plot(ind, dis_rep_mean)
plt.errorbar(ind, dis_rep_mean, yerr=dis_rep_std, capsize=5)

plt.show()

ind = np.arange(len(dis_e2e_mean))

width = 0.3

fig, ax = plt.subplots()
rects1 = ax.bar(ind - width, dis_e2e_mean, width, yerr=dis_e2e_std, label='Distance', capsize=5, hatch='\\\\')
rects2 = ax.bar(ind, lat_e2e_mean, width, yerr=lat_e2e_std, label='Latency', capsize=5, hatch='//')
rects3 = ax.bar(ind + width, spa_e2e_mean, width, yerr=spa_e2e_std, label='Spatial', capsize=5, hatch='-')
ax.set_xticks(ind)
ax.set_xticklabels(['50,100,200', '50,100,400', '50,100,600', '50,100,800'])
ax.set_xlabel('#Host,#Producer,#Consumer')
ax.set_ylabel('Average end to end latency (ms)')
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
# ax.set_ylim([60, 100])
ax.set_xticks(ind)
ax.set_xticklabels(['50,100,400', '50,100,600', '50,100,800'])
ax.set_xlabel('#Host,#Producer,#Consumer')
ax.set_ylabel('Replica overhead (ms)')
ax.legend()
fig.tight_layout()
plt.show()