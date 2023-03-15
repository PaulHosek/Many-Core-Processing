from matplotlib import rcParams, colors
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import seaborn as sns

plt.style.use('seaborn-v0_8-darkgrid')
rcParams['font.size'] = 16

data = pd.read_csv("data/res_buffersize_14.csv")

grouped = data.groupby(["n_nums", "buffersize"]).agg(["mean", "sem"])
grouped.reset_index(inplace=True)
grouped.columns = ["n_nums", "buffersize", "runtime_mean", "runtime_sem"]

cmap = colors.LinearSegmentedColormap.from_list("mycmap", ["green", "red"])

fig, ax = plt.subplots()
lengths = sorted(list(set(grouped["n_nums"])),reverse=True)
print(lengths)
ax.set_prop_cycle(color=cmap(np.linspace(0, 1, len(lengths))))

for i in lengths:
    if i ==1000 or i==4000:
        subset = grouped[grouped["n_nums"] == i]
        cur_mean = subset["runtime_mean"]
        plt.semilogy(subset["buffersize"], cur_mean, label=i, alpha=0.5, linestyle='--', linewidth=.7)
        continue
    subset = grouped[grouped["n_nums"] == i]
    cur_mean = subset["runtime_mean"]
    plt.semilogy(subset["buffersize"], cur_mean, label=i,alpha=0.8)

    # plt.fill_between(subset["buffersize"], cur_mean+cur_mean, cur_mean-cur_mean, alpha = 0.1)

# sns.lineplot(x='buffersize', y='runtime_mean',hue="n_nums", data=grouped)
# ax = sns.lineplot(x='buffersize', y='runtime_mean', data=grouped[grouped["n_nums"]==1000])

plt.xlabel("Buffersize")
plt.ylabel("Runtime (seconds)")
plt.xscale("log")
plt.xlim((0,1000))
plt.ylim((0,10))
# plt.yscale("log")
# ax.legend(loc='center left', bbox_to_anchor=(1.02, 0.5))
legend= plt.legend(loc='upper center', bbox_to_anchor=(0.5, -0.15), fancybox=True, shadow=True, ncol=4)
legend.set_title("Input length", prop={'size': 12})

plt.savefig("pp_buffersize.png", bbox_inches="tight", dpi=300)
