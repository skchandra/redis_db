import redis
from imdb import IMDb

r = redis.StrictRedis(host='localhost', port=6379, db=0)
pipe = r.pipeline()

from imdbpie import Imdb
imdb1 = Imdb()
imdb1 = Imdb(anonymize=True) # to proxy requests

# Creating an instance with caching enabled
# Note that the cached responses expire every 2 hours or so.
# The API response itself dictates the expiry time)
imdb1 = Imdb(cache=True)

top250 = imdb1.top_250()
dict_top250 = {}
for i in range(len(top250)):
    dict_top250[((top250[i]['tconst'].encode('utf-8'))[2:len(top250[i]['tconst'].encode('utf-8'))])] = (top250[i]['title'].encode('utf-8'))


for movId, title in dict_top250.iteritems():
    cast_list = []
    cast_dict = {}
#     print movId
    imdb2 = IMDb()
    my = imdb2.get_movie(movId)
#     pipe.execute()
    for castMember in my['cast'][0:10]:
#         cast_dict[castMember['name'].encode('utf-8')] = castMember.getID();
            cast_list.append(castMember.getID());
#     print cast_list
#     pipe.set(movId, cast_list);        
    cast_dict[movId] = cast_list
print cast_dict
    