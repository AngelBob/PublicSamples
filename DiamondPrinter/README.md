# Diamond Printer

 This is one of several comp-sci projects that my son was assigned in high school. No, I didn't do his homework for him. I merely put on my Professional Software Developer hat to see if there was a better solution than the one he came up with. For some value of better.

 This program takes an integer value as input (no, there's no check on size, so be nice!) and uses that value to print a diamond shape using asterisk characters. For example:
 ```
Enter the side length: 4
```
Results in this output:
```
   *
  * *
 *   *
*     *
 *   *
  * *
   *
```
The code has three different algorithms:
1. The "brute force" double-double loop method,
2. A method that uses line equations to determine how many spaces are needed, and
3. A re-imaged line equation method that uses absolute values.
