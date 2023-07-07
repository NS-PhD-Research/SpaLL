import numpy as np
import sys

path = sys.argv[1] 
producer = np.array([])
consumer = np.array([])

success_replica_elapsed_time = 0
fail_replica_elapsed_time = 0
replica_overhead = 0
e2e_latency = 0
cons_host_selection_time = 0;

n_e2e = 0
n_replica_success = 0
n_replica_fail = 0
n_cons= 0

prod_rep = {}

log_data = open(path, 'r')
for line in log_data:
    columns = line.strip().split(' ')
    if(columns[0] == "SRET"): #Successful replica elapsed time
        success_replica_elapsed_time += float(columns[1])
        n_replica_success += 1

    elif(columns[0] == "NRET"):  #Failed replica elapsed time
        fail_replica_elapsed_time += float(columns[1])
        n_replica_fail += 1

    elif(columns[0] == "RO"): # Replication overhead
        replica_overhead += float(columns[1])
    
    elif(columns[0] == "E2EL"): # End to end latency
        e2e_latency += float(columns[1])
        n_e2e += 1

    elif(columns[0] == "REPLICA"):
        prod_rep[columns[1]] = int(columns[2])

    elif(columns[0] == "CHST"): # Consumer host selection latency
        cons_host_selection_time += float(columns[1])
        n_cons += 1


n_prod = len(prod_rep)

# End to end latency
print("End to end latency: ", e2e_latency/float(n_e2e))

# Average replica count per producer
max_replicas = 0
for key in prod_rep:
    max_replicas += prod_rep[key]
print("Average replica per producer: ", float(max_replicas)/float(n_prod))

# Replication overhead : RO + SRET
if(n_replica_success != 0):
    print("Replication overhead: ", (replica_overhead + success_replica_elapsed_time)/float(n_replica_success))

# Denied consumer request count
print("Number of denied consumer request: ", n_replica_fail)

# Average consumer host selection time
if(n_cons != 0):
    print("Consumer selection time: ", cons_host_selection_time/float(n_cons))

# Replica selection time
if(n_replica_success != 0):
    print("Replica selection time: ", success_replica_elapsed_time/ float(n_replica_success))
