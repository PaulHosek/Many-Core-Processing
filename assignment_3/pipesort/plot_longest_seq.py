from matplotlib import rcParams
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

plt.style.use('seaborn-v0_8-darkgrid')
rcParams['font.size'] = 16

data = pd.read_csv("data/res_longest_seq_final.csv")



# ax = sns.lineplot(x='n_nums', y='runtime', data=data[data["buffersize"]==10], label="10")
# ax = sns.lineplot(x='n_nums', y='runtime', data=data[data["buffersize"]==100], label="100")
plt.figure()

sns.lineplot(x='n_nums', y='runtime', hue="buffersize", data=data, palette=["#red", "#brown", "#blue"], alpha=0.5)


plt.xlabel("Length random sequence")
plt.ylabel("Runtime (seconds)")
# # Add a black box around the legend
legend = plt.legend(title="Buffer size")
# frame = legend.get_frame()
# frame.set_linewidth(0.0)
# frame.set_facecolor('black')
# frame.set_alpha(1)

# Set the legend background color to white
# legend.legendPatch.set_facecolor('white')
# plt.xscale("log")
# plt.yscale("log")
plt.savefig("pp_longest_seq.png", bbox_inches="tight", dpi=300)
