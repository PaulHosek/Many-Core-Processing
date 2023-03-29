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
df = pd.read_csv("res_heat.csv")

# Assuming your DataFrame is named 'df' and the column with the values is named 'column_name'

# sequential
df_seq = df[df.version == "heat_seq"]
# df_seq["thread"] = 1
df_seq.loc[:, "thread"] = 1
df_seq.drop(columns=["flops"], inplace=True)
# df_seq.drop(columns=["version","thread"], inplace=True)
df = df[df["version"] != "heat_seq"]
# df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
# df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

df_seq = df_seq.groupby(['version','pattern','n_rows','n_cols',"thread"]).agg(['mean'])
df_seq.reset_index(inplace=True)
df_seq.columns = ['version','pattern','n_rows','n_cols',"thread", "runtime"]


merged_df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how='left', suffixes=("_seq", "_par"))

merged_df['speedup'] = merged_df['runtime_seq'] / merged_df['runtime_par']
df = merged_df
# df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how="right",suffixes=("_par", "_seq"), validate="many_to_one")




# df.drop(columns=["time_elapsed_seq"], inplace=True)

# aggregate data for bar plot
df = df.groupby(['version_seq','version_par','pattern','thread_par','thread_seq','n_rows','n_cols']).agg(["mean", "sem"])

df.columns = df.columns.map('_'.join)

df.reset_index(inplace=True)

df['thread_par'] = pd.to_numeric(df['thread_par'], errors='coerce').astype(int)
df["pattern"] = df["pattern"].str.strip()

print(df.head(3))

################################################################
# Runtime for different versions
# rows=cols, pat1, threads=16

df_plot1 = df[(df['n_rows'] == df['n_cols']) & (df['thread_par'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version_par','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
runtime_omp_means = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_mean'].values
runtime_omp_sem = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_sem'].values
print(runtime_omp_means)
runtime_pth3_means = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_mean'].values
runtime_pth3_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_sem'].values

runtime_pth4_means = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_mean'].values
runtime_pth4_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_sem'].values

runtime_cuda1_means = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_mean'].values
runtime_cuda1_sem = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_sem'].values

runtime_cuda2_means = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_mean'].values
runtime_cuda2_sem = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_sem'].values

# versions = df_plot1['version'].values
# num_versions = df.version.unique().size
# runtime_means = df_plot1['runtime_mean']
# runtime_sem = df_plot1['runtime_sem']
# flops_means = df_plot1['flops_mean']
# flops_sem = df_plot1['flops_sem']

rows = df_plot1.n_rows.unique()
n_rows = df_plot1.n_rows.unique().size

x = np.arange(n_rows)  # the label locations

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.1  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, runtime_omp_means, yerr=1.96*runtime_omp_sem, width=width, label="OpenMP", color="orange")

offset += width
rects = ax.bar(x + offset, runtime_pth3_means, yerr=1.96*runtime_pth3_sem, width=width, label="POSIXThreads", color="red")

offset += width
rects = ax.bar(x + offset, runtime_pth4_means, yerr=1.96*runtime_pth4_sem, width=width, label="POSIXThreads+SIMD", color="green")

offset += width
rects = ax.bar(x + offset, runtime_cuda1_means, yerr=1.96*runtime_cuda1_sem, width=width, label="CUDA v1", color="purple")

offset += width
rects = ax.bar(x + offset, runtime_cuda2_means, yerr=1.96*runtime_cuda2_sem, width=width, label="CUDA v2", color="blue")



ax.set_title("Execution time for different parallel versions of heat dissipation.", fontsize=16)

ax.set_xticks(x + width, rows)
ax.set_xlabel("Number of rows and columns", fontsize=14)
ax.set_ylabel("Runtime in s", fontsize=14)
#ax.set_yscale("log")
#ax.axhline(1)
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Versions", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("runtime_versions.png", dpi=300, bbox_inches="tight")

################################################################
# FLOP/s for different versions
# rows=cols, pat1, threads=16

df_plot1 = df[(df['n_rows'] == df['n_cols']) & (df['thread_par'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version_par','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
flops_omp_means = df_plot1[df_plot1['version_par'] == "heat_omp"]['flops_mean'].values
flops_omp_sem = df_plot1[df_plot1['version_par'] == "heat_omp"]['flops_sem'].values

flops_pth3_means = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['flops_mean'].values
flops_pth3_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['flops_sem'].values

flops_pth4_means = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['flops_mean'].values
flops_pth4_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['flops_sem'].values

flops_cuda1_means = df_plot1[df_plot1['version_par'] == "heat_cuda"]['flops_mean'].values
flops_cuda1_sem = df_plot1[df_plot1['version_par'] == "heat_cuda"]['flops_sem'].values

flops_cuda2_means = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['flops_mean'].values
flops_cuda2_sem = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['flops_sem'].values

# versions = df_plot1['version'].values
# num_versions = df.version.unique().size
# runtime_means = df_plot1['runtime_mean']
# runtime_sem = df_plot1['runtime_sem']
# flops_means = df_plot1['flops_mean']
# flops_sem = df_plot1['flops_sem']

rows = df_plot1.n_rows.unique()
n_rows = df_plot1.n_rows.unique().size

x = np.arange(n_rows)  # the label locations

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.1  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, flops_omp_means, yerr=1.96*flops_omp_sem, width=width, label="OpenMP", color="orange")

offset += width
rects = ax.bar(x + offset, flops_pth3_means, yerr=1.96*flops_pth3_sem, width=width, label="POSIXThreads", color="red")

offset += width
rects = ax.bar(x + offset, flops_pth4_means, yerr=1.96*flops_pth4_sem, width=width, label="POSIXThreads+SIMD", color="green")

offset += width
rects = ax.bar(x + offset, flops_cuda1_means, yerr=1.96*flops_cuda1_sem, width=width, label="CUDA v1", color="purple")

offset += width
rects = ax.bar(x + offset, flops_cuda2_means, yerr=1.96*flops_cuda2_sem, width=width, label="CUDA v2", color="blue")



ax.set_title("FLOP/s for different parallel versions of heat dissipation.", fontsize=16)

ax.set_xticks(x + width, rows)
ax.set_xlabel("Number of rows and columns", fontsize=14)
ax.set_ylabel("FLOP/s", fontsize=14)
#ax.set_yscale("log")
#ax.axhline(1)
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Versions", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("flops_versions.png", dpi=300, bbox_inches="tight")

################################################################
# Speedup for different versions
# rows=cols, pat1, threads=16

df_plot1 = df[(df['n_rows'] == df['n_cols']) & (df['thread_par'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version_par','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
speedup_omp_means = df_plot1[df_plot1['version_par'] == "heat_omp"]['speedup_mean'].values
speedup_omp_sem = df_plot1[df_plot1['version_par'] == "heat_omp"]['speedup_sem'].values

speedup_pth3_means = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['speedup_mean'].values
speedup_pth3_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['speedup_sem'].values

speedup_pth4_means = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['speedup_mean'].values
speedup_pth4_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['speedup_sem'].values

speedup_cuda1_means = df_plot1[df_plot1['version_par'] == "heat_cuda"]['speedup_mean'].values
speedup_cuda1_sem = df_plot1[df_plot1['version_par'] == "heat_cuda"]['speedup_sem'].values

speedup_cuda2_means = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['speedup_mean'].values
speedup_cuda2_sem = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['speedup_sem'].values

# versions = df_plot1['version'].values
# num_versions = df.version.unique().size
# runtime_means = df_plot1['runtime_mean']
# runtime_sem = df_plot1['runtime_sem']
# flops_means = df_plot1['flops_mean']
# flops_sem = df_plot1['flops_sem']

rows = df_plot1.n_rows.unique()
n_rows = df_plot1.n_rows.unique().size

x = np.arange(n_rows)  # the label locations

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.1  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, speedup_omp_means, yerr=1.96*speedup_omp_sem, width=width, label="OpenMP", color="orange")

offset += width
rects = ax.bar(x + offset, speedup_pth3_means, yerr=1.96*speedup_pth3_sem, width=width, label="POSIXThreads", color="red")

offset += width
rects = ax.bar(x + offset, speedup_pth4_means, yerr=1.96*speedup_pth4_sem, width=width, label="POSIXThreads+SIMD", color="green")

offset += width
rects = ax.bar(x + offset, speedup_cuda1_means, yerr=1.96*speedup_cuda1_sem, width=width, label="CUDA v1", color="purple")

offset += width
rects = ax.bar(x + offset, speedup_cuda2_means, yerr=1.96*speedup_cuda2_sem, width=width, label="CUDA v2", color="blue")



ax.set_title("Speed-ups for different parallel versions of heat dissipation.", fontsize=16)

ax.set_xticks(x + width, rows)
ax.set_xlabel("Number of rows and columns", fontsize=14)
ax.set_ylabel("Speed-up", fontsize=14)
#ax.set_yscale("log")
#ax.axhline(1)
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Versions", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("speedup.png", dpi=300, bbox_inches="tight")

################################################################
# Different shapes of input images for different versions
# rows=cols, pat1, threads=16

df_plot1 = df[((df['n_rows'] == 5000) | (df['n_rows'] == 500) | (df['n_rows'] == 50000)) & (df['thread_par'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version_par','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
runtime_omp_means = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_mean'].values
runtime_omp_sem = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_sem'].values

runtime_pth3_means = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_mean'].values
runtime_pth3_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_sem'].values

runtime_pth4_means = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_mean'].values
runtime_pth4_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_sem'].values

runtime_cuda1_means = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_mean'].values
runtime_cuda1_sem = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_sem'].values

runtime_cuda2_means = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_mean'].values
runtime_cuda2_sem = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_sem'].values

# versions = df_plot1['version'].values
# num_versions = df.version.unique().size
# runtime_means = df_plot1['runtime_mean']
# runtime_sem = df_plot1['runtime_sem']
# flops_means = df_plot1['flops_mean']
# flops_sem = df_plot1['flops_sem']

rows = df_plot1.n_rows.unique()
n_rows = df_plot1.n_rows.unique().size

x = np.arange(n_rows)  # the label locations

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.1  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, runtime_omp_means, yerr=1.96*runtime_omp_sem, width=width, label="OpenMP", color="orange")

offset += width
rects = ax.bar(x + offset, runtime_pth3_means, yerr=1.96*runtime_pth3_sem, width=width, label="POSIXThreads", color="red")

offset += width
rects = ax.bar(x + offset, runtime_pth4_means, yerr=1.96*runtime_pth4_sem, width=width, label="POSIXThreads+SIMD", color="green")

offset += width
rects = ax.bar(x + offset, runtime_cuda1_means, yerr=1.96*runtime_cuda1_sem, width=width, label="CUDA v1", color="purple")

offset += width
rects = ax.bar(x + offset, runtime_cuda2_means, yerr=1.96*runtime_cuda2_sem, width=width, label="CUDA v2", color="blue")



ax.set_title("Execution times for different parallel versions of heat dissipation applied to different shapes of input images.", fontsize=16)

sizes = ["500x50000", "5000x5000", "50000x500"]
ax.set_xticks(x + width, sizes)
ax.set_xlabel("Number of rows and columns", fontsize=14)
ax.set_ylabel("Runtime", fontsize=14)
#ax.set_yscale("log")
#ax.axhline(1)
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Versions", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("shapes.png", dpi=300, bbox_inches="tight")

################################################################
# Different patterns of input images for different versions
# rows=cols=5000, threads=16

df_plot1 = df[(df['n_rows'] == 5000) & (df['n_cols'] == 5000) & (df['thread_par'] == 16)]
df_plot1.sort_values(by=['version_par','pattern'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
runtime_omp_means = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_mean'].values
runtime_omp_sem = df_plot1[df_plot1['version_par'] == "heat_omp"]['runtime_par_sem'].values

runtime_pth3_means = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_mean'].values
runtime_pth3_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v3"]['runtime_par_sem'].values

runtime_pth4_means = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_mean'].values
runtime_pth4_sem = df_plot1[df_plot1['version_par'] == "heat_pth_v4"]['runtime_par_sem'].values

runtime_cuda1_means = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_mean'].values
runtime_cuda1_sem = df_plot1[df_plot1['version_par'] == "heat_cuda"]['runtime_par_sem'].values

runtime_cuda2_means = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_mean'].values
runtime_cuda2_sem = df_plot1[df_plot1['version_par'] == "heat_cuda_v2"]['runtime_par_sem'].values

# versions = df_plot1['version'].values
# num_versions = df.version.unique().size
# runtime_means = df_plot1['runtime_mean']
# runtime_sem = df_plot1['runtime_sem']
# flops_means = df_plot1['flops_mean']
# flops_sem = df_plot1['flops_sem']

patterns = df_plot1.pattern.unique()
n_patterns = df_plot1.pattern.unique().size

x = np.arange(n_patterns)  # the label locations

# Plot for speedup
fig, ax = plt.subplots(figsize=(9, 6),constrained_layout=True)

width = 0.1  # the width of the bars
offset = -0.25
offset += width
rects = ax.bar(x + offset, runtime_omp_means, yerr=1.96*runtime_omp_sem, width=width, label="OpenMP", color="orange")

offset += width
rects = ax.bar(x + offset, runtime_pth3_means, yerr=1.96*runtime_pth3_sem, width=width, label="POSIXThreads", color="red")

offset += width
rects = ax.bar(x + offset, runtime_pth4_means, yerr=1.96*runtime_pth4_sem, width=width, label="POSIXThreads+SIMD", color="green")

offset += width
rects = ax.bar(x + offset, runtime_cuda1_means, yerr=1.96*runtime_cuda1_sem, width=width, label="CUDA v1", color="purple")

offset += width
rects = ax.bar(x + offset, runtime_cuda2_means, yerr=1.96*runtime_cuda2_sem, width=width, label="CUDA v2", color="blue")



ax.set_title("Execution times for different parallel versions of heat dissipation applied to different image patterns.", fontsize=16)

ax.set_xticks(x + width, patterns)
ax.set_xlabel("Patterns", fontsize=14)
ax.set_ylabel("Runtime", fontsize=14)
#ax.set_yscale("log")
#ax.axhline(1)
legend= ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=3)
legend.set_title("Versions", prop={'size': 14})
#ax.set_ylim(1)

#plt.xticks(rotation=0)

plt.savefig("patterns.png", dpi=300, bbox_inches="tight")
