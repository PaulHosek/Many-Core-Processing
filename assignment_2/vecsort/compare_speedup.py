import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
from scipy import stats

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("data/28_100.csv")
# convert to speedup
seq_mean = df[df["version"] == "sequential"]["runtime"].mean()
df["speedup"] = seq_mean / df["runtime"]
print(df)
# aggregate data for bar plot
df = df.groupby("version").agg(["mean", "sem"])

df.reset_index(inplace=True)
df.columns = ['Implementation', 'runtime_mean', 'runtime_sem', 'speedup_mean', 'speedup_sem']

# rename and order bars
df = df.replace({"parallel_v1_onlynested": "task",
                 "parallel_v2_onlyouter": "data",
                 "parallel_v3_both": "combined",
                 "parallel_v4_both_o3": "combined+O3", })
category_order = ['sequential', 'task', 'data', 'combined', 'combined+O3']
df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
df = df.sort_values('Implementation')

print(df)
versions = df["Implementation"]
means = df['speedup_mean'].values
sems = df['speedup_sem'].values

fig, ax = plt.subplots(figsize=(9, 9))
ax.bar(versions, means, yerr=1.96 * sems, capsize=5, color="#8FA993")
ax.set_xticks(versions, labels=versions)
ax.set_title("Vecsort speedup comparison", fontsize=16)
ax.set_xlabel("Type of parallelism", fontsize=16)
ax.set_ylabel("Speedup (to sequential)", fontsize=16)
plt.ylim(0)
plt.xticks(rotation=0)

plt.savefig("vecsort_rel.png", dpi=300, bbox_inches="tight")

#
# tval,pval = stats.ttest_ind()
# print(stats.ttest_ind())

