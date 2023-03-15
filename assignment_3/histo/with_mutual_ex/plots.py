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
df_seq.drop(columns=["version","thread"], inplace=True)
df = df.loc[df["version"] != "histo_seq"]
# df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
# df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

df_seq = df_seq.groupby(['version','pattern','n_rows','n_cols'])["runtime"].aggregate("mean")

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

################################################################
# Speed-up for different patterns and versions
# Fix length : 5000x5000, threads=16
df_plot1 = df[(df.n_rows==5000) & (df.n_cols==5000) & (df.thread==16)]
df_plot1.sort_values(by=['pattern', 'version'],inplace=True)

versions = df_plot1['version'].values
num_versions = df.version.unique().size
patterns = df_plot1['pattern'].values
num_patterns = df.pattern.unique().size
speedup_means_atomic = df_plot1[df_plot1.version=='histo_atomic']['speedup_mean'].values
speedup_means_mutex = df_plot1[df_plot1.version=='histo_mutex']['speedup_mean'].values
speedup_means_semaphores = df_plot1[df_plot1.version=='histo_semaphores']['speedup_mean'].values
speedup_means_sw_transactional = df_plot1[df_plot1.version=='histo_sw_transactional']['speedup_mean'].values
speedup_means_avoiding_mutex = df_plot1[df_plot1.version=='histo_avoiding_mutual_ex']['speedup_mean'].values
speedup_sem_atomic = df_plot1[df_plot1.version=='histo_atomic']['speedup_sem'].values
speedup_sem_mutex = df_plot1[df_plot1.version=='histo_mutex']['speedup_sem'].values
speedup_sem_semaphores = df_plot1[df_plot1.version=='histo_semaphores']['speedup_sem'].values
speedup_sem_sw_transactional = df_plot1[df_plot1.version=='histo_sw_transactional']['speedup_sem'].values
speedup_sem_avoiding_mutex = df_plot1[df_plot1.version=='histo_avoiding_mutual_ex']['speedup_sem'].values

x = np.arange(num_patterns)  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.25  # the width of the bars
offset = 0
rects = ax.bar(x + offset, speedup_means_atomic, yerr=1.96*speedup_sem_atomic, width=width, label="atomic", color="red")
    
offset += width
rects = ax.bar(x + offset, speedup_means_mutex, yerr=1.96*speedup_sem_semaphores, width=width, label="semaphores", color="green")

offset += width
rects = ax.bar(x + offset, speedup_means_sw_transactional, yerr=1.96*speedup_sem_sw_transactional, width=width, label="SW transactional", color="blue")

offset += width
rects = ax.bar(x + offset, speedup_means_avoiding_mutex, yerr=1.96*speedup_sem_avoiding_mutex, width=width, label="no mutex", color="orange")

ax.set_title("Speed-up for different patterns and variants of mutual exclusion", fontsize=16)

ax.set_xticks(x + width, df.pattern.unique())
ax.set_xlabel("Patterns", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
ax.legend(loc="upper left") 
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup_patterns_versions.png", dpi=300, bbox_inches="tight")

################################################################
# Speed-up for different versions and threads
# Fix length : 5000x5000, pattern=pat1
df_plot2 = df[(df.n_rows==5000) & (df.n_cols==5000) & (df.pattern=="pat1")]
df_plot2.sort_values(by=['version', 'thread'], inplace=True)

threads = df_plot2.thread.unique()
num_threads = df_plot2.thread.version.unique().size
versions = df_plot2.version.unique()
num_versions = df_plot2.version.unique().size
speedup_means_atomic = df_plot2[df_plot2.version=='histo_atomic']['speedup_mean'].values
speedup_means_mutex = df_plot2[df_plot2.version=='histo_mutex']['speedup_mean'].values
speedup_means_semaphores = df_plot2[df_plot2.version=='histo_semaphores']['speedup_mean'].values
speedup_means_sw_transactional = df_plot2[df_plot2.version=='histo_sw_transactional']['speedup_mean'].values
speedup_means_avoiding_mutex = df_plot2[df_plot2.version=='histo_avoiding_mutual_ex']['speedup_mean'].values
speedup_sem_atomic = df_plot2[df_plot2.version=='histo_atomic']['speedup_sem'].values
speedup_sem_mutex = df_plot2[df_plot2.version=='histo_mutex']['speedup_sem'].values
speedup_sem_semaphores = df_plot2[df_plot2.version=='histo_semaphores']['speedup_sem'].values
speedup_sem_sw_transactional = df_plot2[df_plot2.version=='histo_sw_transactional']['speedup_sem'].values
speedup_sem_avoiding_mutex = df_plot2[df_plot2.version=='histo_avoiding_mutual_ex']['speedup_sem'].values

fig, ax = plt.subplots(figsize=(8, 6))
# ax.bar(threads, means, yerr=1.96*sems, capsize=5, color="#8FA993")
ax.set_xticks(threads, labels=threads)
ax.set_yticks(threads, labels=threads)
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
ax.scatter(threads, speedup_means_atomic, color="red")
ax.plot(threads, speedup_means_atomic, color="red", label="atomic")
ax.scatter(threads, speedup_means_mutex, color="blue")
ax.plot(threads, speedup_means_mutex, color="blue", label="mutex")
ax.scatter(threads, speedup_means_semaphores, color="orange")
ax.plot(threads, speedup_means_semaphores, color="orange", label="semaphores")
ax.scatter(threads, speedup_means_sw_transactional, color="brown")
ax.plot(threads, speedup_means_sw_transactional, color="brown", label="SW transational")
ax.scatter(threads, speedup_means_avoiding_mutex, color="yellow")
ax.plot(threads, speedup_means_avoiding_mutex, color="yellow", label="avoiding mutex")
ax.scatter(threads, threads, color="green")
ax.plot(threads, threads, color="green", label="linear") # linear for reference
ax.legend(loc="upper left")
ax.set_title("Threads Speed-up", fontsize=16)
ax.set_xlabel("Number of Threads", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_ylim(1)

plt.xticks(rotation=0)

plt.savefig("threads_speedup.png", dpi=300, bbox_inches="tight")