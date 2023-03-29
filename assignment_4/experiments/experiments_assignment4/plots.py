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
#df_seq = df[df.version == "histo_seq"]
# df_seq["thread"] = 1
#df_seq.loc[:, "thread"] = 1

# df_seq.drop(columns=["version","thread"], inplace=True)
#df = df[df["version"] != "histo_seq"]
# df_seq_desc_means = df_seq_desc.groupby(["length"]).mean()
# df_seq_rand_means = df_seq_rand.groupby(["length"]).mean()

df = df.groupby(['version','pattern','n_rows','n_cols',"thread"]).agg(["mean", "sem"])
df.reset_index(inplace=True)
#df.columns = ['version','pattern','n_rows','n_cols',"thread", "runtime","flops"]

print(df.head(3))
#merged_df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how='left', suffixes=("_seq", "_par"))

#merged_df['speedup'] = merged_df['time_elapsed_seq'] / merged_df['time_elapsed_par']
#df = merged_df
# df = df_seq.merge(df, on=['pattern','n_rows','n_cols'], how="right",suffixes=("_par", "_seq"), validate="many_to_one")




# df.drop(columns=["time_elapsed_seq"], inplace=True)

# aggregate data for bar plot
#df = df.groupby(['version_seq','version_par','pattern','thread_par','thread_seq','n_rows','n_cols']).agg(["mean", "sem"])

df.columns = df.columns.map('_'.join).str.strip("_")

df.reset_index(inplace=True)
print(df.head(3))
df['thread'] = pd.to_numeric(df['thread'], errors='coerce').astype(int)
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
# Runtime for different versions
# rows=cols, pat1, threads=16

df_plot1 = df[(df['n_rows'] == df['n_cols']) & (df['thread'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
runtime_omp_means = df_plot1[df_plot1['version'] == "heat_omp"]['runtime_mean'].values
runtime_omp_sem = df_plot1[df_plot1['version'] == "heat_omp"]['runtime_sem'].values
print(runtime_omp_means)
runtime_pth3_means = df_plot1[df_plot1['version'] == "heat_pth_v3"]['runtime_mean'].values
runtime_pth3_sem = df_plot1[df_plot1['version'] == "heat_pth_v3"]['runtime_sem'].values

runtime_pth4_means = df_plot1[df_plot1['version'] == "heat_pth_v4"]['runtime_mean'].values
runtime_pth4_sem = df_plot1[df_plot1['version'] == "heat_pth_v4"]['runtime_sem'].values

runtime_cuda1_means = df_plot1[df_plot1['version'] == "heat_cuda"]['runtime_mean'].values
runtime_cuda1_sem = df_plot1[df_plot1['version'] == "heat_cuda"]['runtime_sem'].values

runtime_cuda2_means = df_plot1[df_plot1['version'] == "heat_cuda_v2"]['runtime_mean'].values
runtime_cuda2_sem = df_plot1[df_plot1['version'] == "heat_cuda_v2"]['runtime_sem'].values

runtime_cuda3_means = df_plot1[df_plot1['version'] == "heat_cuda_v3"]['runtime_mean'].values
runtime_cuda3_sem = df_plot1[df_plot1['version'] == "heat_cuda_v3"]['runtime_sem'].values

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

offset += width
rects = ax.bar(x + offset, runtime_cuda3_means, yerr=1.96*runtime_cuda3_sem, width=width, label="CUDA v3", color="yellow")



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

df_plot1 = df[(df['n_rows'] == df['n_cols']) & (df['thread'] == 16) & (df['pattern'] == "pat1")]
df_plot1.sort_values(by=['version','n_rows'], inplace=True)
print(df_plot1)
# df_plot1 = df_plot1[(df['thread_par'] == 16)]
# df_plot1.sort_values(by=['pattern', 'version'],inplace=True)
flops_omp_means = df_plot1[df_plot1['version'] == "heat_omp"]['flops_mean'].values
flops_omp_sem = df_plot1[df_plot1['version'] == "heat_omp"]['runtime_sem'].values

flops_pth3_means = df_plot1[df_plot1['version'] == "heat_pth_v3"]['flops_mean'].values
flops_pth3_sem = df_plot1[df_plot1['version'] == "heat_pth_v3"]['flops_sem'].values

flops_pth4_means = df_plot1[df_plot1['version'] == "heat_pth_v4"]['flops_mean'].values
flops_pth4_sem = df_plot1[df_plot1['version'] == "heat_pth_v4"]['flops_sem'].values

flops_cuda1_means = df_plot1[df_plot1['version'] == "heat_cuda"]['flops_mean'].values
flops_cuda1_sem = df_plot1[df_plot1['version'] == "heat_cuda"]['flops_sem'].values

flops_cuda2_means = df_plot1[df_plot1['version'] == "heat_cuda_v2"]['flops_mean'].values
flops_cuda2_sem = df_plot1[df_plot1['version'] == "heat_cuda_v2"]['flops_sem'].values

flops_cuda3_means = df_plot1[df_plot1['version'] == "heat_cuda_v3"]['flops_mean'].values
flops_cuda3_sem = df_plot1[df_plot1['version'] == "heat_cuda_v3"]['flops_sem'].values

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

offset += width
rects = ax.bar(x + offset, flops_cuda3_means, yerr=1.96*flops_cuda3_sem, width=width, label="CUDA v3", color="yellow")



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