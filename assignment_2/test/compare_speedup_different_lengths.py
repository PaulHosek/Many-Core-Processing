import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df_parallel_asc = pd.read_csv("merge_parallel_length_asc/data.csv")
df_parallel_desc = pd.read_csv("merge_parallel_length_desc/data.csv")
df_parallel_rand = pd.read_csv("merge_parallel_length_rand/data.csv")
df_parallel_asc.drop(columns=["iteration"], inplace=True)
df_parallel_desc.drop(columns=["iteration"], inplace=True)
df_parallel_rand.drop(columns=["iteration"], inplace=True)

df_seq_asc = pd.read_csv("merge_sequential_length_asc/data.csv")
df_seq_desc = pd.read_csv("merge_sequential_length_desc/data.csv")
df_seq_rand = pd.read_csv("merge_sequential_length_rand/data.csv")
df_seq_asc.drop(columns=["iteration"], inplace=True)
df_seq_desc.drop(columns=["iteration"], inplace=True)
df_seq_rand.drop(columns=["iteration"], inplace=True)

# convert to speedup
df_seq_asc_means = df_seq_asc.groupby(["length"]).mean()
df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

print(df_seq_asc_means)
print(df_parallel_asc)
df_asc = df_parallel_asc.merge(df_seq_asc_means, on="length", how="inner", suffixes=("_par", "_seq"),validate="many_to_one")
df_desc = df_parallel_desc.merge(df_seq_desc_means, on="length", how="inner", suffixes=("_par", "_seq"),validate="many_to_one")
df_rand = df_parallel_rand.merge(df_seq_rand_means, on="length", how="inner", suffixes=("_par", "_seq"),validate="many_to_one")

df_asc["speedup"] = df_asc["time_seq"] / df_asc["time_par"]
df_desc["speedup"] = df_desc["time_seq"] / df_desc["time_par"]
df_rand["speedup"] = df_rand["time_seq"] / df_rand["time_par"]

df_asc.drop(columns=["time_seq"], inplace=True)
df_desc.drop(columns=["time_seq"], inplace=True)
df_rand.drop(columns=["time_seq"], inplace=True)

# aggregate data for bar plot
df_asc = df_asc.groupby("length").agg(["mean", "sem"])
df_desc = df_desc.groupby("length").agg(["mean", "sem"])
df_rand = df_rand.groupby("length").agg(["mean", "sem"])
df_asc.columns = df_asc.columns.map('_'.join)
df_desc.columns = df_desc.columns.map('_'.join)
df_rand.columns = df_rand.columns.map('_'.join)

df_asc.reset_index(inplace=True)
df_desc.reset_index(inplace=True)
df_rand.reset_index(inplace=True)
print(df_asc)
# rename and order bars
# df = df.replace({"parallel_v1_onlynested": "nested",
#                  "parallel_v2_onlyouter": "outer",
#                  "parallel_v3_both": "both",
#                  "parallel_v4_both_o3": "both+O3", })
# category_order = ['sequential', 'nested', 'outer', 'both', 'both+O3']
# df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
# df = df.sort_values('Implementation')

lengths = df_asc["length"].values
speedup_means_asc = df_asc['speedup_mean'].values
speedup_sems_asc = df_asc['speedup_sem'].values
speedup_means_desc = df_desc['speedup_mean'].values
speedup_sems_desc = df_desc['speedup_sem'].values
speedup_means_rand = df_rand['speedup_mean'].values
speedup_sems_rand = df_rand['speedup_sem'].values

time_means_asc = df_asc['time_par_mean'].values
time_sems_asc = df_asc['time_par_sem'].values
time_means_desc = df_desc['time_par_mean'].values
time_sems_desc = df_desc['time_par_sem'].values
time_means_rand = df_rand['time_par_mean'].values
time_sems_rand = df_rand['time_par_sem'].values

x = np.arange(len(lengths))  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(12, 6),constrained_layout=True)

width = 0.25  # the width of the bars
offset = 0
rects = ax.bar(x + offset, speedup_means_asc, yerr=1.96*speedup_sems_asc, width=width, label="ascending", color="red")
    
offset += width
rects = ax.bar(x + offset, speedup_means_desc, yerr=1.96*speedup_sems_desc, width=width, label="descending", color="green")

offset += width
rects = ax.bar(x + offset, speedup_means_rand, yerr=1.96*speedup_sems_rand, width=width, label="random", color="blue")

ax.set_title("Speed-up for different lengths", fontsize=16)

lengths_strings = np.array([])

for i in range(len(lengths)):
    lengths_strings = np.append(lengths_strings, str("{:.0e}".format(lengths[i])))

ax.set_xticks(x + width, lengths_strings)
ax.set_xlabel("Length", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
ax.legend(loc="upper left") 
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup_lengths.png", dpi=300, bbox_inches="tight")

#---------------------------------------------------------------
# Plot for runtime
fig, ax = plt.subplots(figsize=(12, 6),constrained_layout=True)

width = 0.25  # the width of the bars
offset = 0
rects = ax.bar(x + offset, time_means_asc, yerr=1.96*time_sems_asc, width=width, label="ascending", color="red")
    
offset += width
rects = ax.bar(x + offset, time_means_desc, yerr=1.96*time_sems_desc, width=width, label="descending", color="green")

offset += width
rects = ax.bar(x + offset, time_means_rand, yerr=1.96*time_sems_rand, width=width, label="random", color="blue")

ax.set_title("Runtime for different lengths", fontsize=16)

ax.set_xticks(x + width, lengths_strings)
ax.set_xlabel("Length", fontsize=14)
ax.set_ylabel("Runtime in s", fontsize=14)
ax.set_yscale('log', base=10)
ax.legend(loc="upper left") 
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("runtime_lengths.png", dpi=300, bbox_inches="tight")
