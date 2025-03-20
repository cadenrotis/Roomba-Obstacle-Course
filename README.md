# Roomba-Obstacle-Course
# =======================
 Repository that contains my final project for Iowa State University's CprE 2880 Roomba Obstacle Course Final Project.
 
 # Project Overview:
 As a final project for my embedded systems course at Iowa State (CPRE 2880), my team and I were tasked with programming a Roomba in C that could navigate a course without hitting any obstacles or leaving the boundaries 
 of the course, all while the driver of the Roomba wasn't allowed to look at the course or the robot. To program the Roomba, we had to modify a 32-bit microcontroller (TM4C123GH6PM Microcontroller) to control 
 a Roomba via keyboard inputs with a ping ultrasonic sensor and IR sensor attached to a servo. We also had to performed serial communication using UART that would display information to a computer monitor, used an ADC to control the 
 IR sensor, analyzed time pulses to understand the results of the ping sensor, and used PWM within a Timer module to control the servo. By using the IR and ping sensor, qw was able to get distance readings of objects 
 near the Roomba that would display on a computer monitor, which would help the driver of the Roomba get an idea of their surroundings the course. Besides keyboard input, we also had several autonomous features that 
 would automatically back the Roomba up if it got to close to a course boundary (detected by a cliff sensor on the bottom of the Roomba) or it would take over going through the final portion of the course on its own.

 # My Role:
 My responsibility on the team was to create and develop the software for all of the autonomous functions of the Roomba. This meant that I had to utilize the cliff sensors, which were just color sensors at the bottom of the Roomba, to detect changes in color on the ground for the obstacle course boundaries and hole on the ground. Since the ground was gray and the boundaries of the course were marked in white tape, all I had to look for was the change in bright color being sensed, and then have the Roomba immediately stop and back up a little bit away from the course boundary. This same concept applied to sensing the hole in the course, where the Roomba would go from sensing gray to sensing black, and then it would also immediately stop and back up a little bit. The bigger challenge was creating the autonomous program for the last part of the course, where the Roomba needed to fully go around the perimeter of a red square that represented a fire and the Roomba was putting out the fire. To get this autonomous program to work, I had to do many test trials, where I figured out that I could use the cliff sensors to have the Roomba go around the "fire". By having one cliff sensor on the red square at all times and one cliff sensor off the red square, sensing the floor, at all times, I could easily keep the Roomba aligned with the perimeter of the red square and have the correct turning radius to where the Roomba was always on track with the red square.
 
 # Learnings:
 Throughout working on this project, I learned a lot more about using the C programming language to modify the bits of certain registers (GPIO, UART, Interrupts, etc.) to perform real world actions. I also got to practice
 my debugging skills when programming the different functionalities of the Roomba, espically for the autonomous portions. Besides the technical aspects learned, I also learned how to lead a team better and how to efficently break big tasks down into smaller tasks, and 
 then divide those tasks up among the people in the team. 
 
 # Resources Used: 
  - GitHub
  - C
  - Code Composer Studio
  - CyBOT platform (ISU's modified version of the Roomba device)
  - Tiva C Series TM4C123G Launchpad Evaluation Board
  - iRobot Create 2 mobile sensor platform
