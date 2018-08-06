# Simple map/reduce header for C++17 client side code

This is a small project that should help me learn more about C++, but I'm putting it here
since it might be helpful for new people coming from a more functional background, such as haskell or scala.

The single header file contains a number of basic commodities, which work on a range of containers, 
such as `fmap`, `flatmap`, `filter`, `group_by` and a generic container-to-string with `show`.

It also features a few macros to provide a more convenient lambda syntax for the easiest cases.

**Any suggestions, improvements, issues and pull requests are greatly appreciated.**

## Compatibility

This code compiles with VSC2017 for now. Will upload the actual code once I got it portable enough for at least clang.



