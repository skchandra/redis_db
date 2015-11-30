# graph = {'A': ['B', 'C'],
#          'B': ['A', 'D', 'E'],
#          'C': ['A', 'F'],
#          'D': ['B'],
#          'E': ['B', 'F'],
#          'F': ['C', 'E']}


# def bfs(graph,root,end):
#   	if root == end: 
# 		return [root]
  
# 	queue = []
# 	path_list = []
# 	queue.append(root)
# 	path_list.append([root])
# 	while queue and path_list:
# 		node = queue.pop()
# 		path = path_list.pop()
# 		for k,v in graph.iteritems():
# 			queue.append(v)
# 			path_list.append(path.append(v))

# print bfs(graph,'A','F')


# graph is in adjacent list representation
graph = {
        '1': ['2', '3', '4'],
        '2': ['5', '6'],
        '5': ['9', '10'],
        '4': ['7', '8'],
        '7': ['11', '12']
        }

def bfs(graph, start, end):
    # maintain a queue of paths
    queue = []
    # push the first path into the queue
    queue.append([start])
    while queue:
        # get the first path from the queue
        path = queue.pop(0)
        # get the last node from the path
        node = path[-1]
        # path found
        if node == end:
            return path
        # enumerate all adjacent nodes, construct a new path and push it into the queue
        for adjacent in graph.get(node, []):
            new_path = list(path)
            new_path.append(adjacent)
            queue.append(new_path)

print bfs(graph, '1', '11')
















