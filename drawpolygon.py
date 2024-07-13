import matplotlib.pyplot as plt

coord = [[0.0,0.0], [0.0,1.0], [0.5,0.5], [1.0,1.0], [0.8,0.8], [1.0,0.0], [0.0,0.0]]

# coord = [
# [1,0],[0,0],[0,1],
# [1,0],[0,1],[0.5,0.5],
# [0.8,0.8],[1,0],[0.5,0.5],
# [0.8,0.8],[0.5,0.5],[1,1]
# ]

# coord = [
# ]

xs, ys = zip(*coord) #create lists of x and y values

plt.figure()
plt.plot(xs,ys) 
plt.show()

