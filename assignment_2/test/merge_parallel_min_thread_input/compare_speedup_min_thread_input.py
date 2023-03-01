import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df_parallel = pd.read_csv("data.csv")
df_parallel.drop(columns=["iteration"], inplace=True)

df_seq = pd.read_csv("../merge_sequential_length_rand/data.csv")
df_seq.drop(columns=["iteration"], inplace=True)

seq_mean = df_seq[df_seq["length"] == 10000000]["time"].mean()

df_parallel["speedup"] = seq_mean / df_parallel["time"]

# aggregate data for bar plot
df_parallel = df_parallel.groupby("min_thread_input").agg(["mean", "sem"])
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

min_thread_input = df_parallel["min_thread_input"].values
speedup_means = df_parallel['speedup_mean'].values
speedup_sems = df_parallel['speedup_sem'].values

time_means = df_parallel['time_mean'].values
time_sems = df_parallel['time_sem'].values

x = np.arange(len(min_thread_input))  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(8, 6),constrained_layout=True)

width = 0.25  # the width of the bars
rects = ax.bar(x, speedup_means, yerr=1.96*speedup_sems, width=width, color="red")
ax.set_title("Speed-up for different minimal thread input lengths", fontsize=16)

min_thread_input_strings = np.array([])

for i in range(len(min_thread_input)):
    min_thread_input_strings = np.append(min_thread_input_strings, str("{:.0e}".format(min_thread_input[i])))

ax.set_xticks(x, min_thread_input_strings)
ax.set_xlabel("Minimal array length to create a thread", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup_min_thread_input.png", dpi=300, bbox_inches="tight")

#---------------------------------------------------------------
# Plot for runtime
fig, ax = plt.subplots(figsize=(8, 6),constrained_layout=True)

width = 0.25  # the width of the bars
rects = ax.bar(x, time_means, yerr=1.96*time_sems, width=width, color="red")
    
ax.set_title("Runtime for different minimal thread input lengths", fontsize=16)

ax.set_xticks(x, min_thread_input_strings)
ax.set_xlabel("Minimal array length to create a thread", fontsize=14)
ax.set_ylabel("Runtime in s", fontsize=14)
ax.set_yscale('log', base=10)
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("runtime_min_thread_input.png", dpi=300, bbox_inches="tight")
