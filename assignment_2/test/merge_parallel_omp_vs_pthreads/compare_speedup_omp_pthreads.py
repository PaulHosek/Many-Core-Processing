import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df_parallel = pd.read_csv("data.csv")
df_parallel.drop(columns=["iterations"], inplace=True)

df_seq = pd.read_csv("../merge_sequential_length_rand/data.csv")
df_seq.drop(columns=["iteration"], inplace=True)

seq_mean = df_seq[df_seq["length"] == 1000000]["time"].mean()

df_parallel["speedup"] = seq_mean / df_parallel["time"]

# aggregate data for bar plot
df_parallel = df_parallel.groupby("method").agg(["mean", "sem"])
df_parallel.columns = df_parallel.columns.map('_'.join)

df_parallel.reset_index(inplace=True)
print(df_parallel)
# rename and order bars
# df = df.replace({"parallel_v1_onlynested": "nested",
#                  "parallel_v2_onlyouter": "outer",
#                  "parallel_v3_both": "both",
#                  "parallel_v4_both_o3": "both+O3", })
# category_order = ['sequential', 'nested', 'outer', 'both', 'both+O3']
# df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
# df = df.sort_values('Implementation')

methods = df_parallel["method"].values
speedup_means = df_parallel['speedup_mean'].values
speedup_sems = df_parallel['speedup_sem'].values

time_means = df_parallel['time_mean'].values
time_sems = df_parallel['time_sem'].values

x = np.arange(len(methods))  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(12, 6),constrained_layout=True)

width = 0.25  # the width of the bars
rects = ax.bar(x, speedup_means, yerr=1.96*speedup_sems, width=width, color="red")
ax.set_title("Speed-up with OpenMP vs. pthreads", fontsize=16)

ax.set_xticks(x, methods)
ax.set_xlabel("Method", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup_omp_pthreads.png", dpi=300, bbox_inches="tight")

#---------------------------------------------------------------
# Plot for runtime
fig, ax = plt.subplots(figsize=(12, 6),constrained_layout=True)

width = 0.25  # the width of the bars
rects = ax.bar(x, time_means, yerr=1.96*time_sems, width=width, color="red")
    
ax.set_title("Runtime with OpenMP vs. pthreads", fontsize=16)

ax.set_xticks(x, methods)
ax.set_xlabel("Method", fontsize=14)
ax.set_ylabel("Runtime in s", fontsize=14)
ax.set_yscale('log', base=10)
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("runtime_omp_pthreads.png", dpi=300, bbox_inches="tight")
