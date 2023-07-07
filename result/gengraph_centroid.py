import numpy as np
import sys

path = sys.argv[1] 
producer = np.array([])
consumer = np.array([])

success_replica_elapsed_time = 0
fail_replica_elapsed_time = 0

replica_selection_time = 0
failed_replica_selection_time = 0

e2e_latency = 0
c_e2e_latency = 0
n_e2e = 0

prod_host_selection_time = 0
prod_host_selection_latency = 0
cons_host_decision_latency = 0
cons_host_selection_latency = 0

n_e2e_rep = 0
n_replica_success = 0
n_replica_fail = 0

prod_rep = {}
n_prods = 0
n_cons = 0
log_data = open(path, 'r')
for line in log_data:
    columns = line.strip().split(' ')
    if(columns[0] == "SRET"): #Successfull Replica elapsed time
        success_replica_elapsed_time += float(columns[1])
        n_replica_success += 1
    elif(columns[0] == "NRET"):
        fail_replica_elapsed_time += float(columns[1])
        n_replica_fail += 1
    elif(columns[0] == "RSL"): # Replica selection time
        replica_selection_time += float(columns[1])
    elif(columns[0] == "FRSL"): # Replica selection time
        failed_replica_selection_time += float(columns[1])
    elif(columns[0] == "E2EL"): # End to end latency
        e2e_latency += float(columns[1])
        n_e2e_rep += 1
    # elif(columns[0] == "PHST"): # Product host selection time
    #     prod_host_selection_time += float(columns[1])
    elif(columns[0] == "PHST"): # Product host selection latency
        prod_host_selection_latency += float(columns[1])
        n_prods += 1
    elif(columns[0] == "CHST"): # Product host selection latency
        cons_host_selection_latency += float(columns[1])
        n_cons += 1
    elif(columns[0] == "CHDL"):
        cons_host_decision_latency += float(columns[1])
    elif(columns[0] == "REPLICA"):
        prod_rep[columns[1]] = int(columns[2])
    elif(columns[0] == "C"):
        c_e2e_latency += float(columns[1])
        n_e2e += 1


print("End to end latency without replica overhead: ", c_e2e_latency/float(n_e2e))

print("Number of success replica: ", n_replica_success)

if(n_replica_success != 0):
    print("Success replica overhead: ", (success_replica_elapsed_time + replica_selection_time)/float(n_replica_success))

if(n_replica_fail != 0):
    print("Failed replica overhead: ", (fail_replica_elapsed_time + failed_replica_selection_time)/float(n_replica_fail))

print("End to end latency with replica overhead: ", (e2e_latency + success_replica_elapsed_time + replica_selection_time) /float(n_e2e))
print("E2E + replica overhead: ", (e2e_latency + success_replica_elapsed_time + replica_selection_time) /float(n_e2e_rep))

print("Consumers with no host: ", n_replica_fail)

max_replicas = 0
for key in prod_rep:
    max_replicas += prod_rep[key]

if(n_prods != 0):
    print("Average number of replicas: ", float(max_replicas)/float(n_prods))

print("Average producer host selection time: ", float(prod_host_selection_latency)/float(n_prods))
print("Average consumer host selection time: ", float(cons_host_selection_latency)/float(n_cons))

print("End to end latency: ", (c_e2e_latency + e2e_latency + success_replica_elapsed_time + replica_selection_time) /float(n_e2e))