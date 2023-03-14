import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("res_hist_patterns.csv")

# sequential
df_seq = df[df.version == "histo_seq"]
df_seq.drop(columns=["version"], inplace=True)
df = df.loc[df["version"] != "histo_seq"]
# df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
# df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

df_seq = df_seq.groupby(['version','thread','pattern','n_rows','n_cols'])["runtime"].aggregate("mean")

df = df.merge(df_seq, on=["version",'pattern','n_rows','n_cols'], suffixes=("_par", "_seq"),validate="many_to_one")

df["speedup"] = df["runtime_seq"] / df_asc["runtime_par"]

df.drop(columns=["runtime_seq"], inplace=True)

# aggregate data for bar plot
df = df.groupby(['version','pattern','thread','n_rows','n_cols']).agg(["mean", "sem"])

df.columns = df.columns.map('_'.join)

df.reset_index(inplace=True)


print(df)
# rename and order bars
# df = df.replace({"parallel_v1_onlynested": "nested",
#                  "parallel_v2_onlyouter": "outer",
#                  "parallel_v3_both": "both",
#                  "parallel_v4_both_o3": "both+O3", })
# category_order = ['sequential', 'nested', 'outer', 'both', 'both+O3']
# df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
# df = df.sort_values('Implementation')

# Fix length : 5000x5000, threads=16
df_plot1 = df[(df.n_rows==5000) & (df.n_cols==5000) & (df.thread==16)]
df_plot1.sort_values(by=['version', 'pattern'])

versions = df_plot1['version'].values
patterns = df_plot1['pattern'].values
speedup_means_asc = df_plot1['speedup_mean'].values
speedup_sems_asc = df_plot1['speedup_sem'].values

x = np.arange(len(versions))  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

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
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.25  # the width of the bars
offset = 0
rects = ax.bar(x + offset, time_means_asc, yerr=1.96*time_sems_asc, width=width, label="ascending", color="red", alpha =0.3)
ax.plot(x + offset, time_means_asc / (lengths), color="darkred", label="time_ascending/length")   

offset += width
rects = ax.bar(x + offset, time_means_desc, yerr=1.96*time_sems_desc, width=width, label="descending", color="green", alpha =0.3)
ax.plot(x + offset, time_means_desc / (lengths), color="darkgreen", label="time_descending/length") 

offset += width
rects = ax.bar(x + offset, time_means_rand, yerr=1.96*time_sems_rand, width=width, label="random", color="blue", alpha =0.3)
ax.plot(x + offset, time_means_rand / (lengths), color="darkblue", label="time_random/length") 

ax.set_title("Runtime for different lengths", fontsize=16)

ax.set_xticks(x + width, lengths_strings)

ax.set_xlabel("Length", fontsize=14)
ax.set_ylabel("Runtime in s", fontsize=14)
ax.set_yscale('log', base=10)
ax.legend(loc="upper left") 
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("runtime_lengths.png", dpi=300, bbox_inches="tight")