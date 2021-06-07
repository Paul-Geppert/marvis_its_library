import matplotlib.pyplot as plt

with open("mem-with-free.log", "r") as data_file:
    data = data_file.readlines()

data_int = [int(x) for x in data]

plt.xlabel("Time in sec")
plt.ylabel("Memory in Kbyte")

plt.plot(data_int)
plt.show()
