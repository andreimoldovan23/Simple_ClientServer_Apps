# A guessing game

- the server chooses a random integer between 1 and 1000
- the client's keep trying to guess that number
- the server let's each client know if it's guess was higher or lower than the actual number and the clients refine their guessing based on that information
- the game ends when someone guesses the number of the server
- the server will let everyone know at the end if they won or lost and how many times they've tried to guess

# The server is concurrent multithreaded
