# docker-compose directory

This directory contains files that can be used to run fast_daq in a docker-compose setup.  

This is mainly intended for debugging purposes.

## drip-mon service

In the docker-compose, there is a service called `drip-mon` that runs `dl-mon`.  
While using `dl-mon` can be a useful diagnostic, this service is primarily here 
as a way to have a service with fast_daq available as part of the compose setup.

As of this writing, there's no fast_daq image posted to Docker Hub, 
so you, the user, have to supply your own image in place of `[some image]`:

```
  drip-mon:
    image: [some image]
```

## Using fast_daq

First, start the broker in one temrinal.
```
host> docker-compose up rabbit-broker
```

Then run the drip-mon container interactively.
```
host> docker-compose run -v /path/to/data_prod_to_dead_end.yaml:/root/dp_to_de.yaml drip-mon /bin/bash
container> /path/to/fast_daq --auth-file /root/authentications.json -c /root/dp_to_de.yaml
```
