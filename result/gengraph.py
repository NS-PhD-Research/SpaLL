import numpy as np
import sys

path = sys.argv[1] 
producer = np.array([])
consumer = np.array([])
replica_call = 0
no_host_cons = 0
no_host_prod = 0
replica_time = 0
transfer_time = 0

log_data = open(path, 'r')
for line in log_data:
    columns = line.strip().split(' ')
    if columns[0] == 'P':
        producer = np.append(producer, float(columns[1]))
    elif columns[0] == 'C':
        consumer = np.append(consumer, float(columns[1]))
    elif columns[0] == "R":
        replica_time += float(columns[1])
        replica_call += 1
    elif columns[0] == "SHC":
        no_host_cons += 1
    elif columns[0] == "SHP":
        no_host_prod += 1
    elif columns[0] == "TL":
        transfer_time += float(columns[1])


print("Producer latency:", np.average(producer))
print("Consumer latency: ", np.average(consumer))

total_time = sum(consumer) + replica_time + transfer_time
print("Average data retrieval latency: ", total_time/float(len(consumer)))

n_replica = replica_call - no_host_cons
print("Number of replica: ", n_replica)

if(replica_call != 0):
    print("Replica overhead: ", (replica_time + transfer_time)/replica_call)
    
print("Consumers with no host: ", no_host_cons)
print("Producers with no host: ", no_host_prod)
