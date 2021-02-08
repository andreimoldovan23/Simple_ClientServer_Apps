# A datetime polling app, which is similar to NTP

- the app acts both as a client and a server
- every 3 seconds sends a broadcast on port 7777 with the message TIMEQUERRY(this is a time querry request)
- every 10 seconds sends a broadcast on port 7777 with the message DATEQUERRY(this is a date querry request)
- upon reading such requests the program sends its host's current time/date to the program that broadcasted the request(unicast)
- keeps a list of peers of the form ("ip:port", date/time)   (machines that answer its requests) and with each answer the list is updated
- the peers who haven't responded to the last 3 broadcasted requests will be removed
- keeps a queue of malformed responses of the form ("ip:port", message)
