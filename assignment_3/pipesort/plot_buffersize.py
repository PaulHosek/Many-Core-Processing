from matplotlib import rcParams
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

plt.style.use('seaborn-v0_8-darkgrid')
rcParams['font.size'] = 16

data = pd.read_csv("data/res_buffersize_2rep.csv")

grouped = data.groupby(["n_nums", "buffersize"]).agg(["mean", "sem"])
grouped.reset_index(inplace=True)
grouped.columns = ["n_nums", "buffersize", "runtime_mean", "runtime_sem"]

plt.figure()
lengths = sorted(list(set(grouped["n_nums"])))
for i in lengths:
    subset = grouped[grouped["n_nums"] == i]
    cur_mean = subset["runtime_mean"]
    plt.plot(subset["buffersize"], cur_mean, label=i)
#     plt.fill_between(subset["buffersize"], cur_mean+1.96*cur_mean, cur_mean-1.96*cur_mean, alpha = 0.4)
# ax = sns.lineplot(x='buffersize', y='runtime_mean',hue="n_nums", data=grouped)
# ax = sns.lineplot(x='buffersize', y='runtime_mean', data=grouped[grouped["n_nums"]==1000])

plt.xlabel("Buffersize")
plt.ylabel("Runtime (seconds)")
plt.xlim((0,50))
# plt.xscale("log")
# plt.yscale("log")
plt.legend()
plt.savefig("test.png", bbox_inches="tight", dpi=300)
