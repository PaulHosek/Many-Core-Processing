import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import matplotlib as mpl

pd.set_option('display.max_columns', 500)
plt.style.use('seaborn-v0_8-darkgrid')
mpl.rcParams['font.size'] = 16

data = pd.read_csv("summary.csv")[["N", 'time_not_precomputed', "time_precomputed"]].groupby('N').agg(["mean", 'sem']).reset_index()
print()
print(data)




# Data

# Set the width of each bar
bar_width = 0.35

# Position of the bars on the x-axis
N = data.N
r1 = np.arange(len(N))
r2 = [x + bar_width for x in r1]

# Create the grouped bar plot
fig, ax = plt.subplots()
rects1 = ax.bar(r1,data['time_not_precomputed']['mean'] , yerr=1.96*data['time_not_precomputed']['sem'], width=bar_width, color='#8EA992',capsize = 5, label='default')
rects2 = ax.bar(r2, data['time_precomputed']['mean'], yerr=1.96*data['time_not_precomputed']['sem'], width=bar_width, color='#516F75',capsize = 5, label='precomputed')

# Add labels, title and axis ticks
ax.set_xlabel('N')
ax.set_ylabel('Runtime')
ax.set_title('Precomputed sums')
ax.set_xticks([r + bar_width / 2 for r in range(len(N))])
ax.set_xticklabels(N)

# Add legend
ax.legend()

# Show the plot
# plt.show()



# print(set(data.N))
# print([i+str(set(data[i])) for i in data.columns])



plt.savefig("precomputed_pic.png", bbox_inches ="tight", dpi=300)