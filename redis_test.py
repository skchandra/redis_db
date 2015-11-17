from imdb import IMDb
import redis

r_server = redis.Redis('localhost') #this line creates a new Redis object and
                                    #connects to our redis server

r_server.flushall()

ia = IMDb()

# first_movie = ia.get_movie('0000001')
# print first_movie

# the_matrix = ia.get_movie('0133093')
# directors = the_matrix['director']
# movies_directed = list()
# for i,v in enumerate(directors):
# 	person = ia.get_person(directors[i].getID(), info=["filmography"])
# 	movies_directed = [i for i in person['director movie']]
# 	r_server.set(v, movies_directed)

#print r_server.get('Lana Wachowski')

for i in range(10):
	movies = list()
	name = ia.get_person(i, info=["filmography"])
	if 'actor' in name.keys():
		movies = [j for j in name['actor']]
	elif 'actress' in name.keys():
		movies = [k for k in name['actress']]
	else:
		pass
	r_server.set(i, movies)

keys = r_server.keys()
for i, v in enumerate(keys):
	print r_server.get(v)

# for person in ia.search_person('Natalie Portman'):
#     print person.personID, person['name']

# person = ia.get_person(actor[0].getID(), info=["filmography"])
# person.keys()

