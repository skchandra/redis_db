class Node(object):
	def __init__(self, x):
		self.val = x
		self.children = None

	def BFS(self, root):
		if not root: 
			return []
		queue = [root]
		nodes = []
		while queue:
			node = queue.pop()
			nodes.append(node.val)
			print node.val
			if node:
				queue.append(node.children)
			nodes.reverse()
		return nodes

first = Node(1)
second = Node(2)
third = Node(3)
fourth = Node(4)

first.children = second, third
second.children = fourth

print first.BFS(first)








