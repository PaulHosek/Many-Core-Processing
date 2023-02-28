import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

pd.set_option('display.max_columns', None)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

# load data
df = pd.read_csv("exp_outer_thread_28_2.csv")


# convert to speedup to sequential
df_seq = pd.read_csv("../data/28_100.csv")
seq_mean = df_seq[df_seq["version"] == "sequential"]["runtime"].mean()
print(seq_mean)




df["speedup"] = seq_mean / df["runtime"]




# aggregate data for bar plot
df = df.groupby("nr_outer_threads").agg(["mean", "sem"])
print(df)
df.reset_index(inplace=True)

df.columns = ['nr_outer_threads', 'runtime_mean', 'runtime_sem', 'speedup_mean', 'speedup_sem']
print(df)

x_vals = df["nr_outer_threads"]
mean = df["speedup_mean"]
upper = mean + 1.96 * df["speedup_sem"]
lower = mean - 1.96 * df["speedup_sem"]

plt.figure(figsize=(8,8))
plt.title("Outer loop threading speedup comparison")
plt.fill_between(x_vals, upper, lower, alpha=0.5, color="#8FA993")

plt.plot(x_vals, mean, color="#526F75", alpha=0.8)
plt.scatter(x_vals, mean, color="#526F75", alpha=0.8, s=4)
plt.xlabel("Nr threads assigned to data parallelism")
plt.ylabel("Speedup (to sequential)")
plt.xticks(np.arange(1, 17, 2))
plt.ylim(0)

plt.savefig("exp_outer_thread.png", dpi=300, bbox_inches="tight")
