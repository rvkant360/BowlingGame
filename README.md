# BowlingGame
Implement a program calculating the score of a single player in Bowling.
# Game Rules
* Game consists of 10 Frames
* Each frame, the player has two tries to knock down 10 pins with a bowling ball
* The score for the frame is the total of knocked down pins, plus bounuses for strikes and spare
* A Spare is when a player knocks down all 10 pins in two tries
  * The bonus for this frame is the number of pins knocked down by the next roll
* A Strike is when the player knocks down all the pins on his first try
  * The bonus for this frame is the number of pins knocked down by the next two rolls
* In the 10th frame, a player is allowed to throw an extra ball if he/she scores a spare or a strike
  * No more than three throws are allowed for the 10th frame
  * No bonus scores are granted for strikes and spares for the 10th frame
  * if a player hits a strike/ spare, 10 pins will be avaialble again for the next throw within the 10th frame

# Solution
# Design
BowlingGame.jpg
