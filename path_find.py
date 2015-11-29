graph = {'A': ['B', 'C'],
         'B': ['A', 'D', 'E'],
         'C': ['A', 'F'],
         'D': ['B'],
         'E': ['B', 'F'],
         'F': ['C', 'E']}

def find_path(graph, start, end, path=[]):
        path = path + [start]

        #base case
        if start == end:
            return path

        #check if start is in the graph
        if not graph.has_key(start):
            return None

        #for each child
        for node in graph[start]:
            if node not in path:
                #find the child's path
                newpath = find_path(graph, node, end, path)
                if newpath: return newpath
        return None

def find_shortest_path(graph, start, end, path=[]):
        path = path + [start]

        #base case
        if start == end:
            return path

        #check if start is in graph
        if not graph.has_key(start):
            return None
        shortest = None
        for node in graph[start]:
            if node not in path:
                #find the child's path until base case
                newpath = find_shortest_path(graph, node, end, path)
                if newpath:
                    #extra check for making sure shortest path
                    if not shortest or len(newpath) < len(shortest):
                        shortest = newpath
        return shortest


# print list(bfs_paths(graph, 'A', 'F')) # [['A', 'C', 'F'], ['A', 'B', 'E', 'F']]
print find_path(graph, 'A', 'F')
print find_shortest_path(graph, 'A', 'F')