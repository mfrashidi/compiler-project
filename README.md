# Compiler project (Fall 2023)

Final project of compiler course based on LLVM. You can find the report [here](https://docs.google.com/document/d/12rqSnLdv_H0jpD1G5L42B39vf5DCf5rOzRu4ujpFGP4/edit)

## How to run?
```
mkdir build
cd build
cmake ..
make
cd src
./gsm "<the input you want to be compiled>" > gsm.ll
llc --filetype=obj -o=gsm.o gsm.ll
clang -o gsmbin gsm.o ../../rtGSM.c
```

## Sample inputs
### Variable Declaration without Assignment
```
int a;
```
### Variable Declaration with Assignment
```
int a = 3;
```
### Multiple Declaration
```
int a, b = 4, 5;
int c, d = 6;
c = a * b;
```
### If-Elif-Else Condition
```
int a, b = 5, 3;
if a > b: begin
  a -= 1;
end
elif a < b: begin
  a -= 2;
end
else: begin
  a -= 3;
end
```
### Loop
```
int a = 7;
loopc a > 0: begin
  a -= 1;
end
```
### Multi-Line Comments
```
int a, b = 5 ^ 3;
/* a *= 2; This line is commented out and doesn't affect the procedure
b += 5 */
a -= 1;
```
### Print
```
int a = 2 * 4;
print a % 2;
```
