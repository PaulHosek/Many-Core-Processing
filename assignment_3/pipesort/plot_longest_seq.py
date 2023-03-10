from matplotlib import rcParams
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

plt.style.use('seaborn-v0_8-darkgrid')
rcParams['font.size'] = 16

data = pd.read_csv("data/res_longest_seq.csv")


print(data)
plt.figure()

ax = sns.lineplot(x='n_nums', y='runtime', data=data[data["buffersize"]==10], label="10")
ax = sns.lineplot(x='n_nums', y='runtime', data=data[data["buffersize"]==100], label="100")

plt.xlabel("Length random sequence")
plt.ylabel("Runtime (seconds)")
# plt.xscale("log")
# plt.yscale("log")
plt.legend()
plt.savefig("test.png", bbox_inches="tight", dpi=300)
