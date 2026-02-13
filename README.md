# Better C++
BC++, or Better C++, is a translation layer made for C++ in an attempt to make annoying syntax more readable and enjoyable. Made with C++23.

## Why?
This project was made primarily for me and writing NewASM; since I found several things in C++ very annoying, so I changed them with BC++.
So, in conclusion, "better" is stricly speaking from my own perspective.

## Contribution
If you want to add any stuff, feel free to make a pull request.
If you have any recommendations, open an issue.

## What's different?
### Access modifiers in classes
Now they're more Java like:
```cpp
class myClass
{
  private int var = 0;
  template<typename T>
  public static void func() {}
  public explicit inline myClass() {}
  protected void func2() {}
};
```

### Class inheritance
```cpp
class Child extends Base {
  //whatever
};
```
For multiple inheritance, just do:
```cpp
class Child extends Base, public Base2 {
  //whatever
};
```

### Annotations
```cpp
@maybe_unused
void func() {}
```

### File inclusion
Now it is Go like, but this applies only to your files, you can keep `#include <iostream>`, etc.
```
link "path/to/file";
```
