
import  os
import csv
import re
'''
files_list=[]
path = "/home/chao/LAPKT-public/benchmarks/ipc-2011/blocksworld/instances"
files = os.listdir(path)
s = []
for file in files:
     if not os.path.isdir(file):
          files_list.append(file)

result_dict={}

print(files_list)
#files_list=["duality_blocksaips01.pddl","blocksaips01.pddl"]
files_list.sort()
for file in files_list:
    print(file)
    result=os.popen("./siw --domain  /home/chao/LAPKT-public/benchmarks/ipc-2011/blocksworld/domain.pddl --problem /home/chao/LAPKT-public/benchmarks/ipc-2011/blocksworld/instances/"+file+" --output ./blocksworld_result/"+file+"_result.txt --bound 2").read()
    result_dict[file] = {}
    for line in result.split("\n"):
        if "Plan found with cost" in line:
            result_dict[file]["Plan found with cost"] = line.split(":")[-1]
        if "Total time" in line:
            result_dict[file]["Total time"]=line.split(":")[-1]
        if "Nodes generated during search" in line:
            result_dict[file]["Nodes generated during search"] = line.split(":")[-1]
        if "Nodes expanded during search" in line:
            result_dict[file]["Nodes expanded during search"] = line.split(":")[-1]
        if "IW search completed in" in line:
            result_dict[file]["IW search completed in"] =" ".join(line.split(" ")[-2:])

    if "Plan found with cost" not in result_dict[file]:
        result_dict[file]["Plan found with cost"]="no plan"


print(result_dict)


result_out_put={}

out_put_list=[]


result_dict_keys=sorted(result_dict.keys())
for key in result_dict_keys:
    temp_list=[]
    for i in ["Plan found with cost","Total time","Nodes generated during search","Nodes expanded during search" ,"IW search completed in"]:
        temp_list.append(result_dict[key][i])
    result_out_put[key]=temp_list
    temp_list=[]



for key in result_out_put.keys():
    temp_list=[]
    temp_list.append(key)
    for i in range(len(result_out_put[key])):
        temp_list.append(result_out_put[key][i])
    out_put_list.append(temp_list)

def write(output_list,output_file):
    with open(output_file,"w") as f:
        writer=csv.writer(f)
        writer.writerow(["file name","Plan found with cost","Total time","Nodes generated during search","Nodes expanded during search" ,"IW search completed in" ])
        for row in output_list:
            writer.writerow(row)


write(out_put_list,"./blocksworld_result/result_left.csv")

#result=os.popen("./bfws --domain  ../ipc-2011-duality/blocksworld/domain_duality.pddl --problem ../ipc-2011-duality/blocksworld/instances/duality_blocksaips01.pddl --output ./result/duality_01_result.txt --BFWS-f5 1").read()

#for line in result.split("\n"):
#     if "Total time" in line:
'''
import time
import subprocess

def cmd(command,result_dict,file):
    result_dict[file]=dict()
    subp = subprocess.Popen(command,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,encoding="utf-8")
    try:
        subp.wait(600)
    except:
            result_dict[file]["Plan found with cost"] = "--"
            result_dict[file]["Total time"] = "--"
            result_dict[file]["Nodes generated during search"] = "--"
            result_dict[file]["Nodes expanded during search"] = "--"
            result_dict[file]["IW search completed in"] = "--"
#            result_dict[file]["Nodes pruned by mutex"] = "--"
            result_dict[file]["Nodes pruned by bound"] = "--"
            result_dict[file]["Plan found with cost"] = "no plan"
    if subp.poll() == 0:
        for line in (subp.communicate()[0]).split("\n"):
            if "Plan found with cost" in line:
                result_dict[file]["Plan found with cost"] = line.split(":")[-1]
            if "Total time" in line:
                result_dict[file]["Total time"] = line.split(":")[-1]
            if "Nodes generated during search" in line:
                result_dict[file]["Nodes generated during search"] = line.split(":")[-1]
            if "Nodes expanded during search" in line:
                result_dict[file]["Nodes expanded during search"] = line.split(":")[-1]
            if "IW search completed in" in line:
                result_dict[file]["IW search completed in"] = " ".join(line.split(" ")[-2:])
            if "Nodes pruned by mutex" in line:
                result_dict[file]["Nodes pruned by mutex"] = line.split(":")[-1]
            if "Nodes pruned by bound" in line:
                result_dict[file]["Nodes pruned by bound"] = line.split(":")[-1]
            if "Plan found with cost" not in result_dict[file]:
                result_dict[file]["Plan found with cost"] = "no plan"
    else:
        result_dict[file]["Plan found with cost"] = "time out"
        result_dict[file]["Total time"] = "time out"
        result_dict[file]["Nodes generated during search"] = "time out"
        result_dict[file]["Nodes expanded during search"] = "time out"
        result_dict[file]["IW search completed in"] = "time out"
#        result_dict[file]["Nodes pruned by mutex"] = "time out"
        result_dict[file]["Nodes pruned by bound"] = "time out"
        result_dict[file]["Plan found with cost"] = "no plan"
    return result_dict




def write(output_list, output_file):

    with open(output_file, "w") as f:
        writer = csv.writer(f)
#        writer.writerow(["file name", "Nodes pruned by mutex","Nodes pruned by bound","Plan found with cost", "Total time", "Nodes generated during search", "Nodes expanded during search",
#                  "IW search completed in"])
        writer.writerow(
            ["file name", "Nodes pruned by bound", "Plan found with cost", "Total time",
             "Nodes generated during search", "Nodes expanded during search",
                             "IW search completed in"])
        for row in output_list:
            writer.writerow(row)

def run_bfws(domain):
    subprocess.Popen("mkdir /home/chao/LAPKT-edit/Nir_results/FW_1/"+domain, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8")
    path = os.path.join("/home/chao/LAPKT-edit/new-pddl-domains",domain,"instances")
    domain_path=os.path.join("/home/chao/LAPKT-edit/new-pddl-domains",domain,"domain.pddl")
    files = os.listdir( path)

    result_dict=dict()
    files.sort()
    for file in files:
        print(file)
        cmd("/home/chao/LAPKT_Forward/siw_forward_cmake/cmake-build-debug/siw --domain "+ domain_path+" --problem "+os.path.join(path,file)+" --output /home/chao/LAPKT-edit/Nir_results/FW_1/"+domain+"/" + file + "_result.txt --bound 1",result_dict,file)
    result_out_put = {}

    out_put_list = []

    result_dict_keys = sorted(result_dict.keys())
    for key in result_dict_keys:
        temp_list = []
        #for i in ["Nodes pruned by mutex", "Nodes pruned by bound", "Plan found with cost", "Total time",
        #              "Nodes generated during search", "Nodes expanded during search",
        #              "IW search completed in"]:
        for i in [ "Nodes pruned by bound", "Plan found with cost", "Total time",
                      "Nodes generated during search", "Nodes expanded during search",
                      "IW search completed in"]:
            temp_list.append(result_dict[key][i])
        result_out_put[key] = temp_list

    for key in result_out_put.keys():
        temp_list = []
        temp_list.append(key)
        for i in range(len(result_out_put[key])):
            temp_list.append(result_out_put[key][i])
        out_put_list.append(temp_list)

    write(out_put_list,os.path.join("/home/chao/LAPKT-edit/Nir_results/FW_1",domain,"result.csv"))

def start():
    files = os.listdir("/home/chao/LAPKT-edit/new-pddl-domains")
    for file in files:
        run_bfws(file)



if __name__ == "__main__":
    start()