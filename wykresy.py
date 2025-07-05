import matplotlib.pyplot as plt
import numpy as np

# Dane z tabelki
instancje = [
    '9.200+20', '18.200+20', '35.200+20', '20.300+30',
    '55.300+30', '58.300+30', '55.400+40', '62.400+40',
    '68.400+40', '10.500+50', '25.500+50', '53.500+50'
]

pokrycie = [90, 85, 96, 91, 91, 84, 87, 83, 90, 83, 84, 84]

# Tworzenie wykresu
x = np.arange(len(instancje))
fig, ax = plt.subplots(figsize=(12, 6))
bars = ax.bar(x, pokrycie, color='lightgray', edgecolor='black')

# Dodanie wartości na słupkach
for bar, value in zip(bars, pokrycie):
    height = bar.get_height()
    ax.text(
        bar.get_x() + bar.get_width() / 2,
        height + 0.3,
        f'{value}%', 
        ha='center',
        va='bottom',
        fontsize=9,
        fontweight='bold'
    )


# Opisy osi
ax.set_xlabel('Instancja', fontsize=12)
ax.set_ylabel('Pokrycie [%]', fontsize=12)
ax.set_title('Pokrycie (%) dla instancji z błędami pozytywnymi', fontsize=14, fontweight='bold')

# Oś Y od 80 do 100%
ax.set_ylim(80, 100)

# Etykiety X
ax.set_xticks(x)
ax.set_xticklabels(instancje, rotation=45, ha='right')

# Siatka pozioma
ax.yaxis.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()
plt.show()
