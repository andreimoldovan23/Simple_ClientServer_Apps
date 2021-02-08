# A program that simulates communication between teacher and students of his belonging to different groups

- there are students and teachers
- some of the students are regular students, others are group leaders
- each group has its own port
- every 5 seconds the group leader broadcasts on the group port the message "I am the leader"
- every 3 seconds the regular students generate some random strings called "questions" which they send to the group leader, who will forward them to the teacher
- the teacher listens for incoming questions from all the group leaders and responds to each one by generating a random string and an array of random integers and sending those to the corresponding group leader
- the group leaders will broadcast each answer received from the teacher on the group port
