# Alpha

![Alpha Logo](alpha.png)

Alpha is a minimalist, stack-based programming language. It focuses on extreme compiler simplicity by offloading language features into a high-level library written in Alpha itself.

## Key Features
* **Minimalist Compiler**: Designed to be as small and elegant as possible.
* **Extensible Design**: Core functionality is defined within the language, not the binary.
* **Forth-like Stack**: Uses stack-based logic for efficient data manipulation.

---

## The Logo Code
The following code generates the project logo by calculating a distance field to render a geometric ring into a PPM image format.

```forth
VARIABLE W. VARIABLE H. VARIABLE Radius. VARIABLE RingWidth. VARIABLE I. VARIABLE J. 

( --- Set Canvas Dimensions --- )
128 W SET. 128 H SET. 

( --- Define Circle Geometry --- )
W GET 3 / Radius SET. 
W GET H GET * 22 / RingWidth SET. 

( --- Output PPM Header for P3 Format --- )
"P3\n" W GET . '\t' .. H GET . '\t' .. CR. "255\n\n" 

( --- Pixel Calculation Logic --- )
( Stack: I, J -> Output: B, G, R ) 
: Color. 
  DUP2 I SET J SET. 
  
  ( Calculate squared distance from center )
  I GET H GET 2 / - DUP * J GET W GET 2 / - DUP * + 
  Radius GET DUP * - ABS 
  
  ( Determine if pixel is within the ring boundary )
  RingWidth GET < I GET W GET 4 / < OR. 
  J GET H GET 2 / - ABS H GET 4 * 11 / < AND. 
  I GET W GET 1 * 10 / > AND. 
  
  ( Apply color: Blue/Yellow if in-bounds, else background gradient )
  IF 
    J GET 255 * H GET / 3 * 4 / 20 80 
  ELSE 
    I GET 255 * W GET / 16 / DUP DUP 
  FI. 
; 

( --- Main Rendering Loop --- )
H GET DO. 
  W GET DO. 
    Color 3 DO >R . '\t' .. < R 1 - OD. 
  1 - OD.
