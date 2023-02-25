import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import matplotlib as mpl
pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

df = pd.read_csv("data/final_run.csv").groupby("version").agg(["mean", "sem"])
df.reset_index(inplace=True)
df.columns = ['Implementation', 'runtime_mean', 'runtime_sem']

names = ["sequential", "only nested", "only outer", "both", "inline"]
versions = df.index.values
print(df)


means = df['runtime_mean'].values
sems = df['runtime_sem'].values

fig, ax = plt.subplots(figsize=(8, 6))
ax.bar(versions, means, yerr=1.96*sems, capsize=5, color="#8FA993")
ax.set_xticks(versions, labels=names)
ax.set_title("Vecsort runtime comparison", fontsize=16)
ax.set_xlabel("Version", fontsize=14)
ax.set_ylabel("Runtime", fontsize=14)

plt.xticks(rotation=0)

plt.savefig("vecsort_comp.png", dpi=300, bbox_inches="tight")
