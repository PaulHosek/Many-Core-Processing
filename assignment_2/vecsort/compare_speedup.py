import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("data/27_short_1_2.csv")
# convert to speedup
seq_mean = df[df["version"] == "sequential"]["runtime"].mean()
df["speedup"] = seq_mean / df["runtime"]

# aggregate data for bar plot
df = df.groupby("version").agg(["mean", "sem"])
print(df)
df.reset_index(inplace=True)
df.columns = ['Implementation', 'runtime_mean', 'runtime_sem', 'speedup_mean', 'speedup_sem']

# rename and order bars
df = df.replace({"parallel_v1_onlynested": "nested",
                 "parallel_v2_onlyouter": "outer",
                 "parallel_v3_both": "both",
                 "parallel_v4_both_o3": "both+O3", })
category_order = ['sequential', 'nested', 'outer', 'both', 'both+O3']
df['Implementation'] = pd.Categorical(df['Implementation'], categories=category_order, ordered=True)
df = df.sort_values('Implementation')

print(df)
versions = df["Implementation"]
means = df['speedup_mean'].values
sems = df['speedup_sem'].values

fig, ax = plt.subplots(figsize=(12, 6))
ax.bar(versions, means, yerr=1.96 * sems, capsize=5, color="#8FA993")
ax.set_xticks(versions, labels=versions)
ax.set_title("Vecsort speedup comparison", fontsize=16)
ax.set_xlabel("Version", fontsize=14)
ax.set_ylabel("Speedup", fontsize=14)
ax.set_ylim(1)

plt.xticks(rotation=0)

plt.savefig("vecsort_rel.png", dpi=300, bbox_inches="tight")
