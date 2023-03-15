import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
# pd.set_option('display.max_rows', 500)
pd.set_option('display.max_columns', 500)
pd.set_option('display.width', 1000)

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("res_hist.csv")

# Assuming your DataFrame is named 'df' and the column with the values is named 'column_name'





# sequential
df_seq = df[df.version == "histo_seq"]
# df_seq["thread"] = 1
df_seq.loc[:, "thread"] = 1

# df_seq.drop(columns=["version","thread"], inplace=True)
df = df[df["version"] != "histo_seq"]
# df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
# df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

df_seq = df_seq.groupby(['version','pattern','n_rows','n_cols',"thread"]).agg(['mean'])
df_seq.reset_index(inplace=True)
df_seq.columns = ['version','pattern','n_rows','n_cols',"thread", "time_elapsed"]


merged_df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how='left', suffixes=("_seq", "_par"))

merged_df['speedup'] = merged_df['time_elapsed_seq'] / merged_df['time_elapsed_par']
df = merged_df
# df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how="right",suffixes=("_par", "_seq"), validate="many_to_one")




# df.drop(columns=["time_elapsed_seq"], inplace=True)

# aggregate data for bar plot
df = df.groupby(['version_seq','version_par','pattern','thread_par','thread_seq','n_rows','n_cols']).agg(["mean", "sem"])

df.columns = df.columns.map('_'.join)

df.reset_index(inplace=True)

df['thread_par'] = pd.to_numeric(df['thread_par'], errors='coerce').astype(int)
df["pattern"] = df["pattern"].str.strip()
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

df_plot1 = df[(df['n_rows'] == 5000) & (df['n_cols'] == 5000) & (df['thread_par'] == 16)]

# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
versions = df_plot1['version_par'].values
num_versions = df.version_par.unique().size
patterns = df_plot1['pattern'].values
num_patterns = df.pattern.unique().size
speedup_means_atomic = df_plot1[df_plot1.version_par=='histo_atomic']['speedup_mean'].values
speedup_means_mutex = df_plot1[df_plot1.version_par=='histo_mutex']['speedup_mean'].values
speedup_means_semaphores = df_plot1[df_plot1.version_par=='histo_semaphores']['speedup_mean'].values
speedup_means_sw_transactional = df_plot1[df_plot1.version_par=='histo_sw_transactional']['speedup_mean'].values
speedup_means_avoiding_mutex = df_plot1[df_plot1.version_par=='histo_avoiding_mutual_ex']['speedup_mean'].values
speedup_sem_atomic = df_plot1[df_plot1.version_par=='histo_atomic']['speedup_sem'].values
speedup_sem_mutex = df_plot1[df_plot1.version_par=='histo_mutex']['speedup_sem'].values
speedup_sem_semaphores = df_plot1[df_plot1.version_par=='histo_semaphores']['speedup_sem'].values
speedup_sem_sw_transactional = df_plot1[df_plot1.version_par=='histo_sw_transactional']['speedup_sem'].values
speedup_sem_avoiding_mutex = df_plot1[df_plot1.version_par=='histo_avoiding_mutual_ex']['speedup_sem'].values

x = np.arange(num_patterns)  # the label locations
width = 0.25  # the width of the bars

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.2  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, speedup_means_avoiding_mutex, yerr=1.96*speedup_sem_avoiding_mutex, width=width, label="no mutex", color="orange")

rects = ax.bar(x + offset, speedup_means_atomic, yerr=1.96*speedup_sem_atomic, width=width, label="atomic", color="red")

offset += width
rects = ax.bar(x + offset, speedup_means_semaphores, yerr=1.96*speedup_sem_semaphores, width=width, label="semaphores", color="green")

offset += width
rects = ax.bar(x + offset, speedup_means_sw_transactional, yerr=1.96*speedup_sem_sw_transactional, width=width, label="SW transactional", color="blue")

offset += width
rects = ax.bar(x + offset, speedup_means_mutex, yerr=1.96*speedup_sem_mutex, width=width, label="with_mutex", color="purple")

ax.set_title("Speed-up for different patterns and variants of mutual exclusion", fontsize=16)

ax.set_xticks(x + width, df.pattern.unique())
ax.set_xlabel("Patterns", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
ax.set_yscale("log")
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Buffer size", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup_patterns_versions.png", dpi=300, bbox_inches="tight")


################################################################
# Speed-up for different versions and threads
# Fix length : 5000x5000, pattern=pat1
df_plot2 = df[(df.n_rows==5000) & (df.n_cols==5000) & (df.pattern=="pat1")]

df_plot2.sort_values(by=['version_par', 'thread_par'], inplace=True)
print(df_plot2)

threads = df_plot2.thread_par.unique()
num_threads = df_plot2.thread_par.size
versions = df_plot2.version_par.unique()
num_versions = df_plot2.version_par.unique().size
speedup_means_atomic = df_plot2[df_plot2.version_par=='histo_atomic']['speedup_mean'].values
speedup_means_mutex = df_plot2[df_plot2.version_par=='histo_mutex']['speedup_mean'].values
speedup_means_semaphores = df_plot2[df_plot2.version_par=='histo_semaphores']['speedup_mean'].values
speedup_means_sw_transactional = df_plot2[df_plot2.version_par=='histo_sw_transactional']['speedup_mean'].values
speedup_means_avoiding_mutex = df_plot2[df_plot2.version_par=='histo_avoiding_mutual_ex']['speedup_mean'].values
speedup_sem_atomic = df_plot2[df_plot2.version_par=='histo_atomic']['speedup_sem'].values
speedup_sem_mutex = df_plot2[df_plot2.version_par=='histo_mutex']['speedup_sem'].values
speedup_sem_semaphores = df_plot2[df_plot2.version_par=='histo_semaphores']['speedup_sem'].values
speedup_sem_sw_transactional = df_plot2[df_plot2.version_par=='histo_sw_transactional']['speedup_sem'].values
speedup_sem_avoiding_mutex = df_plot2[df_plot2.version_par=='histo_avoiding_mutual_ex']['speedup_sem'].values

fig, ax = plt.subplots(figsize=(8, 6))
# ax.bar(threads, means, yerr=1.96*sems, capsize=5, color="#8FA993")
ax.set_xticks(threads, labels=threads)
ax.set_yticks(threads, labels=threads)
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
ax.scatter(threads, speedup_means_atomic, color="red")
ax.plot(threads, speedup_means_atomic, color="red", label="atomic")
ax.scatter(threads, speedup_means_mutex, color="purple")
ax.plot(threads, speedup_means_mutex, color="purple", label="mutex")
ax.scatter(threads, speedup_means_semaphores, color="green")
ax.plot(threads, speedup_means_semaphores, color="green", label="semaphores")
ax.scatter(threads, speedup_means_sw_transactional, color="blue")
ax.plot(threads, speedup_means_sw_transactional, color="blue", label="SW transational")
ax.scatter(threads, speedup_means_avoiding_mutex, color="orange")
ax.plot(threads, speedup_means_avoiding_mutex, color="orange", label="avoiding mutex")
ax.scatter(threads, threads, color="green")
ax.plot(threads, threads, color="green", label="linear") # linear for reference
ax.legend(loc="upper left")
ax.set_title("Threads Speed-up", fontsize=16)
ax.set_xlabel("Number of Threads", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_ylim(1)

plt.xticks(rotation=0)

plt.savefig("threads_speedup.png", dpi=300, bbox_inches="tight")




df_plot3 = df[(df.n_rows==df.n_cols) & (df.pattern=="random") & (df.thread_par == 16)]

df_plot3.sort_values(by=['version_par', 'n_rows'], inplace=True)
print(df_plot3)

n_rows = df_plot3.n_rows.unique()
# num_nrows = df_plot3.n_rows.size
versions = df_plot3.version_par.unique()
num_versions = df_plot3.version_par.unique().size
speedup_means_atomic = df_plot3[df_plot3.version_par=='histo_atomic']['speedup_mean'].values
speedup_means_mutex = df_plot3[df_plot3.version_par=='histo_mutex']['speedup_mean'].values
speedup_means_semaphores = df_plot3[df_plot3.version_par=='histo_semaphores']['speedup_mean'].values
speedup_means_sw_transactional = df_plot3[df_plot3.version_par=='histo_sw_transactional']['speedup_mean'].values
speedup_means_avoiding_mutex = df_plot3[df_plot3.version_par=='histo_avoiding_mutual_ex']['speedup_mean'].values
speedup_sem_atomic = df_plot3[df_plot3.version_par=='histo_atomic']['speedup_sem'].values
speedup_sem_mutex = df_plot3[df_plot3.version_par=='histo_mutex']['speedup_sem'].values
speedup_sem_semaphores = df_plot3[df_plot3.version_par=='histo_semaphores']['speedup_sem'].values
speedup_sem_sw_transactional = df_plot3[df_plot3.version_par=='histo_sw_transactional']['speedup_sem'].values
speedup_sem_avoiding_mutex = df_plot3[df_plot3.version_par=='histo_avoiding_mutual_ex']['speedup_sem'].values

fig, ax = plt.subplots(figsize=(8, 6))
# ax.bar(threads, means, yerr=1.96*sems, capsize=5, color="#8FA993")
input_sizes = np.power(np.array(n_rows),2)
ax.set_xticks(n_rows, labels=n_rows)
ax.set_yticks(n_rows, labels=n_rows) # TODO hashed this
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
#ax.get_xaxis().set_major_formatter(mpl.ticker.ScalarFormatter())
print(speedup_means_atomic)
print(n_rows)
ax.scatter(n_rows, speedup_means_atomic, color="red")
ax.plot(n_rows, speedup_means_atomic, color="red", label="atomic")
ax.scatter(n_rows, speedup_means_mutex, color="purple")
ax.plot(n_rows, speedup_means_mutex, color="purple", label="mutex")
ax.scatter(n_rows, speedup_means_semaphores, color="green")
ax.plot(n_rows, speedup_means_semaphores, color="green", label="semaphores")
ax.scatter(n_rows, speedup_means_sw_transactional, color="blue")
ax.plot(n_rows, speedup_means_sw_transactional, color="blue", label="SW transational")
ax.scatter(n_rows, speedup_means_avoiding_mutex, color="orange")
ax.plot(n_rows, speedup_means_avoiding_mutex, color="orange", label="avoiding mutex")
# ax.scatter(n_rows, n_rows, color="green")
# ax.plot(n_rows, n_rows, color="green", label="linear") # linear for reference
ax.legend(loc="upper left")
ax.set_title("Input size comparison", fontsize=16)
ax.set_xlabel("Input size", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_ylim(1)

plt.xticks(rotation=0)

plt.savefig("size_versions.png", dpi=300, bbox_inches="tight")