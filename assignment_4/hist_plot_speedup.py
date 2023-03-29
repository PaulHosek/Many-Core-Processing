import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
import os

pd.set_option('display.max_columns', 20)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
current_dir = os.path.dirname(os.path.abspath(__file__))

file_path = os.path.join(current_dir, 'res_hist_gpu.csv')

df_gpu = pd.read_csv(file_path)
df_gpu.drop(columns=['pattern', "n_cols"], inplace=True)
df_gpu = df_gpu.groupby(["version", "n_rows"]).agg(["mean", "sem"])
df_gpu.reset_index(inplace=True)
df_gpu.columns = ["version", "size", "runtime_mean", "runtime_sem"]

df_cpu = pd.read_csv(os.path.join(current_dir, "res_hist_cpu.csv"))
df_cpu.drop(columns=['pattern', "n_cols"], inplace=True)
df_cpu = df_cpu.groupby(["version", "n_rows"]).agg(["mean", "sem"])
df_cpu.reset_index(inplace=True)
df_cpu.columns = ["version", "size", "runtime_mean", "runtime_sem"]

# df_comb = pd.concat([df_gpu, df_cpu])
# sequential = df_comb[df_comb["version"] == "histo_seq"][["size", "runtime_mean"]]
# sequential = sequential.rename(columns={"runtime_mean": "runtime_mean_seq"})
# df_comb = df_comb.merge(sequential, on=["size"], how="left", validate="m:1")

df_comb = df_gpu
df_comb["runtime_seq"] = 1
df_comb["speedup"] = df_comb["runtime_mean"]/df_comb["runtime_seq"]



labels_dict = {
    'histogram_v1': 'v1 (Global)',
    'histogram_v2_1_localTBhist_strided': 'v2.1 (Shared)',
    'histogram_v2_2_warp_reduc': 'v2.2 (Warp Addition)',
    'histogram_v3_SIMD': 'v3 (SIMD)'
} # todo add nomutex and sequential versions here

fig, ax = plt.subplots()
for i in sorted(list(set(df_comb["version"]))):
    subset = df_comb[df_comb["version"] == i]
    cur_mean = subset["speedup"]
    plt.semilogy(subset["size"], cur_mean, alpha=0.8, label=labels_dict[i])
plt.ylabel("Speedup (to sequential CPU version)")
plt.xlabel("Image size (sidelength)")
legend = plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=2)
legend.set_title("Version", prop={'size': 12})
plt.axhline(1)
plt.savefig(f"{current_dir}/hist_speedup.png", bbox_inches="tight", dpi=300)
