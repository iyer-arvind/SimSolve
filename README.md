(Sim)ultaneous Non linear Equation (Solv)er
===========================================

SimSolve is a simultaneous non linear equation solver. The aim is the solve a bunch of equations in an optimal way.
The applications are engineering problems where you have a bunch of equations to solve. For example consider the kinematics problem. Assuming one knows the distance (S)and initial velocity (u) and acceleration (a) and wanted to know the final velocity (v).

```c
S = u*t + 0.5*a*t*t
v = u + a*t
a = 0.5
u = 0
S = 10
```

Solving this using simsolve yields 
```
S = 10
a = 0.5
t = 6.32455
u = 0
v = 3.16228
```

Note the important fact was dependency was not explicit. One could have equivalently stated
```
S - u*t = 0.5*a*t*t
(v - u)/t = a
a = 0.5
u = 0
S = 10
```

Method
======

SimSolv tries to numerically solve the equations by minimising the error. It uses [NLOPT](http://ab-initio.mit.edu/wiki/index.php/NLopt) for the minimisation step.

However, rather than minimising all the errors simultaneously (which should in theory yield same results), SomSolve breaks down the given set into groups which need to be solved simultaneously.For example, in the equation set above, the equations are solved in this order
```
Solve: [a = 0.5 ] for a 
Solve: [u = 0 ] for u 
Solve: [S = 10 ] for S 
Solve: [S - u*t =  0.5*a*t*t ] for t 
Solve: [a = (v - u) / t ] for v 
```

This breaking down gives massive gains in the optimisation effort required.

Technique
=========
For efficiency SimSolve does not parse the equations per-se. Simsolve generates a .cpp file, calls `g++` to compile it to `.so` file and loads it on the fly and calls the routines. 
