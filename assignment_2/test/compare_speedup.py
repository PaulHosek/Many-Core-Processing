import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df_seq = pd.read_csv("merge_sequential_length_rand/data.csv")
df_par = pd.read_csv("merge_parallel_threads/data.csv")

# convert to speedup
seq_mean = df_seq[df_seq["length"] == 1000000]["time"].mean()
df_par["speedup"] = seq_mean/df_par["time"]
df_par.drop(columns=["iterations"])
# aggregate data for bar plot
df_par = df_par.groupby("threads").agg(["mean", "sem"])
print(df_par)
df_par.reset_index(inplace=True)
df_par.columns = ['threads', 'runtime_mean', 'runtime_sem', 'speedup_mean', 'speedup_sem']



# rename and order bars
# df_par = df_par.replace({"parallel_v1_onlynested":"nested",
#                  "parallel_v2_onlyouter":"outer",
#                  "parallel_v3_both":"both",
#                  "parallel_v4_both_o3":"both+O3",})
category_order = ['1', '2', '4', '8', '16']
df_par['threads'] = pd.Categorical(df['threads'], categories=category_order, ordered=True)
df_par = df_par.sort_values('threads')


print(df_par)
threads = df_par["threads"]
means = df_par['speedup_mean'].values
sems = df_par['speedup_sem'].values


fig, ax = plt.subplots(figsize=(12, 6))
ax.bar(threads, means, yerr=1.96*sems, capsize=5, color="#8FA993")
ax.set_xticks(threads, labels=threads)
ax.set_title("Threads speedup comparison", fontsize=16)
ax.set_xlabel("Number of Threads", fontsize=14)
ax.set_ylabel("Speedup", fontsize=14)
ax.set_ylim(1)

plt.xticks(rotation=0)

plt.savefig("threads_speedup.png", dpi=300, bbox_inches="tight")
