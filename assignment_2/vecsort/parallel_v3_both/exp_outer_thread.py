# import pandas as pd
# import matplotlib as mpl
# import matplotlib.pyplot as plt
# import numpy as np
#
# pd.set_option('display.max_columns', None)
# plt.style.use('seaborn-v0_8-darkgrid')
# mpl.rcParams['font.size'] = 16
#
#
#
#
#
# df = pd.read_csv("exp_outer_thread_old.csv").groupby("nr_outer_threads").agg(["mean", "sem"])
# df.reset_index(inplace=True)
# df.columns = ["nr_outer_threads", "runtime_mean", "runtime_sem"]
# print(df)
#
# x_vals = df["nr_outer_threads"]
# mean = df["runtime_mean"]
# upper = mean + 1.96 * df["runtime_sem"]
# lower = mean - 1.96 * df["runtime_sem"]
# plt.figure()
# plt.title("Outer loop threading runtime comparison")
# plt.fill_between(x_vals, upper, lower, alpha=0.5, color="#8FA993")
#
# plt.plot(x_vals, mean, color="#526F75", alpha=0.8)
# plt.scatter(x_vals, mean, color="#526F75", alpha=0.8, s=4)
# plt.xlabel("Nr of threads in the outer loop")
# plt.ylabel("Runtime (seconds)")
# plt.xticks(np.arange(1, 17, 2))
#
# plt.savefig("exp_outer_thread.png", dpi=300, bbox_inches="tight")
