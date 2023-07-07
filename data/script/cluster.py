import pandas as pd
import sys
from sklearn.cluster import KMeans
import seaborn as sns
import matplotlib.pyplot as plt

df = pd.read_csv(sys.argv[1])

kmeans = KMeans(n_clusters=5, init='k-means++', random_state=0).fit(df[['lat', 'lon']])
df['mach'] = kmeans.labels_

for cluster in range(5):
    cluster_data = df.loc[df['mach'] == cluster]
    cluster_data.to_csv('dataset/c'+str(cluster)+'.csv', index=False)