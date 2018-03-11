# PID Control Project

[![Udacity - Self-Driving Car NanoDegree](https://s3.amazonaws.com/udacity-sdc/github/shield-carnd.svg)](http://www.udacity.com/drive)

Overview
---


In this project we are going to use PID controller to automatically steer a car in a simulation environment. 


Pipeline
---



*The overall pipeline along with the results will be described here!*

<br>

I. The main code has already been provided. I just tweeked with the hyper-parameters in order to get the best result. The main hyper-paramter that I worked with include proportional(Kp), integral(Ki), and differential coefficients(Kd).


II. In order for the vehicle to stay on track, it should always stay aligned on the center. As a result, cross track
error (cte) is the main factor that helps us to keep the vehicle on the center of the road.

III. Proportional, integral, and differential coefficients get multiplied by some other factors that depend on the cte.

IV. Proportional coeffient gets multiplied by cte, integral coefficient gets multiplied by the summation of all cross track errors so far,and finally differential coefficient gets multiplied by the difference between current cte and previous cte. For more detail please refer to the PID.cpp code.


V. Once step IV is done, steering value gets updated so that the vehicle steers in such a way that keeps itself on the center of the road.

VI. Also, in order to avoid high steer values, steer values between -1 and +1.

VII. My intitial guess for Kp, Ki, Kd was based on the course materials itself. I played with them until I got the most stable situation. Final best values include.</br>
</br>
Kp = 0.25, Ki = 0.0005, Kd = 4 </br>
Kp = 0.125, Ki = 0.0005 , Kd = 3 </br>
Kp = 0.112, Ki = 0.005, Kd = 2 </br>

VIII. Kp, Ki, Kd coefficients has their specific effects on keeping the vehicle on track. Our steering is proportional to the distance that we have from center of the track or cross track error (cte), as a result having Kp helps us to compensate for this and steer proportional to our distance from center of the road. On the other hand if we just have the Kp coefficient, the car would steer too harshly to the changes and it will oscilate back and forth. Having Kd helps to avoid this by gradually steering the vehicle. Kd is the differential coefficient which gets multiplied by the difference between the current cte and previous cte. In order to account for the systematic biad we have the Ki (integral coefficient). In other words lets say the car wheels are biased with some degrees. This means even if you keep the vehicle on a constant cte, still the vehicle is away from the center line because cte did not become zeor. Ki solves this problem by adding all previous ctes and trying to minimize their summation.


IX. Final video of the result is provided below. Please click on the following image to view the full video on YouTube. 
</br>

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/Nupljp59Mds/0.jpg)](https://www.youtube.com/watch?v=Nupljp59Mds)

</br>
<br></br>