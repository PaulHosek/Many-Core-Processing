import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
import os

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
current_dir = os.path.dirname(os.path.abspath(__file__))

file_path = os.path.join(current_dir, 'res_hist_gpu.csv')

df = pd.read_csv(file_path)
df.drop(columns=['pattern', "n_cols"], inplace=True)


df = df.groupby(["version", "n_rows"]).agg(["mean", "sem"])
df.reset_index(inplace=True)
df.columns = ["version", "size", "runtime_mean", "runtime_sem"]


labels_dict = {
    'histogram_v1': 'v1 (Global)',
    'histogram_v2_1_localTBhist_strided': 'v2.1 (Shared)',
    'histogram_v2_2_warp_reduc': 'v2.2 (Warp Addition)',
    'histogram_v3_SIMD': 'v3 (SIMD)'
}


fig, ax = plt.subplots()
for i in sorted(list(set(df["version"]))):
    subset = df[df["version"] == i]
    cur_mean = subset["runtime_mean"]
    plt.semilogy(subset["size"], cur_mean, alpha=0.8, label=labels_dict[i])
    plt.fill_between(subset["size"], cur_mean+1.96*subset["runtime_sem"], cur_mean-1.96*subset["runtime_sem"], alpha = 0.2)
plt.ylabel("Runtime log(s)")
plt.xlabel("Image size (sidelength)")
legend= plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=2)
legend.set_title("Input length", prop={'size': 12})
plt.savefig(f"{current_dir}/hist_runtimes_gpu.png", bbox_inches="tight", dpi=300)



