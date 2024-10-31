import os
import pandas as pd
import matplotlib.pyplot as plt

data_dir = './data'
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(14, 15))  # 3 rows, 1 column, 14x15 inches
min_values = {'full': [], 'zoom_forks': [], 'zoom_forks_time': []}

# Loop through all CSV files in the data directory
for file in os.listdir(data_dir):
    if file.endswith('.csv'):
        file_path = os.path.join(data_dir, file)
        data = pd.read_csv(file_path)
        
        if 'FORKS' in data.columns and 'TIME' in data.columns:
            line, = ax1.plot(data['FORKS'], data['TIME'], label=file)

            min_index = data['TIME'].idxmin()
            min_forks, min_time = data['FORKS'][min_index], data['TIME'][min_index]

            ax1.axvline(x=min_forks, color=line.get_color(), linestyle='--')

            min_values['full'].append(f"{file}: ({min_forks}, {min_time})")
            
            subset_data_1 = data[(data['FORKS'] >= 1) & (data['FORKS'] <= 10)]
            line, = ax2.plot(subset_data_1['FORKS'], subset_data_1['TIME'], label=file)

            if not subset_data_1.empty:
                min_index_1 = subset_data_1['TIME'].idxmin()
                min_forks_1, min_time_1 = subset_data_1['FORKS'][min_index_1], subset_data_1['TIME'][min_index_1]
                ax2.axvline(x=min_forks_1, color=line.get_color(), linestyle='--')
                min_values['zoom_forks'].append(f"{file}: ({min_forks_1}, {min_time_1})")
            
            subset_data_2 = data[(data['FORKS'] >= 1) & (data['FORKS'] <= 10) & (data['TIME'] >= 0) & (data['TIME'] <= 250)]
            line, = ax3.plot(subset_data_2['FORKS'], subset_data_2['TIME'], label=file)

            if not subset_data_2.empty:
                min_index_2 = subset_data_2['TIME'].idxmin()
                min_forks_2, min_time_2 = subset_data_2['FORKS'][min_index_2], subset_data_2['TIME'][min_index_2]
                ax3.axvline(x=min_forks_2, color=line.get_color(), linestyle='--')
                min_values['zoom_forks_time'].append(f"{file}: ({min_forks_2}, {min_time_2})")
        else:
            print(f"Skipping file {file} as it does not contain the required columns.")

# full range
ax1.set_xlabel('Number of Forks')
ax1.set_ylabel('Time (ms)')
ax1.set_title('Time vs Forks (Full Range)')
ax1.legend(loc='upper right')

# focused on forks 1 to 10
ax2.set_xlabel('Number of Forks (1-10)')
ax2.set_ylabel('Time (ms)')
ax2.set_title('Time vs Forks (Detail: Forks 1-10)')
ax2.legend(loc='upper right')

# focused on forks 1 to 10 and time 0 to 250ms
ax3.set_xlabel('Number of Forks (1-10)')
ax3.set_ylabel('Time (0-250 ms)')
ax3.set_title('Time vs Forks (Detail: Forks 1-10, Time 0-250ms)')
ax3.legend(loc='upper right')

plt.tight_layout()

for ax, min_vals, title in zip([ax1, ax2, ax3], min_values.values(), ['Full Range', 'Zoom: Forks 1-10', 'Zoom: Forks 1-10, Time 0-250ms']):
    text = '\n'.join(min_vals)
    ax.text(0.95, 0.05, text, transform=ax.transAxes, ha='right', va='bottom', fontsize=9, color='black', bbox=dict(facecolor='white', alpha=0.7))

output_file = os.path.join(data_dir, 'result.png')
plt.savefig(output_file)

plt.close()

print(f"Plot saved as {output_file}")
